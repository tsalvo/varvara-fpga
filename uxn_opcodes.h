#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#include <stdint.h>
#pragma once
#include "intN_t.h"   // intN_t types for any N

#pragma once
#include "uxn_stack.h"
#pragma once
#include "uxn_device.h"
#pragma once
#include "uxn_constants.h"

/* Registers
[ Z ][ Y ][ X ][ L ][ N ][ T ] <
[ . ][ . ][ . ][   H2   ][ . ] <
[   L2   ][   N2   ][   T2   ] <
*/

typedef struct opcode_result_t {
	uint1_t is_pc_updated;
	
	uint1_t is_stack_index_flipped;
	
	uint1_t is_sp_shift;
	int4_t sp_relative_shift; // updated stack pointer value
	
	uint1_t is_stack_operation_16bit;
	uint1_t is_stack_write;
	uint4_t stack_address_sp_offset;
	
	uint1_t is_ram_write;
	
	uint1_t is_device_ram_write;
	uint8_t device_ram_address;
	
	uint1_t is_vram_write;
	uint1_t vram_write_layer;
	uint24_t vram_address;

	uint8_t u8_value; // for 8-bit stack writes, ram_value, vram_value, device_ram_value
	uint16_t u16_value; // for pc value, 16-bit stack writes, and ram address
		
	uint1_t is_opc_done;
} opcode_result_t;

typedef struct eval_opcode_result_t {
	uint1_t is_waiting;
	uint1_t is_pc_updated;
	
	uint1_t is_ram_write;
	
	uint1_t is_vram_write;
	uint1_t vram_write_layer;
	uint24_t vram_address;
	
	uint1_t is_device_ram_write;
	uint8_t device_ram_address;
	
	uint8_t u8_value; // for ram_value, vram_value, device_ram_value
	uint16_t u16_value; // for pc value and ram address
	
	uint1_t is_opc_done;
} eval_opcode_result_t;

int4_t sp_relative_shift(uint8_t ins, int4_t x, int4_t y) {
	return (((ins & 0x80) > 0) ? x + y : y);
}

opcode_result_t jci(uint8_t phase, uint16_t pc, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// t=T;           SHIFT(-1) if(!t) { pc += 2; break; } else { rr = ram + pc; pc += PEEK2(rr) + 2; }
	static uint16_t tmp16;
	static uint8_t t8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** JCI ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
		result.is_sp_shift = 1;
		result.sp_relative_shift = -1;
		result.u16_value = pc;     // peek RAM (byte 1 of 2) at address equal to PC
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_sp_shift = 0;
		result.u16_value = pc + 1; // peek RAM (byte 2 of 2) at address equal to PC + 1
	}
	else if (phase == 3) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
	}
	else if (phase == 4) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_pc_updated = 1;
		result.u16_value = t8 == 0 ? pc + 2 : (pc + tmp16 + 2);
	}
	else if (phase == 5) {
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
		#if DEBUG
		printf("************\n**** JMI ***\n************\n");
		#endif
		result.u16_value = pc;     // peek RAM (byte 1 of 2) at address equal to PC
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.u16_value = pc + 1; // peek RAM (byte 2 of 2) at address equal to PC + 1
	}
	else if (phase == 2) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
	}
	else if (phase == 3) {
		tmp16 |= (uint16_t)(previous_ram_read);
		result.is_pc_updated = 1;
		result.u16_value = (pc + tmp16 + 2);
	}
	else if (phase == 4) {
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
		#if DEBUG
		printf("************\n**** JSI ***\n************\n");
		#endif
		result.is_sp_shift = 1;
		result.sp_relative_shift = 2;
		result.is_stack_write = 1;
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = pc + 2;	// set T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 0;
		result.is_stack_write = 0;
		result.is_stack_operation_16bit = 0;
		result.u16_value = pc;     // peek RAM (byte 1 of 2) at address equal to PC
	}
	else if (phase == 2) {
		result.u16_value = pc + 1; // peek RAM (byte 2 of 2) at address equal to PC + 1
	}
	else if (phase == 3) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
	}
	else if (phase == 4) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_pc_updated = 1;
		result.u16_value = pc + tmp16 + 2; // pc += PEEK2_RAM(pc) + 2
	}
	else if (phase == 5) {
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lit(uint8_t phase, uint16_t pc, uint8_t previous_ram_read) {
	// SHIFT( 1) T = ram[pc++];
	static opcode_result_t result;
	
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** LIT ***\n************\n");
		#endif
		result.is_sp_shift = 1;
		result.sp_relative_shift = 1; 		// shift(1)
		result.u16_value = pc; // peek RAM at at address equal to PC
		result.is_opc_done = 0;
	} 
	else if (phase == 1) {
		result.is_sp_shift = 0;
		result.is_pc_updated = 1;
		result.u16_value = pc + 1; // pc += 1
	}
	else if (phase == 2) {
		result.is_pc_updated = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = previous_ram_read; // T = PEEK_RAM(pc);
	}
	else if (phase == 3) {
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
		#if DEBUG
		printf("************\n*** LIT2 ***\n************\n");
		#endif
		result.is_sp_shift = 1;
		result.sp_relative_shift = 2;
		result.u16_value = pc;     // peek RAM (byte 1 of 2) at address equal to PC
		result.is_opc_done = 0;
	} 
	else if (phase == 1) {
		result.is_sp_shift = 0;
		result.u16_value = pc + 1; // peek RAM (byte 2 of 2) at address equal to PC + 1
	}
	else if (phase == 2) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
		result.is_pc_updated = 1;
		result.u16_value = pc + 2; // pc += 2
	}
	else if (phase == 3) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_pc_updated = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.is_stack_operation_16bit = 1;
		result.u16_value = tmp16;	// set T2
	}
	else if (phase == 4) {
		result.is_stack_write = 0;
		result.is_stack_operation_16bit = 0;
		result.is_opc_done = 1;
	}
		
	return result;
}

