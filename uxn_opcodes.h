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
	
	int4_t sp_relative_shift; // updated stack pointer value
	
	uint1_t is_stack_write;
	uint4_t stack_address_sp_offset;
	
	uint1_t is_ram_write;
	
	uint1_t is_device_ram_write;
	uint8_t device_ram_address;
	
	uint1_t is_vram_write;
	uint1_t vram_write_layer;

	uint8_t u8_value; // for stack_value, ram_value, vram_value, device_ram_value
	uint16_t u16_value; // for pc value, vram address write, or ram address read
		
	uint1_t is_opc_done;
} opcode_result_t;

typedef struct eval_opcode_result_t {
	uint1_t is_waiting;
	uint1_t is_pc_updated;
	
	uint1_t is_ram_write;
	
	uint1_t is_vram_write;
	uint1_t vram_write_layer;
	
	uint1_t is_device_ram_write;
	uint8_t device_ram_address;
	
	uint8_t u8_value; // for ram_value, vram_value, device_ram_value
	uint16_t u16_value; // for pc value, vram address write, or ram address read
	
	uint1_t is_opc_done;
} eval_opcode_result_t;

uint16_t u16_add_u8_as_i8(uint16_t u16, uint8_t u8) {
	uint1_t is_negative = u8(7);
	return is_negative ? (u16 - 0x0080 + (uint16_t)(u8 & 0x7F)) : (u16 + u8);
}

int4_t sp_relative_shift(uint8_t ins, int4_t x, int4_t y) {
	return (((ins & 0x80) != 0) ? x + y : y);
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
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.u16_value = pc;     // peek RAM (byte 1 of 2) at address equal to PC
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
		result.sp_relative_shift = -1;
		result.u16_value = pc + 1; // peek RAM (byte 2 of 2) at address equal to PC + 1
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
		result.sp_relative_shift = 0;
	}
	else if (phase == 3) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_pc_updated = 1;
		result.u16_value = t8 == 0 ? pc + 2 : (pc + tmp16 + 2);
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
		result.is_stack_write = 0;
		result.is_stack_index_flipped = 0;
		result.sp_relative_shift = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
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
		tmp16 = pc + 2;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.sp_relative_shift = 2; 		// shift(2)
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp16(7, 0);	// set T2 (low byte)
		result.u16_value = pc;     // peek RAM (byte 1 of 2) at address equal to PC
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u8_value = tmp16(15, 8); // set T2 (high byte)
		result.u16_value = pc + 1; // peek RAM (byte 2 of 2) at address equal to PC + 1
	}
	else if (phase == 2) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <<= 8;
		result.is_stack_write = 0;
	}
	else if (phase == 3) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_pc_updated = 1;
		result.u16_value = pc + tmp16 + 2; // pc += PEEK2_RAM(pc) + 2
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lit(uint8_t phase, uint16_t pc, uint8_t previous_ram_read) {
	// SHIFT( 1) T = ram[pc++];
	#if DEBUG
	printf("************\n**** LIT ***\n************\n");
	#endif
	opcode_result_t result;
	result.is_stack_index_flipped = 0;
	result.is_pc_updated = phase == 1 ? 1 : 0;
	result.is_ram_write = 0;
	result.is_vram_write = 0;
	result.sp_relative_shift = phase == 0 ? 1 : 0; 	// shift(1)
	result.stack_address_sp_offset = 1;
	result.is_stack_write = phase == 2 ? 1 : 0;
	result.u8_value = phase == 2 ? previous_ram_read : 0;
	result.u16_value = phase == 2 ? 0 : pc + phase; // peek RAM at at address equal to PC
	result.is_opc_done = phase == 2 ? 1 : 0;

	return result;
}

opcode_result_t lit2(uint8_t phase, uint16_t pc, uint8_t previous_ram_read) {
	// SHIFT( 2) rr = ram + pc; T2_(PEEK2(rr)) pc += 2;	
	static uint4_t offsets[4] = {0, 0, 2, 1};
	static uint1_t stack_writes[4] = {0, 0, 1, 1};
	#if DEBUG
	printf("************\n**** LIT2 ***\n************\n");
	#endif
	
	opcode_result_t result;
	result.is_stack_index_flipped = 0;
	result.is_pc_updated = phase == 2 ? 1 : 0;
	result.is_ram_write = 0;
	result.is_vram_write = 0;
	result.sp_relative_shift = phase == 0 ? 2 : 0;
	result.stack_address_sp_offset = offsets[phase];
	result.is_stack_write = stack_writes[phase];
	result.u8_value = previous_ram_read;
	result.u16_value = phase == 3 ? 0 : pc + phase;
	result.is_opc_done = phase == 3 ? 1 : 0;
		
	return result;
}

opcode_result_t pop(uint8_t phase, uint8_t ins) {
	// SET(1,-1)
	#if DEBUG
	printf("************\n**** POP ***\n************\n");
	#endif
	opcode_result_t result;
	result.is_stack_write = 0;
	result.is_stack_index_flipped = 0;
	result.is_pc_updated = 0;
	result.is_ram_write = 0;
	result.is_vram_write = 0;
	result.sp_relative_shift = sp_relative_shift(ins, 1, -1);
	result.is_opc_done = 1;
	
	return result;
}

