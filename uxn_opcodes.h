#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#include <stdint.h>
#pragma once
#include "intN_t.h"   // intN_t types for any N

#pragma once
#include "uxn_stack.h"
#pragma once
#include "uxn_device.h"

/* Registers
[ Z ][ Y ][ X ][ L ][ N ][ T ] <
[ . ][ . ][ . ][   H2   ][ T ] <
[   L2   ][   N2   ][   T2   ] <
*/

typedef struct opcode_result_t {
	uint1_t is_pc_updated;
	uint16_t pc;
	
	uint1_t is_stack_index_flipped;
	
	uint1_t is_sp_shift;
	int8_t sp_relative_shift; // updated stack pointer value
	
	uint1_t is_stack_read;
	uint1_t is_stack_write;
	uint8_t stack_address_sp_offset;
	uint8_t stack_value;
	
	uint1_t is_ram_read;
	uint1_t is_ram_write;
	uint16_t ram_addr;
	uint8_t ram_value;
	
	uint1_t is_device_ram_read;
	uint1_t is_device_ram_write;
	uint8_t device_ram_address;
	uint8_t device_ram_value;
	
	uint1_t is_vram_write;
	uint32_t vram_address;
	uint2_t vram_value;
	
	uint1_t is_opc_done;
} opcode_result_t;

typedef struct eval_opcode_result_t {
	uint1_t is_pc_updated;
	uint16_t pc;
	
	uint1_t is_ram_read;
	uint1_t is_ram_write;
	uint16_t ram_addr;
	uint8_t ram_value;
	
	uint1_t is_vram_write;
	uint32_t vram_address;
	uint2_t vram_value;
	
	uint1_t is_opc_done;
} eval_opcode_result_t;

opcode_result_t jci(uint8_t phase, uint16_t pc, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// t=T;           SHIFT(-1) if(!t) { pc += 2; break; } else { rr = ram + pc; pc += PEEK2(rr) + 2; }
	static uint16_t tmp16;
	static uint8_t t8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** JCI ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = -1;
		result.is_ram_read = 1;
		result.ram_addr = pc; // peek RAM (byte 1 of 2) at address equal to PC
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.ram_addr = pc;
	}
	else if (phase == 4) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
		result.ram_addr = pc + 1; // peek RAM (byte 2 of 2) at address equal to PC + 1
	}
	else if (phase == 5) {
		result.ram_addr = pc + 1;
	}
	else if (phase == 6) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_ram_read = 0;
		result.is_pc_updated = 1;
		result.pc = t8 == 0 ? pc + 2 : (pc + tmp16 + 2);
	}
	else if (phase == 7) {
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t jmi(uint8_t phase, uint16_t pc, uint8_t previous_ram_read) {
	// rr = ram + pc; pc += PEEK2(rr) + 2;
	static uint16_t tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** JMI ***\n************\n");
		result.is_ram_read = 1;
		result.ram_addr = pc; // peek RAM (byte 1 of 2) at address equal to PC
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.ram_addr = pc;
	}
	else if (phase == 2) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
		result.ram_addr = pc + 1; // peek RAM (byte 2 of 2) at address equal to PC + 1
	}
	else if (phase == 3) {
		result.ram_addr = pc + 1;
	}
	else if (phase == 4) {
		tmp16 |= (uint16_t)(previous_ram_read);
		result.is_ram_read = 0;
		result.is_pc_updated = 1;
		result.pc = (pc + tmp16 + 2);
	}
	else if (phase == 5) {
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t jsi(uint8_t phase, uint16_t pc, uint8_t previous_ram_read) {
	// SHIFT( 2) T2_(pc + 2); rr = ram + pc; pc += PEEK2(rr) + 2;
	static uint16_t tmp16 = 0;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** JSI ***\n************\n");
		result.is_sp_shift = 1;
		result.sp_relative_shift = 2; 		// shift(2)
		tmp16 = pc + 2;
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 2) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
		result.is_ram_read = 1;
		result.ram_addr = pc; // peek RAM (byte 1 of 2) at address equal to PC
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.ram_addr = pc;
	}
	else if (phase == 4) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
		result.ram_addr = pc + 1; // peek RAM (byte 2 of 2) at address equal to PC + 1
	}
	else if (phase == 5) {
		result.ram_addr = pc + 1;
	}
	else if (phase == 6) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_ram_read = 0;
		result.is_pc_updated = 1;
		result.pc = pc + tmp16 + 2; // pc += PEEK2_RAM(pc) + 2
	}
	else if (phase == 7) {
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lit(uint8_t phase, uint16_t pc, uint8_t previous_ram_read) {
	// SHIFT( 1) T = ram[pc++];
	static uint8_t tmp8 = 0;
	static opcode_result_t result;
	
	if (phase == 0) {
		printf("************\n**** LIT ***\n************\n");
		result.is_sp_shift = 1;
		result.sp_relative_shift = 1; 		// shift(1)
		result.is_ram_read = 1;
		result.ram_addr = pc; // peek RAM at at address equal to PC
		result.is_opc_done = 0;
	} 
	else if (phase == 1) {
		result.is_sp_shift = 0;
		result.ram_addr = pc; // peek RAM at at address equal to PC
	}
	else if (phase == 2) {
		tmp8 = previous_ram_read;
		result.is_ram_read = 0;
		result.is_pc_updated = 1;
		result.pc = pc + 1; // pc += 1
	}
	else if (phase == 3) {
		result.is_pc_updated = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = tmp8; // T = PEEK_RAM(pc);
	}
	else if (phase == 4) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
		
	return result;
}

opcode_result_t lit2(uint8_t phase, uint16_t pc, uint8_t previous_ram_read) {
	// SHIFT( 2) rr = ram + pc; T2_(PEEK2(rr)) pc += 2;
	static uint16_t tmp16 = 0;
	static opcode_result_t result;
	
	if (phase == 0) {
		printf("************\n*** LIT2 ***\n************\n");
		result.is_sp_shift = 1;
		result.sp_relative_shift = 2; 		// shift(2)
		result.is_ram_read = 1;
		result.ram_addr = pc; // peek RAM (byte 1 of 2) at address equal to PC
		result.is_opc_done = 0;
	} 
	else if (phase == 1) {
		result.is_sp_shift = 0;
		result.ram_addr = pc; 
	}
	else if (phase == 2) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
		result.ram_addr = pc + 1; // peek RAM (byte 2 of 2) at address equal to PC + 1
	}
	else if (phase == 3) {
		result.ram_addr = pc + 1; 
	}
	else if (phase == 4) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_ram_read = 0;
		result.is_pc_updated = 1;
		result.pc = pc + 2; // pc += 2
	}
	else if (phase == 5) {
		result.is_pc_updated = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte) to value of RAM at PC 
	}
	else if (phase == 6) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte) to value of RAM at PC + 1
	}
	else if (phase == 7) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
		
	return result;
}