opcode_result_t pop(uint8_t phase, uint8_t ins) {
	// SET(1,-1)
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** POP ***\n************\n");
		#endif
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 1, -1);
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
		#if DEBUG
		printf("************\n*** POP2 ***\n************\n");
		#endif
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
		result.is_opc_done = 0;
	}
	else {
		result.is_sp_shift = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ovr(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=N;        SET(2, 1) T = n; N = t; L = n;
	static uint8_t n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** OVR ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, 1);
	}
	else if (phase == 2) {
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 3; // set H2 a.k.a. [L][N] to [n8][t8]
		result.u16_value = previous_stack_read;
	}
	else if (phase == 3) {
		result.is_stack_operation_16bit = 0;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8;  // set T to n8
	}
	else if (phase == 4) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ovr2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;n=N2;      SET(4, 2) T2_(n) N2_(t) L2_(n) break;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** OVR2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 4, 2);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 4;
		result.u16_value = t16; // set N2 = previous T2
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u16_value = n16; // set T2 = previous N2
	}
	else if (phase == 4) {		
		result.stack_address_sp_offset = 6;
		result.u16_value = n16;	// set L2 = previous N2
	}
	else if (phase == 5) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t dei(uint8_t phase, uint8_t ins, uint8_t previous_stack_read, uint8_t previous_device_ram_read) {
	// t=T;            SET(1, 0) T = DEI(t);
	static uint1_t has_written_to_t;
	static uint8_t t8;
	static device_in_result_t device_in_result;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** DEI ***\n************\n");
		#endif
		has_written_to_t = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
		result.is_device_ram_write = 0;
		device_in_result.is_dei_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 1, 0);
	}
	else {
		result.is_sp_shift = 0;
		t8 = (phase == 2) ? previous_stack_read : t8;
		if (~device_in_result.is_dei_done) {
			device_in_result = device_in(t8, phase - 2, previous_device_ram_read);
			result.device_ram_address = device_in_result.device_ram_address;
		} else {
			if (~has_written_to_t) {
				result.is_stack_write = 1;
				result.stack_address_sp_offset = 1;
				result.u8_value = device_in_result.dei_value;
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
		#if DEBUG
		printf("************\n*** DEI2 ***\n************\n");
		#endif
		has_written_to_t = 0;
		has_written_to_n = 0;
		is_first_dei_done = 0;
		is_second_dei_done = 0;
		current_dei_phase = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_device_ram_write = 0;
		result.is_opc_done = 0;
		device_in_result.is_dei_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 1, 1);
	}
	else {
		result.is_sp_shift = 0;
		t8 = (phase == 2) ? previous_stack_read : t8;
		dei_param = is_first_dei_done ? t8 + 1 : t8;
		if (~is_first_dei_done | (has_written_to_t & ~is_second_dei_done)) {
			device_in_result = device_in(dei_param, current_dei_phase, previous_device_ram_read);
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
			result.u8_value = device_in_result.dei_value;
			has_written_to_t = 1;
		}
		else if (~has_written_to_n) {
			result.stack_address_sp_offset = 2;
			result.u8_value = device_in_result.dei_value;
			has_written_to_n = 1;
		}
		else {
			result.is_stack_write = 0;
			result.is_opc_done = 1;
		}
	}
	
	return result;
}

opcode_result_t deo(uint8_t phase, uint8_t ins, uint16_t previous_stack_read, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	// t=T;n=N;        SET(2,-2) DEO(t, n)
	static uint8_t t8, n8;
	static opcode_result_t result;
	static device_out_result_t device_out_result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** DEO ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
	}
	else {
		t8 = (phase == 2) ? (uint8_t)(previous_stack_read) : t8;
		n8 = (phase == 2) ? (uint8_t)(previous_stack_read >> 8) : n8;
		result.is_sp_shift = 0;
		device_out_result = device_out(t8, n8, phase - 2, previous_device_ram_read, previous_ram_read);
		result.is_device_ram_write = device_out_result.is_device_ram_write;
		result.device_ram_address = device_out_result.device_ram_address;
		result.u8_value = device_out_result.u8_value;
		result.u16_value = device_out_result.ram_address;
		result.is_vram_write = device_out_result.is_vram_write;
		result.vram_write_layer = device_out_result.vram_write_layer;
		result.vram_address = device_out_result.vram_address;
		result.is_opc_done = device_out_result.is_deo_done;
	}
	
	return result;
}