opcode_result_t pop2(uint8_t phase, uint8_t ins) {
	// SET(2,-2)
	#if DEBUG
	printf("************\n**** POP2 ***\n************\n");
	#endif
	opcode_result_t result;
	result.is_stack_write = 0;
	result.is_stack_index_flipped = 0;
	result.is_pc_updated = 0;
	result.is_ram_write = 0;
	result.is_vram_write = 0;
	result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
	result.is_opc_done = 1;
	
	return result;
}

opcode_result_t ovr(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2, 1) T = n; N = t; L = n;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** OVR ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, 1);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u8_value = t8;  // set N
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8;  // set T
	}
	else if (phase == 4) {
		result.stack_address_sp_offset = 3;
		result.u8_value = n8;  // set L
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ovr2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4, 2) T2_(n) N2_(t) L2_(n) break;
	static uint8_t t16_low, t16_high, n16_low, n16_high;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** OVR2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16_high = previous_stack_read;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16_low = previous_stack_read;
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16_high = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 4, 2);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 3;
		result.u8_value = t16_low; // set N2 = previous T2 (low byte)
	}
	else if (phase == 5) {
		n16_low = previous_stack_read;
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 4;
		result.u8_value = t16_high; // set N2 = previous T2 (high byte)
	}
	else if (phase == 6) {
		result.stack_address_sp_offset = 1;
		result.u8_value = n16_low; // set T2 = previous N2 (low byte)
	}
	else if (phase == 7) {
		result.stack_address_sp_offset = 2;
		result.u8_value = n16_high; // set T2 = previous N2 (high byte)
	}
	else if (phase == 8) {
		result.stack_address_sp_offset = 5;
		result.u8_value = n16_low; // set L2 = previous N2 (low byte)
	}
	else if (phase == 9) {
		result.stack_address_sp_offset = 6;
		result.u8_value = n16_high; // set L2 = previous N2 (high byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t dei(uint8_t phase, uint8_t ins, uint8_t controller0_buttons, uint8_t stack_ptr0, uint8_t stack_ptr1, uint8_t previous_stack_read, uint8_t previous_device_ram_read) {
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
		result.sp_relative_shift = 0;
		result.is_stack_write = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
		result.is_device_ram_write = 0;
		device_in_result.is_dei_done = 0;
	}
	else if (phase == 1) {
		result.sp_relative_shift = sp_relative_shift(ins, 1, 0);
	}
	else {
		result.sp_relative_shift = 0;
		t8 = (phase == 2) ? previous_stack_read : t8;
		if (~device_in_result.is_dei_done) {
			device_in_result = device_in(t8, phase - 2, controller0_buttons, stack_ptr0, stack_ptr1, previous_device_ram_read);
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

opcode_result_t dei2(uint8_t phase, uint8_t ins, uint8_t controller0_buttons, uint8_t stack_ptr0, uint8_t stack_ptr1, uint8_t previous_stack_read, uint8_t previous_device_ram_read) {
	// t=T;            SET(1, 1) T = DEI(t + 1); N = DEI(t);
	static uint8_t t8, current_dei_phase, dei_param;
	static uint1_t is_first_dei_done;
	static device_in_result_t device_in_result;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** DEI2 ***\n************\n");
		#endif
		is_first_dei_done = 0;
		current_dei_phase = 0;
		result.sp_relative_shift = 0;
		result.is_stack_write = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_device_ram_write = 0;
		result.is_opc_done = 0;
		device_in_result.is_dei_done = 0;
	}
	else if (phase == 1) {
		result.sp_relative_shift = sp_relative_shift(ins, 1, 1);
	}
	else {
		result.sp_relative_shift = 0;
		t8 = (phase == 2) ? previous_stack_read : t8;
		dei_param = is_first_dei_done ? t8 : t8 + 1;
		device_in_result = device_in(dei_param, current_dei_phase, controller0_buttons, stack_ptr0, stack_ptr1, previous_device_ram_read);
		if (device_in_result.is_dei_done) {
			current_dei_phase = 0;
			result.is_stack_write = 1;
			result.is_opc_done = is_first_dei_done;
			result.stack_address_sp_offset = is_first_dei_done ? 2 : 1; // set T and then N
			result.u8_value = device_in_result.dei_value;
			is_first_dei_done = 1;
		} else {
			result.device_ram_address = device_in_result.device_ram_address;
			result.is_stack_write = 0;
			current_dei_phase += 1;
		}
	}
	
	return result;
}

opcode_result_t deo(uint12_t phase, uint8_t ins, uint8_t previous_stack_read, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	// t=T;n=N;        SET(2,-2) DEO(t, n)
	static uint8_t t8, n8;
	static opcode_result_t result;
	static device_out_result_t device_out_result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** DEO ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
	}
	else {
		result.sp_relative_shift = 0;
		n8 = (phase == 3) ? previous_stack_read : n8;
		device_out_result = device_out(t8, n8, phase - 3, previous_device_ram_read, previous_ram_read);
		result.is_device_ram_write = device_out_result.is_device_ram_write;
		result.device_ram_address = device_out_result.device_ram_address;
		result.u8_value = device_out_result.u8_value;
		result.u16_value = device_out_result.u16_addr;
		result.is_vram_write = device_out_result.is_vram_write;
		result.vram_write_layer = device_out_result.vram_write_layer;
		result.is_opc_done = device_out_result.is_deo_done;
	}
	
	return result;
}