opcode_result_t pop(uint8_t phase, uint8_t ins) {
	// SET(1,-1)
	static int8_t tmp8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** POP ***\n************\n");
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -1; // x=1;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -1
		result.is_opc_done = 0;
	}
	else {
		result.is_sp_shift = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t pop2(uint8_t phase, uint8_t ins) {
	// SET(2,-2)
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** POP2 ***\n************\n");
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -2; // x=2;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -2
		result.is_opc_done = 0;
	}
	else {
		result.is_sp_shift = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ovr(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2, 1) T = n; N = t; L = n;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** OVR ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 3 : 1; // x=2;y=1; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 3 or 1
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n8;  // set T
	}
	else if (phase == 6) {
		result.stack_address_sp_offset = 2;
		result.stack_value = t8;  // set N
	}
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
		result.stack_value = n8;  // set L
	}
	else if (phase == 8) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ovr2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4, 2) T2_(n) N2_(t) L2_(n) break;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** OVR2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4; 
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 7) {
		result.stack_address_sp_offset = 3; 
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 6 : 2; // x=4;y=2; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 6 or 2
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(n16);	// set T2 = previous N2 (low byte)
	}
	else if (phase == 10) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(n16 >> 8); // set T2 = previous N2 (high byte)
	}
	else if (phase == 11) {
		result.stack_address_sp_offset = 3;
		result.stack_value = (uint8_t)(t16);	// set N2 = previous T2 (low byte)
	}
	else if (phase == 12) {
		result.stack_address_sp_offset = 4;
		result.stack_value = (uint8_t)(t16 >> 8); // set N2 = previous T2 (high byte)
	}
	else if (phase == 13) {
		result.stack_address_sp_offset = 5;
		result.stack_value = (uint8_t)(n16);	// set L2 = previous N2 (low byte)
	}
	else if (phase == 14) {
		result.stack_address_sp_offset = 6;
		result.stack_value = (uint8_t)(n16 >> 8); // set L2 = previous N2 (high byte)
	}
	else if (phase == 15) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t dei(uint8_t phase, uint8_t ins, uint8_t previous_stack_read, uint8_t previous_device_ram_read) {
	// t=T;            SET(1, 0) T = DEI(t);
	static uint1_t has_written_to_t;
	static uint8_t t8, tmp8;
	static device_in_result_t device_in_result;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** DEI ***\n************\n");
		has_written_to_t = 0;
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
		device_in_result.is_dei_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : 0; // x=1;y=0; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or 0
	}
	else {
		result.is_sp_shift = 0;
		if (~device_in_result.is_dei_done) {
			device_in_result = device_in(t8, phase - 3, previous_device_ram_read);
			result.is_device_ram_read = device_in_result.is_device_ram_read;
			result.device_ram_address = device_in_result.device_ram_address;
		} else {
			if (~has_written_to_t) {
				result.is_device_ram_read = 0;
				result.is_stack_write = 1;
				result.stack_address_sp_offset = 1;
				result.stack_value = device_in_result.dei_value;
				has_written_to_t = 1;
			} else {
				result.is_stack_write = 0;
				result.is_opc_done = 1;
			}
		}
	}
	
	return result;
}

opcode_result_t dei2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read, uint8_t previous_device_ram_read) {
	// t=T;            SET(1, 1) T = DEI(t + 1); N = DEI(t);
	static uint8_t t8, current_dei_phase, dei_param;
	static uint1_t is_first_dei_done, is_second_dei_done, has_written_to_t, has_written_to_n;
	static device_in_result_t device_in_result;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** DEI2 ***\n************\n");
		has_written_to_t = 0;
		has_written_to_n = 0;
		is_first_dei_done = 0;
		is_second_dei_done = 0;
		current_dei_phase = 0;
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
		device_in_result.is_dei_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : 1; // x=1;y=1; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or 0
	}
	else {
		result.is_sp_shift = 0;
		dei_param = is_first_dei_done ? t8 + 1 : t8;
		if (~is_first_dei_done | (has_written_to_t & ~is_second_dei_done)) {
			device_in_result = device_in(dei_param, current_dei_phase, previous_device_ram_read);
			result.is_device_ram_read = device_in_result.is_device_ram_read;
			result.device_ram_address = device_in_result.device_ram_address;
			result.is_stack_write = 0;
			current_dei_phase += 1;
			if (~is_first_dei_done & device_in_result.is_dei_done) {
				is_first_dei_done = 1;
			} else if (device_in_result.is_dei_done) {
				is_second_dei_done = 1;
			}
		}
		else if (~has_written_to_t) {
			current_dei_phase = 0;
			result.is_stack_write = 1;
			result.stack_address_sp_offset = 1;
			result.stack_value = device_in_result.dei_value;
			has_written_to_t = 1;
		}
		else if (~has_written_to_n) {
			result.is_stack_write = 1;
			result.stack_address_sp_offset = 2;
			result.stack_value = device_in_result.dei_value;
			has_written_to_n = 1;
		}
		else {
			result.is_stack_write = 0;
			result.is_opc_done = 1;
		}
	}
	
	return result;
}