opcode_result_t deo2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	// t=T;n=N;l=L;    SET(3,-3) DEO(t, l) DEO((t + 1), n)
	static uint8_t t8, n8, l8, current_deo_phase, deo_param0, deo_param1;
	static uint1_t is_second_deo = 0;
	static opcode_result_t result;
	static device_out_result_t device_out_result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** DEO2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
		is_second_deo = 0;
		current_deo_phase = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 3; // get L
		result.is_stack_operation_16bit = 0;
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.sp_relative_shift = sp_relative_shift(ins, 3, -3); 
		result.is_sp_shift = 1;
	}
	else {
		l8 = (phase == 3) ? ((uint8_t)(previous_stack_read)) : l8;
		deo_param0 = is_second_deo ? t8 + 1 : t8;
		deo_param1 = is_second_deo ? n8 : l8;
		result.is_sp_shift = 0;		
		device_out_result = device_out(deo_param0, deo_param1, current_deo_phase, previous_device_ram_read, previous_ram_read);
		result.is_device_ram_write = device_out_result.is_device_ram_write;
		result.device_ram_address = device_out_result.device_ram_address;
		result.is_vram_write = device_out_result.is_vram_write;
		result.vram_write_layer = device_out_result.vram_write_layer;
		result.vram_address = device_out_result.vram_address;
		result.u16_value = device_out_result.ram_address;
		result.u8_value = device_out_result.u8_value;
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
		#if DEBUG
		printf("************\n**** JMP ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 1, -1);
	}
	else if (phase == 2) {
		t8 = (int8_t)(previous_stack_read);
		result.is_sp_shift = 0;
		result.is_pc_updated = 1;
		result.u16_value = pc + t8;
	}
	else if (phase == 3) {
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t jmp2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;           SET(2,-2) pc = t
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** JMP2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
		result.is_stack_operation_16bit = 0;
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_pc_updated = 1;
		result.u16_value = t16; // pc = t16
	}
	else if (phase == 3) {
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}

	return result;
}