opcode_result_t deo2(uint12_t phase, uint8_t ins, uint8_t previous_stack_read, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	// t=T;n=N;l=L;    SET(3,-3) DEO(t, l) DEO((t + 1), n)
	static uint8_t t8, n8, l8, deo_param0, deo_param1;
	static uint12_t current_deo_phase;
	static uint1_t is_second_deo = 0, is_phase_3 = 0, is_phase_4 = 0;
	static opcode_result_t result;
	static device_out_result_t device_out_result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** DEO2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
		is_second_deo = 0;
		current_deo_phase = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 3; // get L
	}
	else if (phase == 2) {
		result.stack_address_sp_offset = 2; // get N
		t8 = previous_stack_read;
	}
	else {
		is_phase_3 = (phase == 3) ? 1 : 0;
		is_phase_4 = (phase == 4) ? 1 : 0;
		l8 = is_phase_3 ? previous_stack_read : l8;
		n8 = is_phase_4 ? previous_stack_read : n8;
		deo_param0 = is_second_deo ? t8 + 1 : t8;
		deo_param1 = is_second_deo ? n8 : l8;
		result.sp_relative_shift = is_phase_3 ? sp_relative_shift(ins, 3, -3) : 0;
		device_out_result = device_out(deo_param0, deo_param1, current_deo_phase, previous_device_ram_read, previous_ram_read);
		result.is_device_ram_write = device_out_result.is_device_ram_write;
		result.device_ram_address = device_out_result.device_ram_address;
		result.is_vram_write = device_out_result.is_vram_write;
		result.vram_write_layer = device_out_result.vram_write_layer;
		result.u16_value = device_out_result.u16_addr;
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
	static uint8_t t8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** JMP ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 1, -1);
		result.is_pc_updated = 1;
		result.u16_value = u16_add_u8_as_i8(pc, t8); 
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t jmp2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;           SET(2,-2) pc = t
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** JMP2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
		result.is_pc_updated = 1;
		result.u16_value = t16; // pc = t16
		result.is_opc_done = 1;
	}

	return result;
}