opcode_result_t deo(uint8_t phase, uint8_t ins, uint8_t previous_stack_read, uint8_t previous_device_ram_read) {
	// t=T;n=N;        SET(2,-2) DEO(t, n)
	static uint8_t t8, n8;
	static opcode_result_t result;
	static device_out_result_t device_out_result;
	if (phase == 0) {
		printf("************\n**** DEO ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -2; // x=2;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -2
	}
	else {
		result.is_sp_shift = 0;
		device_out_result = device_out(t8, n8, phase - 5, previous_device_ram_read);
		result.is_device_ram_write = device_out_result.is_device_ram_write;
		result.is_device_ram_read = device_out_result.is_device_ram_read;
		result.device_ram_address = device_out_result.device_ram_address;
		result.device_ram_value = device_out_result.device_ram_value;
		result.is_vram_write = device_out_result.is_vram_write;
		result.vram_address = device_out_result.vram_address;
		result.vram_value = device_out_result.vram_value;
		result.is_opc_done = device_out_result.is_deo_done;
	}
	
	return result;
}

opcode_result_t deo2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read, uint8_t previous_device_ram_read) {
	// t=T;n=N;l=L;    SET(3,-3) DEO(t, l) DEO((t + 1), n)
	// static uint16_t t16, n16;
	static uint8_t t8, n8, l8, current_deo_phase, deo_param0, deo_param1;
	static uint1_t is_second_deo = 0;
	static opcode_result_t result;
	static device_out_result_t device_out_result;
	if (phase == 0) {
		printf("************\n*** DEO2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
		is_second_deo = 0;
		current_deo_phase = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.stack_address_sp_offset = 3; // get L
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 6) {
		l8 = previous_stack_read;
		result.is_stack_read = 0;
		result.stack_address_sp_offset = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -3; // x=3;y=-3; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -3
	}
	else {
		result.is_sp_shift = 0;
		deo_param0 = is_second_deo ? t8 + 1 : t8;
		deo_param1 = is_second_deo ? n8 : l8;
		device_out_result = device_out(deo_param0, deo_param1, current_deo_phase, previous_device_ram_read);
		result.is_device_ram_write = device_out_result.is_device_ram_write;
		result.is_device_ram_read = device_out_result.is_device_ram_read;
		result.device_ram_address = device_out_result.device_ram_address;
		result.device_ram_value = device_out_result.device_ram_value;
		result.is_vram_write = device_out_result.is_vram_write;
		result.vram_address = device_out_result.vram_address;
		result.vram_value = device_out_result.vram_value;
		result.is_opc_done = device_out_result.is_deo_done & is_second_deo;
		if (device_out_result.is_deo_done) {
			current_deo_phase = 0;
			is_second_deo = 1;
		} else {
			current_deo_phase += 1;
		}
	}

	return result;
}

opcode_result_t jmp(uint8_t phase, uint8_t ins, uint16_t pc, uint8_t previous_stack_read) {
	// t=T;            SET(1,-1) pc += (Sint8)t;
	static int8_t t8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** JMP ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = (int8_t)(previous_stack_read);
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -1; // x=1;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -1
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.is_pc_updated = 1;
		result.pc = pc + t8;
	}
	else if (phase == 4) {
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t jmp2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;           SET(2,-2) pc = t
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** JMP2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -2; // x=2;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -2
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_pc_updated = 1;
		result.pc = t16; // pc = t16
	}
	else if (phase == 6) {
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}

	return result;
}

opcode_result_t jcn(uint8_t phase, uint8_t ins, uint16_t pc, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-2) if(n) pc += (Sint8)t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** JCN ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -2; // x=2;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -2
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_pc_updated = 1;
		result.pc = n8 == 0 ? pc : pc + (int8_t)(t8); // if(n) pc += (Sint8)t;
	}
	else if (phase == 6) {
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t jcn2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=L;       SET(3,-3) if(n) pc = t;
	static uint16_t t16;
	static uint8_t n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** JCN2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3; // get L
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 3;
	} 
	else if (phase == 6) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -3; // x=3;y=(-3); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -3
	}
	else if (phase == 7) {
		result.is_sp_shift = 0;
		result.is_pc_updated = n8 == 0 ? 0 : 1;
		result.pc = n8 == 0 ? 0 : t16; // if(n) pc = t;
	}
	else if (phase == 8) {
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t jsr(uint8_t phase, uint8_t ins, uint16_t pc, uint8_t previous_stack_read) {
	// t=T;            SET(1,-1) FLIP SHIFT(2) T2_(pc) pc += (Sint8)t;
	static uint8_t t8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** JSR ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -1; // x=1;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -1
	}
	else if (phase == 3) {
		result.is_stack_index_flipped = 1;
		result.sp_relative_shift = 2;
	}
	else if (phase == 4) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(pc);		// set T2 (low byte)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(pc >> 8); 	// set T2 (high byte)
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_pc_updated = 1;
		result.pc = pc + (int8_t)(t8); // pc = t
	}
	else if (phase == 7) {
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t jsr2(uint8_t phase, uint8_t ins, uint16_t pc, uint8_t previous_stack_read) {
	// t=T2;           SET(2,-2) FLIP SHIFT(2) T2_(pc) pc = t;
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** JSR2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -2; // x=2;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -2
	}
	else if (phase == 5) {
		result.is_stack_index_flipped = 1;
		result.sp_relative_shift = 2;
	}
	else if (phase == 6) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(pc);		// set T2 (low byte)
	}
	else if (phase == 7) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(pc >> 8); 	// set T2 (high byte)
	}
	else if (phase == 8) {
		result.is_stack_write = 0;
		result.is_pc_updated = 1;
		result.pc = t16; // pc = t16
	}
	else if (phase == 9) {
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t add(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n + t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** ADD ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n8 + t8;	// set T
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t add2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-2) T2_(n + t) 
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** ADD2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = t16 + n16;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : -2; // x=4;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 10) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
	}
	else if (phase == 11) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t and(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T;n=N;        SET(2,-1) T = n & t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** AND ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n8 & t8;	// set T
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t and2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-2) T2_(n & t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** AND2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = t16 & n16;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : -2; // x=4;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 10) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
	}
	else if (phase == 11) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ora(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T;n=N;        SET(2,-1) T = n | t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** ORA ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n8 | t8;	// set T
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ora2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-2) T2_(n | t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** ORA2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = n16 | t16;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : -2; // x=4;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 10) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
	}
	else if (phase == 11) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t eor(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n ^ t; break;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** EOR ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or -1
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n8 ^ t8;	// set T
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	} 
	
	return result;
}