opcode_result_t jcn(uint8_t phase, uint8_t ins, uint16_t pc, uint16_t previous_stack_read) {
	// t=T;n=N;        SET(2,-2) if(n) pc += (Sint8)t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** JCN ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_pc_updated = 1;
		result.u16_value = n8 == 0 ? pc : pc + (int8_t)(t8); // if(n) pc += (Sint8)t;
	}
	else if (phase == 3) {
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t jcn2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;n=L;       SET(3,-3) if(n) pc = t;
	static uint16_t t16;
	static uint8_t n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** JCN2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 3; // get L
		result.is_stack_operation_16bit = 0;
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 3, -3);
	}
	else if (phase == 3) {
		n8 = (uint8_t)(previous_stack_read);
		result.is_sp_shift = 0;
		result.is_pc_updated = n8 == 0 ? 0 : 1;
		result.u16_value = n8 == 0 ? 0 : t16; // if(n) pc = t;
	}
	else if (phase == 4) {
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
		#if DEBUG
		printf("************\n**** JSR ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 1, -1);
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_index_flipped = 1;
		result.sp_relative_shift = 2;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.is_stack_operation_16bit = 1;
		result.u16_value = pc;  // set T2
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.is_stack_write = 0;
		result.is_stack_operation_16bit = 0;
		result.is_pc_updated = 1;
		result.u16_value = pc + (int8_t)(t8); // pc += t
	}
	else if (phase == 4) {
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t jsr2(uint8_t phase, uint8_t ins, uint16_t pc, uint16_t previous_stack_read) {
	// t=T2;           SET(2,-2) FLIP SHIFT(2) T2_(pc) pc = t;
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** JSR2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_stack_index_flipped = 1;
		result.sp_relative_shift = 2;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = pc;		// set T2
	}
	else if (phase == 3) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_sp_shift = 0;
		result.is_pc_updated = 1;
		result.u16_value = t16; // pc = t16
	}
	else if (phase == 4) {
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t add(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n + t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** ADD ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 + t8;	// set T
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t add2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-2) T2_(n + t) 
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** ADD2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		tmp16 = n16 + t16;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = tmp16;	// set T2
	}
	else if (phase == 4) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t and(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	//  t=T;n=N;        SET(2,-1) T = n & t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** AND ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 & t8;	// set T
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t and2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-2) T2_(n & t)
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** AND2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2; // set T2 
		result.u16_value = t16 & n16;
	}
	else if (phase == 4) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ora(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	//  t=T;n=N;        SET(2,-1) T = n | t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** ORA ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 | t8;	// set T
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ora2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-2) T2_(n | t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** ORA2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		tmp16 = n16 | t16;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = tmp16;	// set T2
	}
	else if (phase == 4) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t eor(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n ^ t; break;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** EOR ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 ^ t8;	// set T
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t eor2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-2) T2_(n ^ t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** EOR2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		tmp16 = n16 ^ t16;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = tmp16;	// set T2
	}
	else if (phase == 4) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t equ(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	//  t=T;n=N;        SET(2,-1) T = n == t; 
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** EQU ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 == t8 ? 1 : 0;	// set T
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t equ2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-3) T = n == t;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** EQU2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.is_stack_operation_16bit = 0;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -3);
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n16 == t16 ? 1 : 0;	// set T
	}
	else if (phase == 4) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t neq(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	//  t=T;n=N;        SET(2,-1) T = n != t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** NEQ ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 == t8 ? 0 : 1;	// set T
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t neq2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-3) T = n != t;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** NEQ2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.is_stack_operation_16bit = 0;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -3);
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n16 == t16 ? 0 : 1;	// set T
	}
	else if (phase == 4) {
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
		#if DEBUG
		printf("************\n*** INC ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 1, 0);
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = t8 + 1;	// set T 
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t inc2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	//  t=T2;           SET(2, 0) T2_(t + 1)
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** INC2 ***\n************\n");
		#endif
		result.stack_address_sp_offset = 2; // get T2
		result.is_stack_operation_16bit = 1;
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, 0);
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = t16 + 1;	// set T2
	}
	else if (phase == 3) {
		result.is_stack_operation_16bit = 0;
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
		#if DEBUG
		printf("************\n**** LDZ ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 1, 0);
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_sp_shift = 0;
		result.u16_value = (uint16_t)(t8); // peek RAM at address equal to T
	}
	// PHASE 3 - Do Nothing
	else if (phase == 4) {
		tmp8 = previous_ram_read;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp8;	// set T
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
		#if DEBUG
		printf("************\n*** LDZ2 ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 1, 1);
		result.u16_value = (uint16_t)(t8);     // peek RAM at address equal to T
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.u16_value = (uint16_t)(t8 + 1); // peek RAM at address equal to T + 1
	}
	else if (phase == 4) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
	}
	else if (phase == 5) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_stack_write = 1;
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = tmp16;	// set T2
	}
	else if (phase == 6) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t stz(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=N;        SET(2,-2) ram[t] = n;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** STZ ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_ram_write = 1;
		result.u16_value = (uint16_t)(t8);
		result.u8_value = n8; // set first byte of n16 to ram address t8 
	}
	else if (phase == 3) {
		result.is_ram_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t stz2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=H2;       SET(3,-3) rr = ram + t; POKE2(rr, n)
	static uint8_t t8;
	static uint16_t n16;
	static opcode_result_t result;
	
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** STZ2 ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 3; // get H2
		result.is_stack_operation_16bit = 1;
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		result.is_stack_operation_16bit = 0;
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 3, -3);
		result.is_ram_write = 1;
		result.u16_value = (uint16_t)(t8);
		result.u8_value = (uint8_t)(n16 >> 8); // set first byte of n16 to ram address t8 
	}
	else if (phase == 4) {
		result.is_sp_shift = 0;
		result.u16_value = (uint16_t)(t8 + 1);
		result.u8_value = (uint8_t)(n16); // set second byte of n16 to ram address t8 + 1 
	}
	else if (phase == 5) {
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
		#if DEBUG
		printf("************\n**** LDR ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 1, 0);
		result.u16_value = pc + (int8_t)(t8); // peek RAM at address equal to  PC + T 
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
	}
	else if (phase == 4) {
		tmp8 = previous_ram_read;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp8;	// set T
	}
	else if (phase == 5) {
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
		#if DEBUG
		printf("************\n*** LDR2 ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 1, 1);
		result.u16_value = pc + (int8_t)(t8);     // peek RAM (byte 1 of 2) at address equal to PC + T 
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.u16_value = pc + (int8_t)(t8) + 1; // peek RAM (byte 2 of 2) at address equal to PC + T + 1
	}
	else if (phase == 4) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
	}
	else if (phase == 5) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.is_stack_operation_16bit = 1;
		result.u16_value = tmp16;	// set T2
	}
	else if (phase == 6) {
		result.is_stack_write = 0;
		result.is_stack_operation_16bit = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t str1(uint8_t phase, uint8_t ins, uint16_t pc, uint16_t previous_stack_read) {
	// t=T;n=N;        SET(2,-2) ram[pc + (Sint8)t] = n;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** STR ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_ram_write = 1;
		result.u16_value = pc + (int8_t)(t8);
		result.u8_value = n8; // set first n8 to ram address t8 
	}
	else if (phase == 3) {
		result.is_ram_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t str2(uint8_t phase, uint8_t ins, uint16_t pc, uint16_t previous_stack_read) {
	// t=T;n=H2;       SET(3,-3) rr = ram + pc + (Sint8)t; POKE2(rr, n)
	static uint8_t t8;
	static uint16_t n16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** STR2 ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 3; // get H2
		result.is_stack_operation_16bit = 1;
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		result.is_stack_operation_16bit = 0;
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 3, -3);
		result.is_ram_write = 1;
		result.u16_value = pc + (int8_t)(t8);
		result.u8_value = (uint8_t)(n16 >> 8); // set first byte of n16 to ram address pc + t8 
	}
	else if (phase == 4) {
		result.is_sp_shift = 0;
		result.u16_value = pc + (int8_t)(t8) + 1;
		result.u8_value = (uint8_t)(n16); // set second byte of n16 to ram address pc + t8 + 1 
	}
	else if (phase == 5) {
		result.is_ram_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lda(uint8_t phase, uint8_t ins, uint16_t previous_stack_read, uint8_t previous_ram_read) {
	// t=T2;           SET(2,-1) T = ram[t];
	static uint16_t t16;
	static uint8_t tmp8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** LDA ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_stack_operation_16bit = 0;
		result.u16_value = t16; // peek RAM at address equal to T2
	}
	// PHASE 3 - do nothing
	else if (phase == 4) {
		tmp8 = previous_ram_read;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp8;	// set T
	}
	else if (phase == 5) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lda2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read, uint8_t previous_ram_read) {
	// t=T2;           SET(2, 0) rr = ram + t; T2_(PEEK2(rr))
	static uint16_t t16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** LDA2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, 0);
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_stack_operation_16bit = 0;
		result.u16_value = t16;     // peek RAM (byte 1 of 2) at address equal to t16
	}
	else if (phase == 3) {
		result.u16_value = t16 + 1; // peek RAM (byte 2 of 2) at address equal to t16 + 1
	}
	else if (phase == 4) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
	}
	else if (phase == 5) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_stack_operation_16bit = 1;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2; // set T2
		result.u16_value = tmp16;
	}
	else if (phase == 6) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t gth(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n > t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** GTH ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 > t8 ? 1 : 0;	// set T
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t gth2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-3) T = n > t;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** GTH2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -3);
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n16 > t16 ? 1 : 0;	// set T
	}
	else if (phase == 4) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lth(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n < t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** LTH ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 < t8 ? 1 : 0;	// set T
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lth2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-3) T = n < t;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** LTH2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -3);
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n16 < t16 ? 1 : 0;	// set T
	}
	else if (phase == 4) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t mul(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n * t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** MUL ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 * t8;	// set T
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t mul2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-2) T2_(n * t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** MUL2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		tmp16 = n16 * t16;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = tmp16;	// set T2
	}
	else if (phase == 4) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t div(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = t ? n / t : 0;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** DIV ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = t8 == 0 ? 0 : n8 / t8;	// set T
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t div2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-2) T2_(t ? n / t : 0)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** DIV2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		tmp16 = t16 == 0 ? 0 : n16 / t16;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = tmp16;	// set T2
	}
	else if (phase == 4) {
		result.is_stack_operation_16bit = 0;
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
		#if DEBUG
		printf("************\n**** NIP ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = t8;	// set T
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t nip2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;           SET(4,-2) T2_(t)
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** NIP2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = t16;	// set T2
	}
	else if (phase == 3) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
		
	return result;
}