opcode_result_t jcn(uint8_t phase, uint8_t ins, uint16_t pc, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-2) if(n) pc += (Sint8)t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** JCN ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
		result.is_pc_updated = 1;
		result.u16_value = n8 == 0 ? pc : u16_add_u8_as_i8(pc, t8); // if(n) pc += (Sint8)t;
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
		#if DEBUG
		printf("************\n*** JCN2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 3; // get L
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 3, -3);
		result.is_pc_updated = n8 == 0 ? 0 : 1;
		result.u16_value = n8 == 0 ? 0 : t16; // if(n) pc = t;
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
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
		result.sp_relative_shift = sp_relative_shift(ins, 1, -1);
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_index_flipped = 1;
		result.sp_relative_shift = 2;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = pc(7, 0); // set T2 (low byte)
	}
	else if (phase == 3) {
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u8_value = pc(15, 8); // set T2 (high byte)
		result.is_pc_updated = 1;
		result.u16_value = u16_add_u8_as_i8(pc, t8); // pc += t
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t jsr2(uint8_t phase, uint8_t ins, uint16_t pc, uint8_t previous_stack_read) {
	// t=T2;           SET(2,-2) FLIP SHIFT(2) T2_(pc) pc = t;
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** JSR2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_index_flipped = 1;
		result.sp_relative_shift = 2;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = pc(7, 0);		// set T2 (low byte)
	}
	else if (phase == 4) {
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u8_value = pc(15, 8); 	// set T2 (high byte)
		result.is_pc_updated = 1;
		result.u16_value = t16; // pc = t16
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t add(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n + t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** ADD ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 + t8;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t add2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-2) T2_(n + t) 
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** ADD2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
	}
	else if (phase == 5) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = t16 + n16;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 6) {
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u8_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t and(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T;n=N;        SET(2,-1) T = n & t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** AND ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 & t8;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t and2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-2) T2_(n & t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** AND2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
	}
	else if (phase == 5) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = t16 & n16;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 6) {
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u8_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ora(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T;n=N;        SET(2,-1) T = n | t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** ORA ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 | t8;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ora2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-2) T2_(n | t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** ORA2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
	}
	else if (phase == 5) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = n16 | t16;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 6) {
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u8_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t eor(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n ^ t; break;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** EOR ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 ^ t8;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t eor2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-2) T2_(n ^ t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** EOR2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
	}
	else if (phase == 5) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = n16 ^ t16;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 6) {
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u8_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t equ(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T;n=N;        SET(2,-1) T = n == t; 
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** EQU ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 == t8 ? 1 : 0;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t equ2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-3) T = n == t;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** EQU2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
	}
	else if (phase == 5) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.sp_relative_shift = sp_relative_shift(ins, 4, -3);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n16 == t16 ? 1 : 0;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t neq(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T;n=N;        SET(2,-1) T = n != t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** NEQ ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 == t8 ? 0 : 1;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t neq2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;n=N2;      SET(4,-3) T = n != t;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** NEQ2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -3);
	}
	else if (phase == 5) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.sp_relative_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n16 == t16 ? 0 : 1;	// set T
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
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 1, 0);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = t8 + 1;	// set T 
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t inc2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	//  t=T2;           SET(2, 0) T2_(t + 1)
	static uint8_t t16_high, t16_low;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** INC2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16_low = previous_stack_read + 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, 0);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = t16_low;	// set T2 (low byte)
	}
	else if (phase == 3) {
		t16_high = previous_stack_read + (t16_low == 0 ? 1 : 0);
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u8_value = t16_high; // set T2 (high byte)
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
		#if DEBUG
		printf("************\n**** LDA ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
		result.u16_value = t16; // peek RAM at address equal to T2
	}
	else if (phase == 4) {
		result.sp_relative_shift = 0;
	}
	else if (phase == 5) {
		tmp8 = previous_ram_read;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp8;	// set T
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
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 1, 0);
		result.u16_value = (uint16_t)(t8); // peek RAM at address equal to T
	}
	else if (phase == 3) {
		result.sp_relative_shift = 0;
	}
	else if (phase == 4) {
		tmp8 = previous_ram_read;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp8;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ldz2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	//  t=T;            SET(1, 1) rr = ram + t; T2_(PEEK2(rr))
	static uint8_t t8, tmp8_high, tmp8_low;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** LDZ2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 1, 1);
		result.u16_value = (uint16_t)(t8);     // peek RAM at address equal to T
	}
	else if (phase == 3) {
		result.sp_relative_shift = 0;
		result.u16_value = (uint16_t)(t8 + 1); // peek RAM at address equal to T + 1
	}
	else if (phase == 4) {
		tmp8_high = previous_ram_read;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u8_value = tmp8_high; // set T2 (high byte)
	}
	else if (phase == 5) {
		tmp8_low = previous_ram_read;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp8_low;	// set T2 (low byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t stz(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-2) ram[t] = n;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** STZ ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
		result.is_ram_write = 1;
		result.u16_value = (uint16_t)(t8);
		result.u8_value = n8; // set first byte of n16 to ram address t8 
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t stz2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=H2;       SET(3,-3) rr = ram + t; POKE2(rr, n)
	static uint8_t t8, n16_low, n16_high;
	static opcode_result_t result;
	
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** STZ2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 3; // get H2 (byte 1 of 2)
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get H2 (byte 2 of 2)
	}
	else if (phase == 3) {
		n16_high = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 3, -3);
		result.is_ram_write = 1;
		result.u16_value = (uint16_t)(t8);
		result.u8_value = n16_high; // set first byte of n16 to ram address t8 
	}
	else if (phase == 4) {
		n16_low = previous_stack_read;
		result.sp_relative_shift = 0;
		result.u16_value = (uint16_t)(t8 + 1);
		result.u8_value = n16_low; // set second byte of n16 to ram address t8 + 1 
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
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 1, 0);
		result.u16_value = u16_add_u8_as_i8(pc, t8); // peek RAM at address equal to  PC + T 
	}
	else if (phase == 3) {
		result.sp_relative_shift = 0;
	}
	else if (phase == 4) {
		tmp8 = previous_ram_read;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp8;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ldr2(uint8_t phase, uint8_t ins, uint16_t pc, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// t=T;            SET(1, 1) rr = ram + pc + (Sint8)t; T2_(PEEK2(rr))
	static uint8_t t8, tmp8_high, tmp8_low;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** LDR2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.sp_relative_shift = sp_relative_shift(ins, 1, 1);
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.sp_relative_shift = 0;
		result.u16_value = u16_add_u8_as_i8(pc, t8);     // peek RAM (byte 1 of 2) at address equal to PC + T 
	}
	else if (phase == 3) {
		result.u16_value += 1; // peek RAM (byte 2 of 2) at address equal to PC + T + 1
	}
	else if (phase == 4) {
		tmp8_high = previous_ram_read;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u8_value = tmp8_high; // set T2 (high byte)
	}
	else if (phase == 5) {
		tmp8_low = previous_ram_read;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp8_low;	// set T2 (low byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t str1(uint8_t phase, uint8_t ins, uint16_t pc, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-2) ram[pc + (Sint8)t] = n;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** STR ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
		result.is_ram_write = 1;
		result.u16_value = u16_add_u8_as_i8(pc, t8);
		result.u8_value = n8; // set first n8 to ram address t8 
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t str2(uint8_t phase, uint8_t ins, uint16_t pc, uint8_t previous_stack_read) {
	// t=T;n=H2;       SET(3,-3) rr = ram + pc + (Sint8)t; POKE2(rr, n)
	static uint8_t t8, n16_high, n16_low;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** STR2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 3; // get H2 (byte 1 of 2)
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2; // get H2 (byte 2 of 2)
	}
	else if (phase == 3) {
		n16_high = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 3, -3);
		result.is_ram_write = 1;
		result.u16_value = u16_add_u8_as_i8(pc, t8);
		result.u8_value = n16_high; // set first byte of n16 to ram address pc + t8 
	}
	else if (phase == 4) {
		n16_low = previous_stack_read;
		result.sp_relative_shift = 0;
		result.u16_value += 1;
		result.u8_value = n16_low; // set second byte of n16 to ram address pc + t8 + 1 
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lda2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// t=T2;           SET(2, 0) rr = ram + t; T2_(PEEK2(rr))
	static uint8_t tmp8_high, tmp8_low;
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** LDA2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.sp_relative_shift = sp_relative_shift(ins, 2, 0);
		result.u16_value = t16;     // peek RAM (byte 1 of 2) at address equal to t16
	}
	else if (phase == 4) {
		result.sp_relative_shift = 0;
		result.u16_value = t16 + 1; // peek RAM (byte 2 of 2) at address equal to t16 + 1
	}
	else if (phase == 5) {
		tmp8_high = previous_ram_read;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u8_value = tmp8_high; // set T2 (high byte)
	}
	else if (phase == 6) {
		tmp8_low = previous_ram_read;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp8_low;	// set T2 (low byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t gth(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n > t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** GTH ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 > t8 ? 1 : 0;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t gth2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-3) T = n > t;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** GTH2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
	}
	else if (phase == 5) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.sp_relative_shift = sp_relative_shift(ins, 4, -3);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n16 > t16 ? 1 : 0;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lth(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n < t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** LTH ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 < t8 ? 1 : 0;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lth2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-3) T = n < t;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** LTH2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
	}
	else if (phase == 5) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.sp_relative_shift = sp_relative_shift(ins, 4, -3);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n16 < t16 ? 1 : 0;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t mul(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n * t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** MUL ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 * t8;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t mul2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-2) T2_(n * t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** MUL2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
	}
	else if (phase == 5) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = t16 * n16;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp16(7, 0); // set T2 (low byte)
	}
	else if (phase == 6) {
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u8_value = tmp16(15, 8); // set T2 (high byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t div(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = t ? n / t : 0;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** DIV ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = t8 == 0 ? 0 : n8 / t8;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t div2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-2) T2_(t ? n / t : 0)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** DIV2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
	}
	else if (phase == 5) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = t16 == 0 ? 0 : n16 / t16;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp16(7, 0);// set T2 (low byte)
	}
	else if (phase == 6) {
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u8_value = tmp16(15, 8); // set T2 (high byte)
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
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.sp_relative_shift = 0;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = t8;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t nip2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;           SET(4,-2) T2_(t)
	static uint8_t t16_high, t16_low;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** NIP2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16_high = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u8_value = t16_high; // set T2 (high byte)
	}
	else if (phase == 3) {
		t16_low = previous_stack_read;
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 1;
		result.u8_value = t16_low;	// set T2 (low byte)
		result.is_opc_done = 1;
	}
		
	return result;
}