opcode_result_t eor2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-2) T2_(n ^ t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** EOR2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = n16 ^ t16;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : -2; // x=4;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 10) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
	}
	else if (phase == 11) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t equ(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T;n=N;        SET(2,-1) T = n == t; 
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** EQU ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or -1
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n8 == t8 ? 1 : 0;	// set T
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	} 
	
	return result;
}

opcode_result_t equ2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-3) T = n == t;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** EQU2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -3; // x=4;y=(-3); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or -3
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n16 == t16 ? 1 : 0;	// set T
	}
	else if (phase == 10) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t neq(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T;n=N;        SET(2,-1) T = n != t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** NEQ ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or -1
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n8 == t8 ? 0 : 1;	// set T
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	} 
	
	return result;
}

opcode_result_t neq2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-3) T = n != t;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** NEQ2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -3; // x=4;y=(-3); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or -3
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n16 == t16 ? 0 : 1;	// set T
	}
	else if (phase == 10) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t inc(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;            SET(1, 0) T = t + 1;
	static uint8_t t8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** INC ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : 0; // x=1y=0; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or 0
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = t8 + 1;	// set T 
	}
	else if (phase == 4) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t inc2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;           SET(2, 0) T2_(t + 1)
	static uint16_t t16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** INC2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		tmp16 = t16 + 1;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : 0; // x=2y=0; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or 0
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 6) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
	}
	else if (phase == 7) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lda(uint8_t phase, uint8_t ins, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// t=T2;           SET(2,-1) T = ram[t];
	static uint16_t t16;
	static uint8_t tmp8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** LDA ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2y=-1; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or 0
		result.is_ram_read = 1;
		result.ram_addr = t16; // peek RAM at address equal to T2
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.ram_addr = t16; 
	}
	else if (phase == 6) {
		tmp8 = previous_ram_read;
		result.is_ram_read = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = tmp8;	// set T
	}
	else if (phase == 7) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ldz(uint8_t phase, uint8_t ins, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	//  t=T;            SET(1, 0) T = ram[t]; 
	static uint8_t t8, tmp8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** LDZ ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : 0; // x=1y=0; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or 0
		result.is_ram_read = 1;
		result.ram_addr = (uint16_t)(t8); // peek RAM at address equal to T
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.ram_addr = (uint16_t)(t8); // peek RAM at address equal to T
	}
	else if (phase == 4) {
		tmp8 = previous_ram_read;
		result.is_ram_read = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = tmp8;	// set T
	}
	else if (phase == 5) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ldz2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	//  t=T;            SET(1, 1) rr = ram + t; T2_(PEEK2(rr))
	static uint8_t t8;
	static uint16_t tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** LDZ2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : 1; // x=1y=0; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or 0
		result.is_ram_read = 1;
		result.ram_addr = (uint16_t)(t8); // peek RAM at address equal to T
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.ram_addr = (uint16_t)(t8); // peek RAM at address equal to T
	}
	else if (phase == 4) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
		result.ram_addr = (uint16_t)(t8 + 1); // peek RAM at address equal to T + 1
	}
	else if (phase == 5) {
		result.ram_addr = (uint16_t)(t8 + 1); // peek RAM at address equal to T + 1
	}
	else if (phase == 6) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_ram_read = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 7) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
	}
	else if (phase == 8) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t stz(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-2) ram[t] = n;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** STZ ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -2; // x=2;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -2
		result.is_ram_write = 1;
		result.ram_addr = (uint16_t)(t8);
		result.ram_value = n8; // set first byte of n16 to ram address t8 
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_ram_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t stz2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=H2;       SET(3,-3) rr = ram + t; POKE2(rr, n)
	static uint8_t t8;
	static uint16_t n16;
	static opcode_result_t result;
	
	if (phase == 0) {
		printf("************\n*** STZ2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 3; // get H2 (byte 1 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 2; // get H2 (byte 2 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 6) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -3; // x=3y=-3; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -3
		result.is_ram_write = 1;
		result.ram_addr = (uint16_t)(t8);
		result.ram_value = (uint8_t)(n16 >> 8); // set first byte of n16 to ram address t8 
	}
	else if (phase == 7) {
		result.is_sp_shift = 0;
		result.ram_addr = (uint16_t)(t8 + 1);
		result.ram_value = (uint8_t)(n16); // set second byte of n16 to ram address t8 + 1 
	}
	else if (phase == 8) {
		result.is_ram_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ldr(uint8_t phase, uint8_t ins, uint16_t pc, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// t=T;            SET(1, 0) T = ram[pc + (Sint8)t];
	static uint8_t t8, tmp8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** LDR ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : 0; // x=1y=0; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or 0
		result.is_ram_read = 1;
		result.ram_addr = pc + (int8_t)(t8); // peek RAM at address equal to  PC + T 
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.ram_addr = pc + (int8_t)(t8);
	}
	else if (phase == 3) {
		tmp8 = previous_ram_read;
		result.is_ram_read = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = tmp8;	// set T
	}
	else if (phase == 4) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ldr2(uint8_t phase, uint8_t ins, uint16_t pc, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// t=T;            SET(1, 1) rr = ram + pc + (Sint8)t; T2_(PEEK2(rr))
	static uint8_t t8;
	static uint16_t tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** LDR2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : 1; // x=1y=1; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -3
		result.is_ram_read = 1;
		result.ram_addr = pc + (int8_t)(t8); // peek RAM (byte 1 of 2) at address equal to  PC + T 
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.ram_addr = pc + (int8_t)(t8); 
	}
	else if (phase == 4) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
		result.ram_addr = pc + (int8_t)(t8) + 1; // peek RAM (byte 2 of 2) at address equal to PC + T + 1
	}
	else if (phase == 5) {
		result.ram_addr = pc + (int8_t)(t8) + 1; 
	}
	else if (phase == 6) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_ram_read = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 7) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
	}
	else if (phase == 8) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t str1(uint8_t phase, uint8_t ins, uint16_t pc, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-2) ram[pc + (Sint8)t] = n;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** STR ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -2; // x=2y=-2; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -2
		result.is_ram_write = 1;
		result.ram_addr = pc + (int8_t)(t8);
		result.ram_value = n8; // set first n8 to ram address t8 
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_ram_write = 0;
		result.is_opc_done = 1;
		
	}
	
	return result;
}