opcode_result_t sft(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n >> (t & 0xf) << (t >> 4);
	static uint8_t t8, n8, tmp8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** SFT ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		tmp8 = (n8 >> (t8 & 0x0F)) << (t8 >> 4);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp8;	// set T
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sft2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=H2;       SET(3,-1) T2_(n >> (t & 0xf) << (t >> 4))
	static uint8_t t8;
	static uint16_t n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** SFT2 ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 3;  // get H2
	}
	else if (phase == 2) {
		t8 = (uint8_t)previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 3, -1);
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		tmp16 = (n16 >> (t8 & 0x0F)) << (t8 >> 4);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = tmp16;	// set T2
	} 
	else if (phase == 4) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sta(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;n=L;       SET(3,-3) ram[t] = n;
	static uint16_t t16;
	static uint8_t n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** STA ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.stack_address_sp_offset = 3;  // get L
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 3, -3);
	}
	else if (phase == 3) {
		n8 = (uint8_t)(previous_stack_read);
		result.is_sp_shift = 0;
		result.is_ram_write = 1;
		result.u16_value = t16; // poke RAM at address equal to T2
		result.u8_value = n8;
	}
	else if (phase == 4) {
		result.is_ram_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sta2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-4) rr = ram + t; POKE2(rr, n)
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** STA2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 4; // get N2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get T2
	}
	else if (phase == 2) {
		n16 = previous_stack_read;
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -4);
	}
	else if (phase == 3) {
		t16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_ram_write = 1;
		result.u16_value = t16;
		result.u8_value = (uint8_t)(n16 >> 8); // set first byte of n16 to ram address t16 
	}
	else if (phase == 4) {
		result.u16_value = t16 + 1;
		result.u8_value = (uint8_t)(n16); // set second byte of n16 to ram address t16 + 1 
	}
	else if (phase == 5) {
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
		#if DEBUG
		printf("************\n**** STH ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 1, -1);
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_index_flipped = 1;
		result.sp_relative_shift = 1;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = t8; // set T
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.is_stack_write = 0;
		result.is_stack_index_flipped = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sth2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;           SET(2,-2) FLIP SHIFT(2) T2_(t)
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** STH2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_stack_index_flipped = 1;
		result.sp_relative_shift = 2;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = t16;	// set T2
	}
	else if (phase == 3) {
		result.is_sp_shift = 0;
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_stack_index_flipped = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sub(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n - t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** SUB ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_operation_16bit = 0;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 - t8;	// set T
	}
	else if (phase == 3) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sub2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-2) T2_(n - t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** SUB2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		tmp16 = n16 - t16;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = tmp16;	// set T2
	}
	else if (phase == 4) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t swp(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=N;        SET(2, 0) T = n; N = t;
	static uint8_t t8, n8;
	static uint16_t tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** SWP ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, 0);
	}
	else if (phase == 2) {
		t8 = (uint8_t)(previous_stack_read);
		n8 = (uint8_t)(previous_stack_read >> 8);
		tmp16 = (uint16_t)(t8);
		tmp16 <<= 8;
		tmp16 |= (uint16_t)(n8);
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = tmp16;	// set T2 a.k.a. [N][T] to [t8][n8]
	}
	else if (phase == 3) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t swp2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;n=N2;      SET(4, 0) T2_(n) N2_(t)
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** SWP2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 4, 0);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 4;
		result.u16_value = t16; // set N2
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u16_value = n16; // set T2
	}
	else if (phase == 4) {
		result.is_stack_write = 0;
		result.is_stack_operation_16bit = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t rot(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T;n=N;l=L;    SET(3, 0) T = l; N = t; L = n;
	static uint16_t tmp16;
	static uint8_t t8, n8, l8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** ROT ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2 a.k.a. [N][T]
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 3; // get L
		result.is_stack_operation_16bit = 0;
	}
	else if (phase == 2) {
		tmp16 = previous_stack_read; // T2 a.k.a [n8][t8]
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 3, 0);
		result.is_stack_operation_16bit = 1;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 3; // set H2 a.k.a [L][N] to [n8][t8]
		result.u16_value = tmp16;
	}
	else if (phase == 3) {
		l8 = (uint8_t)(previous_stack_read);
		result.is_sp_shift = 0;
		result.is_stack_operation_16bit = 0;
		result.stack_address_sp_offset = 1;
		result.u8_value = l8;	// set T = l8
	}
	else if (phase == 4) {
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t rot2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;n=N2;l=L2; SET(6, 0) T2_(l) N2_(t) L2_(n)
	static uint16_t t16, n16, l16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** ROT2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 4; // get N2
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.stack_address_sp_offset = 6; // get L2
	}
	else if (phase == 3) {
		n16 = previous_stack_read;
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 6, 0);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 4;
		result.u16_value = t16;	// set N2
	}
	else if (phase == 4) {
		l16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u16_value = l16;	// set T2
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 6;
		result.u16_value = n16; // set L2
	}
	else if (phase == 6) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t dup(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;            SET(1, 1) T = t; N = t;
	static uint8_t t8;
	static uint16_t tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** DUP ***\n************\n");
		#endif
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 1, 1);
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		tmp16 = (uint16_t)(t8);
		tmp16 <<= 8;
		tmp16 |= (uint16_t)(t8);
		result.is_sp_shift = 0;
		result.is_stack_operation_16bit = 1;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = tmp16;	// set T2 = [t8][t8] a.k.a. T=t8 and N=t8
	}
	else if (phase == 3) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t dup2(uint8_t phase, uint8_t ins, uint16_t previous_stack_read) {
	// t=T2;           SET(2, 2) T2_(t) N2_(t) break;
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** DUP2 ***\n************\n");
		#endif
		result.is_stack_operation_16bit = 1;
		result.stack_address_sp_offset = 2; // get T2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_shift = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, 2);
	}
	else if (phase == 2) {
		t16 = previous_stack_read;
		result.is_sp_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u16_value = t16;	// set T2
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 4;
		result.u16_value = t16; // set N2
	}
	else if (phase == 4) {
		result.is_stack_operation_16bit = 0;
		result.is_stack_write = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

eval_opcode_result_t eval_opcode_phased(
	uint8_t phase,
	uint8_t ins,
	uint16_t pc,
	uint8_t previous_ram_read,
	uint8_t previous_device_ram_read
) {
	static uint8_t sp0, sp1;
	static uint12_t opc;
	static uint1_t stack_index, is_wait;
	static uint12_t stack_address = 0;
	static uint16_t previous_stack_read = 0;
	static opcode_result_t opc_result = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static eval_opcode_result_t opc_eval_result = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	opc = ((ins & 0x1F) > 0) ? ((uint12_t)(ins & 0x3F)) : ((uint12_t)(ins) << 4);
	is_wait = 0;
	#if DEBUG
	printf("        EVAL OPCODE: INS = 0x%X, OPC = 0x%X, phase = 0x%X\n", ins, opc, phase);
	#endif
	
	if      (opc == 0x000 /* BRK   */) { is_wait = 1; opc_result.is_opc_done = 1; }
	else if (opc == 0x200 /* JCI   */) { opc_result = jci(phase, pc, previous_stack_read, previous_ram_read); }
	else if (opc == 0x400 /* JMI   */) { opc_result = jmi(phase, pc, previous_ram_read); }
	else if (opc == 0x600 /* JSI   */) { opc_result = jsi(phase, pc, previous_ram_read); }
	else if (opc == 0x800 /* LIT   */) { opc_result = lit(phase, pc, previous_ram_read); }
	else if (opc == 0xA00 /* LIT2  */) { opc_result = lit2(phase, pc, previous_ram_read); }
	else if (opc == 0xC00 /* LITr  */) { opc_result = lit(phase, pc, previous_ram_read); }
	else if (opc == 0xE00 /* LIT2r */) { opc_result = lit2(phase, pc, previous_ram_read); }
	else if (opc == 0x001 /* INC   */) { opc_result = inc(phase, ins, previous_stack_read); }
	else if (opc == 0x021 /* INC2  */) { opc_result = inc2(phase, ins, previous_stack_read); }
	else if (opc == 0x002 /* POP   */) { opc_result = pop(phase, ins); }
	else if (opc == 0x022 /* POP2  */) { opc_result = pop2(phase, ins); }
	else if (opc == 0x003 /* NIP   */) { opc_result = nip(phase, ins, previous_stack_read); }
	else if (opc == 0x023 /* NIP2  */) { opc_result = nip2(phase, ins, previous_stack_read); }
	else if (opc == 0x004 /* SWP   */) { opc_result = swp(phase, ins, previous_stack_read); }
	else if (opc == 0x024 /* SWP2  */) { opc_result = swp2(phase, ins, previous_stack_read); }
	else if (opc == 0x005 /* ROT   */) { opc_result = rot(phase, ins, previous_stack_read); }
	else if (opc == 0x025 /* ROT2  */) { opc_result = rot2(phase, ins, previous_stack_read); }
	else if (opc == 0x006 /* DUP   */) { opc_result = dup(phase, ins, previous_stack_read); }
	else if (opc == 0x026 /* DUP2  */) { opc_result = dup2(phase, ins, previous_stack_read); }
	else if (opc == 0x007 /* OVR   */) { opc_result = ovr(phase, ins, previous_stack_read); }
	else if (opc == 0x027 /* OVR2  */) { opc_result = ovr2(phase, ins, previous_stack_read); }
	else if (opc == 0x008 /* EQU   */) { opc_result = equ(phase, ins, previous_stack_read); }
	else if (opc == 0x028 /* EQU2  */) { opc_result = equ2(phase, ins, previous_stack_read); }
	else if (opc == 0x009 /* NEQ   */) { opc_result = neq(phase, ins, previous_stack_read); }
	else if (opc == 0x029 /* NEQ2  */) { opc_result = neq2(phase, ins, previous_stack_read); }
	else if (opc == 0x00A /* GTH   */) { opc_result = gth(phase, ins, previous_stack_read); }
	else if (opc == 0x02A /* GTH2  */) { opc_result = gth2(phase, ins, previous_stack_read); }
	else if (opc == 0x00B /* LTH   */) { opc_result = lth(phase, ins, previous_stack_read);  }
	else if (opc == 0x02B /* LTH2  */) { opc_result = lth2(phase, ins, previous_stack_read); }
	else if (opc == 0x00C /* JMP   */) { opc_result = jmp(phase, ins, pc, previous_stack_read); }
	else if (opc == 0x02C /* JMP2  */) { opc_result = jmp2(phase, ins, previous_stack_read); }
	else if (opc == 0x00D /* JCN   */) { opc_result = jcn(phase, ins, pc, previous_stack_read); }
	else if (opc == 0x02D /* JCN2  */) { opc_result = jcn2(phase, ins, previous_stack_read); }
	else if (opc == 0x00E /* JSR   */) { opc_result = jsr(phase, ins, pc, previous_stack_read); }
	else if (opc == 0x02E /* JSR2  */) { opc_result = jsr2(phase, ins, pc, previous_stack_read); }
	else if (opc == 0x00F /* STH   */) { opc_result = sth(phase, ins, previous_stack_read); }
	else if (opc == 0x02F /* STH2  */) { opc_result = sth2(phase, ins, previous_stack_read); }
	else if (opc == 0x010 /* LDZ   */) { opc_result = ldz(phase, ins, previous_stack_read, previous_ram_read); }
	else if (opc == 0x030 /* LDZ2  */) { opc_result = ldz2(phase, ins, previous_stack_read, previous_ram_read); }
	else if (opc == 0x011 /* STZ   */) { opc_result = stz(phase, ins, previous_stack_read); }
	else if (opc == 0x031 /* STZ2  */) { opc_result = stz2(phase, ins, previous_stack_read); }
	else if (opc == 0x012 /* LDR   */) { opc_result = ldr(phase, ins, pc, previous_stack_read, previous_ram_read); }
	else if (opc == 0x032 /* LDR2  */) { opc_result = ldr2(phase, ins, pc, previous_stack_read, previous_ram_read); }
	else if (opc == 0x013 /* STR   */) { opc_result = str1(phase, ins, pc, previous_stack_read); }
	else if (opc == 0x033 /* STR2  */) { opc_result = str2(phase, ins, pc, previous_stack_read); }
	else if (opc == 0x014 /* LDA   */) { opc_result = lda(phase, ins, previous_stack_read, previous_ram_read); }
	else if (opc == 0x034 /* LDA2  */) { opc_result = lda2(phase, ins, previous_stack_read, previous_ram_read); }
	else if (opc == 0x015 /* STA   */) { opc_result = sta(phase, ins, previous_stack_read); }
	else if (opc == 0x035 /* STA2  */) { opc_result = sta2(phase, ins, previous_stack_read); }
	else if (opc == 0x016 /* DEI   */) { opc_result = dei(phase, ins, previous_stack_read, previous_device_ram_read); }
	else if (opc == 0x036 /* DEI2  */) { opc_result = dei2(phase, ins, previous_stack_read, previous_device_ram_read); }
	else if (opc == 0x017 /* DEO   */) { opc_result = deo(phase, ins, previous_stack_read, previous_device_ram_read, previous_ram_read); }
	else if (opc == 0x037 /* DEO2  */) { opc_result = deo2(phase, ins, previous_stack_read, previous_device_ram_read, previous_ram_read); }
	else if (opc == 0x018 /* ADD   */) { opc_result = add(phase, ins, previous_stack_read); }
	else if (opc == 0x038 /* ADD2  */) { opc_result = add2(phase, ins, previous_stack_read); }
	else if (opc == 0x019 /* SUB   */) { opc_result = sub(phase, ins, previous_stack_read); }
	else if (opc == 0x039 /* SUB2  */) { opc_result = sub2(phase, ins, previous_stack_read); }
	else if (opc == 0x01A /* MUL   */) { opc_result = mul(phase, ins, previous_stack_read); }
	else if (opc == 0x03A /* MUL2  */) { opc_result = mul2(phase, ins, previous_stack_read); }
	else if (opc == 0x01B /* DIV   */) { opc_result = div(phase, ins, previous_stack_read); }
	else if (opc == 0x03B /* DIV2  */) { opc_result = div2(phase, ins, previous_stack_read); }
	else if (opc == 0x01C /* AND   */) { opc_result = and(phase, ins, previous_stack_read); }
	else if (opc == 0x03C /* AND2  */) { opc_result = and2(phase, ins, previous_stack_read); }
	else if (opc == 0x01D /* ORA   */) { opc_result = ora(phase, ins, previous_stack_read); }
	else if (opc == 0x03D /* ORA2  */) { opc_result = ora2(phase, ins, previous_stack_read); }
	else if (opc == 0x01E /* EOR   */) { opc_result = eor(phase, ins, previous_stack_read); }
	else if (opc == 0x03E /* EOR2  */) { opc_result = eor2(phase, ins, previous_stack_read); }
	else if (opc == 0x01F /* SFT   */) { opc_result = sft(phase, ins, previous_stack_read); }
	else if (opc == 0x03F /* SFT2  */) { opc_result = sft2(phase, ins, previous_stack_read); }
	
	stack_index = ((ins & 0x40) > 0) ? 1 : 0;
	stack_index ^= opc_result.is_stack_index_flipped;
	
	if (opc_result.is_sp_shift) {
		if (stack_index) {
			sp1 += ((int8_t)(opc_result.sp_relative_shift));
		} else {
			sp0 += ((int8_t)(opc_result.sp_relative_shift));
		}
	}
	
	stack_address = ((uint9_t)(stack_index ? sp1 : sp0)) - ((uint9_t)(opc_result.stack_address_sp_offset));
	stack_address += (stack_index ? 256 : 0);
	
	previous_stack_read = stack_ram_update(
		opc_result.is_stack_operation_16bit ? stack_address : 0,
		opc_result.is_stack_operation_16bit ? ((uint8_t)(opc_result.u16_value >> 8)) : 0,
		opc_result.is_stack_operation_16bit & opc_result.is_stack_write,
		opc_result.is_stack_operation_16bit ? (stack_address + 1) : stack_address,
		opc_result.is_stack_operation_16bit ? ((uint8_t)(opc_result.u16_value)) : opc_result.u8_value,
		opc_result.is_stack_write
	);
	
	opc_eval_result.is_waiting = is_wait;
	opc_eval_result.device_ram_address = opc_result.device_ram_address;
	opc_eval_result.is_device_ram_write = opc_result.is_device_ram_write;
	opc_eval_result.is_pc_updated = opc_result.is_pc_updated;
	opc_eval_result.u16_value = opc_result.u16_value;
	opc_eval_result.is_ram_write = opc_result.is_ram_write;
	opc_eval_result.is_vram_write = opc_result.is_vram_write;
	opc_eval_result.vram_write_layer = opc_result.vram_write_layer;
	opc_eval_result.vram_address = opc_result.vram_address;
	opc_eval_result.u8_value = opc_result.u8_value;
	opc_eval_result.is_opc_done = opc_result.is_opc_done;
	
	return opc_eval_result;
}