opcode_result_t sft(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n >> (t & 0xf) << (t >> 4);
	static uint8_t t8, n8, tmp8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** SFT ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = 0;
		tmp8 = (n8 >> (t8 & 0x0F)) << (t8 >> 4);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp8;	// set T
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
		#if DEBUG
		printf("************\n*** SFT2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 3;  // get H2 (byte 1 of 2)
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.stack_address_sp_offset = 2;  // get H2 (byte 2 of 2)
	}
	else if (phase == 3) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
	} 
	else if (phase == 4) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.sp_relative_shift = sp_relative_shift(ins, 3, -1);
		tmp16 = (n16 >> (t8 & 0x0F)) << (t8 >> 4);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = tmp16(7, 0);	// set T2 (low byte)
	}
	else if (phase == 5) {
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u8_value = tmp16(15, 8); // set T2 (high byte)
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
		#if DEBUG
		printf("************\n**** STA ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 3;  // get L
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 3, -3);
		result.is_ram_write = 1;
		result.u16_value = t16; // poke RAM at address equal to T2
		result.u8_value = n8;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sta2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-4) rr = ram + t; POKE2(rr, n)
	static uint8_t n16_high, n16_low;
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** STA2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16_high = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -4);
		result.is_ram_write = 1;
		result.u16_value = t16;
		result.u8_value = n16_high; // set first byte of n16 to ram address t16 
	}
	else if (phase == 5) {
		n16_low = previous_stack_read;
		result.sp_relative_shift = 0;
		result.u16_value = t16 + 1;
		result.u8_value = n16_low; // set second byte of n16 to ram address t16 + 1 
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
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.sp_relative_shift = sp_relative_shift(ins, 1, -1);
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_index_flipped = 1;
		result.sp_relative_shift = 1;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = t8; // set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sth2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;           SET(2,-2) FLIP SHIFT(2) T2_(t)
	static uint8_t t16_high, t16_low;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** STH2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16_high = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -2);
	}
	else if (phase == 3) {
		t16_low = previous_stack_read;
		result.is_stack_index_flipped = 1;
		result.sp_relative_shift = 2;
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u8_value = t16_high; // set T2 (high byte)
	}
	else if (phase == 4) {
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 1;
		result.u8_value = t16_low;	// set T2 (low byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sub1(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2,-1) T = n - t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** SUB ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, -1);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8 - t8;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t sub2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4,-2) T2_(n - t)
	static uint16_t t16, n16, tmp16;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** SUB2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
	}
	else if (phase == 5) {
		n16 |= ((uint16_t)(previous_stack_read));
		tmp16 = n16 - t16;
		result.sp_relative_shift = sp_relative_shift(ins, 4, -2);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 1;
		result.u8_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 6) {
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 2;
		result.u8_value = (uint8_t)(tmp16 >> 8);  // set T2 (high byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t swp(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;        SET(2, 0) T = n; N = t;
	static uint8_t t8, n8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** SWP ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, 0);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u8_value = t8;	// set N
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 1;
		result.u8_value = n8;	// set T
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t swp2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;      SET(4, 0) T2_(n) N2_(t)
	static uint8_t t16_high, t16_low, n16_high, n16_low;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** SWP2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16_high = previous_stack_read;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16_low = previous_stack_read;
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16_high = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 4, 0);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 4;
		result.u8_value = t16_high; // set N2 (high byte)
	}
	else if (phase == 5) {
		n16_low = previous_stack_read;
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 3;
		result.u8_value = t16_low;	// set N2 (low byte)
	}
	else if (phase == 6) {
		result.stack_address_sp_offset = 2;
		result.u8_value = n16_high; // set T2 (high byte)
	}
	else if (phase == 7) {
		result.stack_address_sp_offset = 1;
		result.u8_value = n16_low;	// set T2 (low byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t rot(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;n=N;l=L;    SET(3, 0) T = l; N = t; L = n;
	static uint8_t t8, n8, l8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** ROT ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 2; // get N
	}
	else if (phase == 2) {
		result.stack_address_sp_offset = 3; // get L
		t8 = previous_stack_read;
	}
	else if (phase == 3) {
		n8 = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 3, 0);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 3;
		result.u8_value = n8;	// set L
	}
	else if (phase == 4) {
		l8 = previous_stack_read;
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 1;
		result.u8_value = l8;	// set T
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 2;
		result.u8_value = t8;	// set N
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t rot2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;n=N2;l=L2; SET(6, 0) T2_(l) N2_(t) L2_(n)
	static uint8_t t16_high, t16_low, n16_high, n16_low, l16_high, l16_low;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** ROT2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16_high = previous_stack_read;
		result.stack_address_sp_offset = 4; // get N2 (byte 1 of 2)
	}
	else if (phase == 3) {
		t16_low = previous_stack_read;
		result.stack_address_sp_offset = 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 4) {
		n16_high = previous_stack_read;
		result.stack_address_sp_offset = 6; // get L2 (byte 1 of 2)
	}
	else if (phase == 5) {
		n16_low = previous_stack_read;
		result.stack_address_sp_offset = 5;  // get L2 (byte 2 of 2)
	}
	else if (phase == 6) {
		l16_high = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 6, 0);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 4;
		result.u8_value = t16_high; // set N2 (high byte)
	}
	else if (phase == 7) {
		l16_low = previous_stack_read;
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 3;
		result.u8_value = t16_low;	// set N2 (low byte)
	}
	else if (phase == 8) {
		result.stack_address_sp_offset = 6;
		result.u8_value = n16_high; // set L2 (high byte)
	}
	else if (phase == 9) {
		result.stack_address_sp_offset = 5;
		result.u8_value = n16_low;	// set L2 (low byte)
	}
	else if (phase == 10) {
		result.stack_address_sp_offset = 2;
		result.u8_value = l16_high; // set T2 (high byte)
	}
	else if (phase == 11) {
		result.stack_address_sp_offset = 1;
		result.u8_value = l16_low;	// set T2 (low byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t dup(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T;            SET(1, 1) T = t; N = t;
	static uint8_t t8;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n**** DUP ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.sp_relative_shift = sp_relative_shift(ins, 1, 1);
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.sp_relative_shift = 0;
		result.is_stack_write = 1; 
		result.u8_value = t8;	// set T
	}
	else if (phase == 3) {
		result.stack_address_sp_offset = 2;
		result.u8_value = t8;	// set N
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t dup2(uint8_t phase, uint8_t ins, uint8_t previous_stack_read) {
	// t=T2;           SET(2, 2) T2_(t) N2_(t) break;
	static uint8_t t16_high, t16_low;
	static opcode_result_t result;
	if (phase == 0) {
		#if DEBUG
		printf("************\n*** DUP2 ***\n************\n");
		#endif
		result.is_stack_write = 0;
		result.sp_relative_shift = 0;
		result.is_stack_index_flipped = 0;
		result.is_pc_updated = 0;
		result.is_ram_write = 0;
		result.is_vram_write = 0;
		result.stack_address_sp_offset = 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.stack_address_sp_offset = 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 2) {
		t16_high = previous_stack_read;
		result.sp_relative_shift = sp_relative_shift(ins, 2, 2);
		result.is_stack_write = 1;
		result.stack_address_sp_offset = 2;
		result.u8_value = t16_high; // set T2 (high byte)
	}
	else if (phase == 3) {
		t16_low = previous_stack_read;
		result.sp_relative_shift = 0;
		result.stack_address_sp_offset = 1;
		result.u8_value = t16_low;	// set T2 (low byte)
	}
	else if (phase == 4) {
		result.stack_address_sp_offset = 4;
		result.u8_value = t16_high; // set N2 (high byte)
	}
	else if (phase == 5) {
		result.stack_address_sp_offset = 3;
		result.u8_value = t16_low;	// set N2 (low byte)
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t brk(uint8_t phase) {
	opcode_result_t result = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
	#if DEBUG
	printf("************\n**** BRK ***\n************\n");
	#endif
	result.is_stack_write = 0;
	result.sp_relative_shift = 0;
	result.is_stack_index_flipped = 0;
	result.is_pc_updated = 0;
	result.is_ram_write = 0;
	result.is_vram_write = 0;
	result.is_opc_done = 1;
	
	return result;
}

eval_opcode_result_t eval_opcode_phased(
	uint12_t phase,
	uint8_t ins,
	uint16_t pc,
	uint8_t controller0_buttons,
	uint8_t previous_ram_read,
	uint8_t previous_device_ram_read
) {
	static uint8_t sp0, sp1, ins_a, opc;
	static uint1_t stack_index, is_wait;
	static uint9_t stack_address = 0;
	static uint8_t previous_stack_read = 0;
	static opcode_result_t opc_result = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static eval_opcode_result_t opc_eval_result = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	
	is_wait = 0;
	ins_a = ins(4, 0) == 0 ? 0xFF : 0x3F;
	opc = ins & ins_a;
	
	#if DEBUG
	printf("        EVAL OPCODE: INS = 0x%X, OPC = 0x%X, phase = 0x%X\n", ins, opc, phase);
	#endif
	
	if      (opc == 0x00 /* BRK   */) { is_wait = 1; opc_result = brk(phase); }
	else if (opc == 0x20 /* JCI   */) { opc_result = jci(phase, pc, previous_stack_read, previous_ram_read); }
	else if (opc == 0x40 /* JMI   */) { opc_result = jmi(phase, pc, previous_ram_read); }
	else if (opc == 0x60 /* JSI   */) { opc_result = jsi(phase, pc, previous_ram_read); }
	else if (opc == 0x80 /* LIT   */) { opc_result = lit(phase, pc, previous_ram_read); }
	else if (opc == 0xA0 /* LIT2  */) { opc_result = lit2(phase, pc, previous_ram_read); }
	else if (opc == 0xC0 /* LITr  */) { opc_result = lit(phase, pc, previous_ram_read); }
	else if (opc == 0xE0 /* LIT2r */) { opc_result = lit2(phase, pc, previous_ram_read); }
	else if (opc == 0x01 /* INC   */) { opc_result = inc(phase, ins, previous_stack_read); }
	else if (opc == 0x21 /* INC2  */) { opc_result = inc2(phase, ins, previous_stack_read); }
	else if (opc == 0x02 /* POP   */) { opc_result = pop(phase, ins); }
	else if (opc == 0x22 /* POP2  */) { opc_result = pop2(phase, ins); }
	else if (opc == 0x03 /* NIP   */) { opc_result = nip(phase, ins, previous_stack_read); }
	else if (opc == 0x23 /* NIP2  */) { opc_result = nip2(phase, ins, previous_stack_read); }
	else if (opc == 0x04 /* SWP   */) { opc_result = swp(phase, ins, previous_stack_read); }
	else if (opc == 0x24 /* SWP2  */) { opc_result = swp2(phase, ins, previous_stack_read); }
	else if (opc == 0x05 /* ROT   */) { opc_result = rot(phase, ins, previous_stack_read); }
	else if (opc == 0x25 /* ROT2  */) { opc_result = rot2(phase, ins, previous_stack_read); }
	else if (opc == 0x06 /* DUP   */) { opc_result = dup(phase, ins, previous_stack_read); }
	else if (opc == 0x26 /* DUP2  */) { opc_result = dup2(phase, ins, previous_stack_read); }
	else if (opc == 0x07 /* OVR   */) { opc_result = ovr(phase, ins, previous_stack_read); }
	else if (opc == 0x27 /* OVR2  */) { opc_result = ovr2(phase, ins, previous_stack_read); }
	else if (opc == 0x08 /* EQU   */) { opc_result = equ(phase, ins, previous_stack_read); }
	else if (opc == 0x28 /* EQU2  */) { opc_result = equ2(phase, ins, previous_stack_read); }
	else if (opc == 0x09 /* NEQ   */) { opc_result = neq(phase, ins, previous_stack_read); }
	else if (opc == 0x29 /* NEQ2  */) { opc_result = neq2(phase, ins, previous_stack_read); }
	else if (opc == 0x0A /* GTH   */) { opc_result = gth(phase, ins, previous_stack_read); }
	else if (opc == 0x2A /* GTH2  */) { opc_result = gth2(phase, ins, previous_stack_read); }
	else if (opc == 0x0B /* LTH   */) { opc_result = lth(phase, ins, previous_stack_read);  }
	else if (opc == 0x2B /* LTH2  */) { opc_result = lth2(phase, ins, previous_stack_read); }
	else if (opc == 0x0C /* JMP   */) { opc_result = jmp(phase, ins, pc, previous_stack_read); }
	else if (opc == 0x2C /* JMP2  */) { opc_result = jmp2(phase, ins, previous_stack_read); }
	else if (opc == 0x0D /* JCN   */) { opc_result = jcn(phase, ins, pc, previous_stack_read); }
	else if (opc == 0x2D /* JCN2  */) { opc_result = jcn2(phase, ins, previous_stack_read); }
	else if (opc == 0x0E /* JSR   */) { opc_result = jsr(phase, ins, pc, previous_stack_read); }
	else if (opc == 0x2E /* JSR2  */) { opc_result = jsr2(phase, ins, pc, previous_stack_read); }
	else if (opc == 0x0F /* STH   */) { opc_result = sth(phase, ins, previous_stack_read); }
	else if (opc == 0x2F /* STH2  */) { opc_result = sth2(phase, ins, previous_stack_read); }
	else if (opc == 0x10 /* LDZ   */) { opc_result = ldz(phase, ins, previous_stack_read, previous_ram_read); }
	else if (opc == 0x30 /* LDZ2  */) { opc_result = ldz2(phase, ins, previous_stack_read, previous_ram_read); }
	else if (opc == 0x11 /* STZ   */) { opc_result = stz(phase, ins, previous_stack_read); }
	else if (opc == 0x31 /* STZ2  */) { opc_result = stz2(phase, ins, previous_stack_read); }
	else if (opc == 0x12 /* LDR   */) { opc_result = ldr(phase, ins, pc, previous_stack_read, previous_ram_read); }
	else if (opc == 0x32 /* LDR2  */) { opc_result = ldr2(phase, ins, pc, previous_stack_read, previous_ram_read); }
	else if (opc == 0x13 /* STR   */) { opc_result = str1(phase, ins, pc, previous_stack_read); }
	else if (opc == 0x33 /* STR2  */) { opc_result = str2(phase, ins, pc, previous_stack_read); }
	else if (opc == 0x14 /* LDA   */) { opc_result = lda(phase, ins, previous_stack_read, previous_ram_read); }
	else if (opc == 0x34 /* LDA2  */) { opc_result = lda2(phase, ins, previous_stack_read, previous_ram_read); }
	else if (opc == 0x15 /* STA   */) { opc_result = sta(phase, ins, previous_stack_read); }
	else if (opc == 0x35 /* STA2  */) { opc_result = sta2(phase, ins, previous_stack_read); }
	else if (opc == 0x16 /* DEI   */) { opc_result = dei(phase, ins, controller0_buttons, sp0, sp1, previous_stack_read, previous_device_ram_read); }
	else if (opc == 0x36 /* DEI2  */) { opc_result = dei2(phase, ins, controller0_buttons, sp0, sp1, previous_stack_read, previous_device_ram_read); }
	else if (opc == 0x17 /* DEO   */) { opc_result = deo(phase, ins, previous_stack_read, previous_device_ram_read, previous_ram_read); }
	else if (opc == 0x37 /* DEO2  */) { opc_result = deo2(phase, ins, previous_stack_read, previous_device_ram_read, previous_ram_read); }
	else if (opc == 0x18 /* ADD   */) { opc_result = add(phase, ins, previous_stack_read); }
	else if (opc == 0x38 /* ADD2  */) { opc_result = add2(phase, ins, previous_stack_read); }
	else if (opc == 0x19 /* SUB   */) { opc_result = sub1(phase, ins, previous_stack_read); }
	else if (opc == 0x39 /* SUB2  */) { opc_result = sub2(phase, ins, previous_stack_read); }
	else if (opc == 0x1A /* MUL   */) { opc_result = mul(phase, ins, previous_stack_read); }
	else if (opc == 0x3A /* MUL2  */) { opc_result = mul2(phase, ins, previous_stack_read); }
	else if (opc == 0x1B /* DIV   */) { opc_result = div(phase, ins, previous_stack_read); }
	else if (opc == 0x3B /* DIV2  */) { opc_result = div2(phase, ins, previous_stack_read); }
	else if (opc == 0x1C /* AND   */) { opc_result = and(phase, ins, previous_stack_read); }
	else if (opc == 0x3C /* AND2  */) { opc_result = and2(phase, ins, previous_stack_read); }
	else if (opc == 0x1D /* ORA   */) { opc_result = ora(phase, ins, previous_stack_read); }
	else if (opc == 0x3D /* ORA2  */) { opc_result = ora2(phase, ins, previous_stack_read); }
	else if (opc == 0x1E /* EOR   */) { opc_result = eor(phase, ins, previous_stack_read); }
	else if (opc == 0x3E /* EOR2  */) { opc_result = eor2(phase, ins, previous_stack_read); }
	else if (opc == 0x1F /* SFT   */) { opc_result = sft(phase, ins, previous_stack_read); }
	else if (opc == 0x3F /* SFT2  */) { opc_result = sft2(phase, ins, previous_stack_read); }
	
	stack_index = ins >> 6;
	stack_index ^= opc_result.is_stack_index_flipped;
	
	if (stack_index) {
		sp1 += ((int8_t)(opc_result.sp_relative_shift));
	} else {
		sp0 += ((int8_t)(opc_result.sp_relative_shift));
	}
			
	stack_address = ((uint9_t)(stack_index ? sp1 : sp0)) - ((uint9_t)(opc_result.stack_address_sp_offset));
	stack_address |= (stack_index ? 0b100000000 : 0);
	
	previous_stack_read = stack_ram_update(
		stack_address, 
		opc_result.u8_value,
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
	opc_eval_result.u8_value = opc_result.u8_value;
	opc_eval_result.is_opc_done = opc_result.is_opc_done;
	
	return opc_eval_result;
}