opcode_result_t str2(uint8_t phase, uint8_t ins, uint16_t pc, uint8_t previous_stack_read) {
	// t=T;n=H2;       SET(3,-3) rr = ram + pc + (Sint8)t; POKE2(rr, n)
	static uint8_t t8;
	static uint16_t n16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** STR2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 3; // get H2 (byte 1 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 2; // get H2 (byte 2 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 6) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -3; // x=3y=-3; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -3
		result.is_ram_write = 1;
		result.ram_addr = pc + (int8_t)(t8);
		result.ram_value = (uint8_t)(n16 >> 8); // set first byte of n16 to ram address pc + t8 
	}
	else if (phase == 7) {
		result.is_sp_shift = 0;
		result.ram_addr = pc + (int8_t)(t8) + 1;
		result.ram_value = (uint8_t)(n16); // set second byte of n16 to ram address pc + t8 + 1 
	}
	else if (phase == 8) {
		result.is_ram_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lda2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// t=T2;           SET(2, 0) rr = ram + t; T2_(PEEK2(rr))
	static uint16_t t16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** LDA2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : 0; // x=2y=0; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or 0
		result.is_ram_read = 1;
		result.ram_addr = t16; // peek RAM (byte 1 of 2) at address equal to t16
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.ram_addr = t16;
	}
	else if (phase == 6) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
		result.ram_addr = t16 + 1; // peek RAM (byte 2 of 2) at address equal to t16 + 1
	}
	else if (phase == 7) {
		result.ram_addr = t16 + 1;
	}
	else if (phase == 8) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_ram_read = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 9) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
	}
	else if (phase == 10) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t gth(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n > t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** ADD ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n8 > t8 ? 1 : 0;	// set T
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t gth2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-3) T = n > t;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** GTH2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -3; // x=4;y=(-3); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or -3
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n16 > t16 ? 1 : 0;	// set T
	}
	else if (phase == 10) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lth(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n < t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** ADD ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n8 < t8 ? 1 : 0;	// set T
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lth2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-3) T = n < t;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** LTH2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -3; // x=4;y=(-3); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or -3
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n16 < t16 ? 1 : 0;	// set T
	}
	else if (phase == 10) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t mul(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n * t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** MUL ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n8 * t8;	// set T
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t mul2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-2) T2_(n * t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** MUL2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = t16 * n16;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : -2; // x=4;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 10) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
	}
	else if (phase == 11) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t div(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = t ? n / t : 0;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** DIV ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = t8 == 0 ? 0 : n8 / t8;	// set T
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t div2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-2) T2_(t ? n / t : 0)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** DIV2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = t16 == 0 ? 0 : n16 / t16;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : -2; // x=4;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 10) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
	}
	else if (phase == 11) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t nip(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;            SET(2,-1) T = t;
	static uint8_t t8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** NIP ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or -1
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = t8;	// set T
	}
	else if (phase == 4) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t nip2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;           SET(4,-2) T2_(t)
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** NIP2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : -2; // x=4;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(t16);	// set T2 (low byte)
	}
	else if (phase == 6) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(t16 >> 8); // set T2 (high byte)
	}
	else if (phase == 7) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
		
	return result;
}


opcode_result_t sft(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n >> (t & 0xf) << (t >> 4);
	static uint8_t t8, n8, tmp8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** SFT ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or -1
	}
	else if (phase == 5) {
		tmp8 = (n8 >> (t8 & 0x0F)) << (t8 >> 4);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = tmp8;	// set T
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sft2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=H2;       SET(3,-1) T2_(n >> (t & 0xf) << (t >> 4))
	static uint8_t t8;
	static uint16_t n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** SFT2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 3;  // get H2 (byte 1 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 2;  // get H2 (byte 2 of 2)
	} 
	else if (phase == 5) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 6) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : -1; // x=3;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 1 or -1
	}
	else if (phase == 7) {
		tmp16 = (n16 >> (t8 & 0x0F)) << (t8 >> 4);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 8) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
	}
	else if (phase == 9) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sta(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=L;       SET(3,-3) ram[t] = n;
	static uint16_t t16;
	static uint8_t n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** STA ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3;  // get L
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 6) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -3; // x=3;y=(-3); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -4
		result.is_ram_write = 1;
		result.ram_addr = t16; // poke RAM at address equal to T2
		result.ram_value = n8;
	}
	else if (phase == 7) {
		result.is_sp_shift = 0;
		result.is_ram_write = 0;
		result.is_opc_done = 1;
		
	}
	
	return result;
}

opcode_result_t sta2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-4) rr = ram + t; POKE2(rr, n)
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** STA2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -4; // x=4;y=(-4); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -4
		result.is_ram_write = 1;
		result.ram_addr = t16;
		result.ram_value = (uint8_t)(n16 >> 8); // set first byte of n16 to ram address t16 
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.ram_addr = t16 + 1;
		result.ram_value = (uint8_t)(n16); // set second byte of n16 to ram address t16 + 1 
	}
	else if (phase == 10) {
		result.is_ram_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sth(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;            SET(1,-1) FLIP SHIFT(1) T = t;
	static uint8_t t8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** STH ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -1; // x=1;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -1
	}
	else if (phase == 3) {
		result.is_stack_index_flipped = 1;
		result.sp_relative_shift = 2;
	}
	else if (phase == 4) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = t8; // set T
	}
	else if (phase == 5) {
		result.is_stack_write = 0;
		result.is_stack_index_flipped = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sth2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;           SET(2,-2) FLIP SHIFT(2) T2_(t)
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** STH2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 0 : -2; // x=2;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 0 or -2
	}
	else if (phase == 5) {
		result.is_stack_index_flipped = 1;
		result.sp_relative_shift = 2;
	}
	else if (phase == 6) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(t16);	// set T2 (low byte)
	}
	else if (phase == 7) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(t16 >> 8); // set T2 (high byte)
	}
	else if (phase == 8) {
		result.is_stack_write = 0;
		result.is_stack_index_flipped = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sub(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n - t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** SUB ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 1 : -1; // x=2;y=(-1); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n8 - t8;	// set T
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sub2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-2) T2_(n - t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** SUB2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = n16 - t16;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : -2; // x=4;y=(-2); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or -2
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 10) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
	}
	else if (phase == 11) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t swp(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2, 0) T = n; N = t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** SWP ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : 0; // x=2;y=(0); shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 2 or 0
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = n8;	// set T
	}
	else if (phase == 6) {
		result.stack_address_sp_offset = 2;
		result.stack_value = t8;	// set N
	}
	else if (phase == 7) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t swp2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4, 0) T2_(n) N2_(t)
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** SWP2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 4 : 0; // x=4;y=0; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 4 or 0
	}
	else if (phase == 9) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(n16);	// set T2 (low byte)
	}
	else if (phase == 10) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(n16 >> 8); // set T2 (high byte)
	}
	else if (phase == 11) {
		result.is_sp_shift = 0;
		result.stack_address_sp_offset = 3;
		result.stack_value = (uint8_t)(t16);	// set N2 (low byte)
	}
	else if (phase == 12) {
		result.stack_address_sp_offset = 4;
		result.stack_value = (uint8_t)(t16 >> 8); // set N2 (high byte)
	}
	else if (phase == 13) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t rot(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;l=L;    SET(3, 0) T = l; N = t; L = n;
	static uint8_t t8, n8, l8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** ROT ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.stack_address_sp_offset = 3; // get L
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 6) {
		l8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 3 : 0; // x=4;y=0; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 3 or 0
	}
	else if (phase == 7) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = l8;	// set T
	}
	else if (phase == 8) {
		result.stack_address_sp_offset = 2;
		result.stack_value = t8;	// set N
	}
	else if (phase == 9) {
		result.stack_address_sp_offset = 3;
		result.stack_value = n8;	// set L
	}
	else if (phase == 10) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t rot2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;l=L2; SET(6, 0) T2_(l) N2_(t) L2_(n)
	static uint16_t t16, n16, l16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** ROT2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 4;
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.stack_address_sp_offset = 3;  // get N2 (byte 2 of 2)
	} 
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 6; // get L2 (byte 1 of 2)
	}
	else if (phase == 9) {
		result.stack_address_sp_offset = 6;
	}
	else if (phase == 10) {
		l16 = (uint16_t)(previous_stack_read);
		l16 <<= 8;
		result.stack_address_sp_offset = 5;  // get L2 (byte 2 of 2)
	}
	else if (phase == 11) {
		result.stack_address_sp_offset = 5;
	}
	else if (phase == 12) {
		l16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 6 : 0; // x=6;y=0; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 6 or 0
	}
	else if (phase == 13) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(l16);	// set T2 (low byte)
	}
	else if (phase == 14) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(l16 >> 8); // set T2 (high byte)
	}
	else if (phase == 15) {
		result.stack_address_sp_offset = 3;
		result.stack_value = (uint8_t)(t16);	// set N2 (low byte)
	}
	else if (phase == 16) {
		result.stack_address_sp_offset = 4;
		result.stack_value = (uint8_t)(t16 >> 8); // set N2 (high byte)
	}
	else if (phase == 17) {
		result.stack_address_sp_offset = 5;
		result.stack_value = (uint8_t)(n16);	// set L2 (low byte)
	}
	else if (phase == 18) {
		result.stack_address_sp_offset = 6;
		result.stack_value = (uint8_t)(n16 >> 8); // set L2 (high byte)
	}
	else if (phase == 19) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t dup(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;            SET(1, 1) T = t; N = t;
	static uint8_t t8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n**** DUP ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 2 : 1; // x=6;y=0; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 6 or 0
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = t8;	// set T
	}
	else if (phase == 4) {
		result.stack_address_sp_offset = 2;
		result.stack_value = t8;	// set N
	}
	else if (phase == 5) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t dup2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;           SET(2, 2) T2_(t) N2_(t) break;
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("************\n*** DUP2 ***\n************\n");
		result.is_stack_read = 1;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = ((ins & 0x80) > 0) ? 4 : 2; // x=2;y=2; shift amount = (((ins & 0x80) > 0) ? x + y : y) ====> 4 or 2
	}
	else if (phase == 5) {
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.stack_value = (uint8_t)(t16);	// set T2 (low byte)
	}
	else if (phase == 6) {
		result.stack_address_sp_offset = 2;
		result.stack_value = (uint8_t)(t16 >> 8); // set T2 (high byte)
	}
	else if (phase == 7) {
		result.stack_address_sp_offset = 3;
		result.stack_value = (uint8_t)(t16);	// set N2 (low byte)
	}
	else if (phase == 8) {
		result.stack_address_sp_offset = 4;
		result.stack_value = (uint8_t)(t16 >> 8); // set N2 (high byte)
	}
	else if (phase == 9) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

eval_opcode_result_t eval_opcode_phased(
	uint8_t phase,
	uint8_t ins,
	uint16_t pc,
	uint8_t previous_ram_read
) {
	static uint8_t sp0, sp1;
	static uint12_t opc;
	static uint1_t stack_index;
	static uint1_t is_stack_read = 0;
	static uint1_t is_stack_write = 0;
	static uint16_t stack_address = 0;
	static uint8_t stack_write_value = 0;
	static uint8_t stack_read_value = 0;
	static uint8_t device_ram_read_value = 0;
	static opcode_result_t opc_result = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static eval_opcode_result_t opc_eval_result = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	opc = ((ins & 0x1F) > 0) ? ((uint12_t)(ins & 0x3F)) : ((uint12_t)(ins) << 4);
	stack_index = ((ins & 0x40) > 0) ? 1 : 0;
	
	printf("        EVAL OPCODE: INS = 0x%X, OPC = 0x%X, phase = 0x%X\n", ins, opc, phase);
	
	if      (opc == 0x000 /* BRK   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x200 /* JCI   */) { opc_result = jci(phase, pc, stack_read_value, previous_ram_read); }
	else if (opc == 0x400 /* JMI   */) { opc_result = jmi(phase, pc, previous_ram_read); }
	else if (opc == 0x600 /* JSI   */) { opc_result = jsi(phase, pc, previous_ram_read); }
	else if (opc == 0x800 /* LIT   */) { opc_result = lit(phase, pc, previous_ram_read); }
	else if (opc == 0xA00 /* LIT2  */) { opc_result = lit2(phase, pc, previous_ram_read); }
	else if (opc == 0xC00 /* LITr  */) { opc_result = lit(phase, pc, previous_ram_read); }
	else if (opc == 0xE00 /* LIT2r */) { opc_result = lit2(phase, pc, previous_ram_read); }
	else if (opc == 0x001 /* INC   */) { opc_result = inc(phase, ins, stack_read_value); }
	else if (opc == 0x021 /* INC2  */) { opc_result = inc2(phase, ins, stack_read_value); }
	else if (opc == 0x002 /* POP   */) { opc_result = pop(phase, ins); }
	else if (opc == 0x022 /* POP2  */) { opc_result = pop2(phase, ins); }
	else if (opc == 0x003 /* NIP   */) { opc_result = nip(phase, ins, stack_read_value); }
	else if (opc == 0x023 /* NIP2  */) { opc_result = nip2(phase, ins, stack_read_value); }
	else if (opc == 0x004 /* SWP   */) { opc_result = swp(phase, ins, stack_read_value); }
	else if (opc == 0x024 /* SWP2  */) { opc_result = swp2(phase, ins, stack_read_value); }
	else if (opc == 0x005 /* ROT   */) { opc_result = rot(phase, ins, stack_read_value); }
	else if (opc == 0x025 /* ROT2  */) { opc_result = rot2(phase, ins, stack_read_value); }
	else if (opc == 0x006 /* DUP   */) { opc_result = dup(phase, ins, stack_read_value); }
	else if (opc == 0x026 /* DUP2  */) { opc_result = dup2(phase, ins, stack_read_value); }
	else if (opc == 0x007 /* OVR   */) { opc_result = ovr(phase, ins, stack_read_value); }
	else if (opc == 0x027 /* OVR2  */) { opc_result = ovr2(phase, ins, stack_read_value); }
	else if (opc == 0x008 /* EQU   */) { opc_result = equ(phase, ins, stack_read_value); }
	else if (opc == 0x028 /* EQU2  */) { opc_result = equ2(phase, ins, stack_read_value); }
	else if (opc == 0x009 /* NEQ   */) { opc_result = neq(phase, ins, stack_read_value); }
	else if (opc == 0x029 /* NEQ2  */) { opc_result = neq2(phase, ins, stack_read_value); }
	else if (opc == 0x00A /* GTH   */) { opc_result = gth(phase, ins, stack_read_value); }
	else if (opc == 0x02A /* GTH2  */) { opc_result = gth2(phase, ins, stack_read_value); }
	else if (opc == 0x00B /* LTH   */) { opc_result = lth(phase, ins, stack_read_value);  }
	else if (opc == 0x02B /* LTH2  */) { opc_result = lth2(phase, ins, stack_read_value); }
	else if (opc == 0x00C /* JMP   */) { opc_result = jmp(phase, ins, pc, stack_read_value); }
	else if (opc == 0x02C /* JMP2  */) { opc_result = jmp2(phase, ins, stack_read_value); }
	else if (opc == 0x00D /* JCN   */) { opc_result = jcn(phase, ins, pc, stack_read_value); }
	else if (opc == 0x02D /* JCN2  */) { opc_result = jcn2(phase, ins, stack_read_value); }
	else if (opc == 0x00E /* JSR   */) { opc_result = jsr(phase, ins, pc, stack_read_value); }
	else if (opc == 0x02E /* JSR2  */) { opc_result = jsr2(phase, ins, pc, stack_read_value); }
	else if (opc == 0x00F /* STH   */) { opc_result = sth(phase, ins, stack_read_value); }
	else if (opc == 0x02F /* STH2  */) { opc_result = sth2(phase, ins, stack_read_value); }
	else if (opc == 0x010 /* LDZ   */) { opc_result = ldz(phase, ins, stack_read_value, previous_ram_read); }
	else if (opc == 0x030 /* LDZ2  */) { opc_result = ldz2(phase, ins, stack_read_value, previous_ram_read); }
	else if (opc == 0x011 /* STZ   */) { opc_result = stz(phase, ins, stack_read_value); }
	else if (opc == 0x031 /* STZ2  */) { opc_result = stz2(phase, ins, stack_read_value); }
	else if (opc == 0x012 /* LDR   */) { opc_result = ldr(phase, ins, pc, stack_read_value, previous_ram_read); }
	else if (opc == 0x032 /* LDR2  */) { opc_result = ldr2(phase, ins, pc, stack_read_value, previous_ram_read); }
	else if (opc == 0x013 /* STR   */) { opc_result = str1(phase, ins, pc, stack_read_value); }
	else if (opc == 0x033 /* STR2  */) { opc_result = str2(phase, ins, pc, stack_read_value); }
	else if (opc == 0x014 /* LDA   */) { opc_result = lda(phase, ins, stack_read_value, previous_ram_read); }
	else if (opc == 0x034 /* LDA2  */) { opc_result = lda2(phase, ins, stack_read_value, previous_ram_read); }
	else if (opc == 0x015 /* STA   */) { opc_result = sta(phase, ins, stack_read_value); }
	else if (opc == 0x035 /* STA2  */) { opc_result = sta2(phase, ins, stack_read_value); }
	else if (opc == 0x016 /* DEI   */) { opc_result = dei(phase, ins, stack_read_value, device_ram_read_value); }
	else if (opc == 0x036 /* DEI2  */) { opc_result = dei2(phase, ins, stack_read_value, device_ram_read_value); }
	else if (opc == 0x017 /* DEO   */) { opc_result = deo(phase, ins, stack_read_value, device_ram_read_value); }
	else if (opc == 0x037 /* DEO2  */) { opc_result = deo2(phase, ins, stack_read_value, device_ram_read_value); }
	else if (opc == 0x018 /* ADD   */) { opc_result = add(phase, ins, stack_read_value); }
	else if (opc == 0x038 /* ADD2  */) { opc_result = add2(phase, ins, stack_read_value); }
	else if (opc == 0x019 /* SUB   */) { opc_result = sub(phase, ins, stack_read_value); }
	else if (opc == 0x039 /* SUB2  */) { opc_result = sub2(phase, ins, stack_read_value); }
	else if (opc == 0x01A /* MUL   */) { opc_result = mul(phase, ins, stack_read_value); }
	else if (opc == 0x03A /* MUL2  */) { opc_result = mul2(phase, ins, stack_read_value); }
	else if (opc == 0x01B /* DIV   */) { opc_result = div(phase, ins, stack_read_value); }
	else if (opc == 0x03B /* DIV2  */) { opc_result = div2(phase, ins, stack_read_value); }
	else if (opc == 0x01C /* AND   */) { opc_result = and(phase, ins, stack_read_value); }
	else if (opc == 0x03C /* AND2  */) { opc_result = and2(phase, ins, stack_read_value); }
	else if (opc == 0x01D /* ORA   */) { opc_result = ora(phase, ins, stack_read_value); }
	else if (opc == 0x03D /* ORA2  */) { opc_result = ora2(phase, ins, stack_read_value); }
	else if (opc == 0x01E /* EOR   */) { opc_result = eor(phase, ins, stack_read_value); }
	else if (opc == 0x03E /* EOR2  */) { opc_result = eor2(phase, ins, stack_read_value); }
	else if (opc == 0x01F /* SFT   */) { opc_result = sft(phase, ins, stack_read_value); }
	else if (opc == 0x03F /* SFT2  */) { opc_result = sft2(phase, ins, stack_read_value); }
	else { printf("************\n ERR 0x%X \n************\n", opc); }
	
	if (opc_result.is_sp_shift) {
		if (stack_index ^ opc_result.is_stack_index_flipped) {
			sp1 += opc_result.sp_relative_shift;
		} else {
			sp0 += opc_result.sp_relative_shift;
		}
	}
	
	is_stack_read = opc_result.is_stack_read;
	is_stack_write = opc_result.is_stack_write;
	stack_address = (stack_index ^ opc_result.is_stack_index_flipped ? sp1 : sp0) - opc_result.stack_address_sp_offset;
	stack_write_value = opc_result.stack_value;
	
	stack_read_value = stack_ram_update(
		~(stack_index ^ opc_result.is_stack_index_flipped),
		stack_address, 
		stack_write_value,
		is_stack_write, // write 0 enable
		0,				// read 0 enable
		stack_address,
		is_stack_read
	);
	
	device_ram_read_value = device_ram_update(
		opc_result.device_ram_address,
		opc_result.device_ram_value,
		opc_result.is_device_ram_write,
		0,
		opc_result.device_ram_address,
		opc_result.is_device_ram_read
	);
	
	opc_eval_result.is_pc_updated = opc_result.is_pc_updated;
	opc_eval_result.pc = opc_result.pc;
	opc_eval_result.is_ram_read = opc_result.is_ram_read;
	opc_eval_result.is_ram_write = opc_result.is_ram_write;
	opc_eval_result.ram_addr = opc_result.ram_addr;
	opc_eval_result.ram_value = opc_result.ram_value;
	opc_eval_result.is_vram_write = opc_result.is_vram_write;
	opc_eval_result.vram_address = opc_result.vram_address;
	opc_eval_result.vram_value = opc_result.vram_value;
	opc_eval_result.is_opc_done = opc_result.is_opc_done;
	
	return opc_eval_result;
}
