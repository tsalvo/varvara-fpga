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
	
	uint1_t is_sp_updated;
	uint8_t sp; // updated stack pointer value
	
	uint1_t is_stack_read;
	uint1_t is_stack_write;
	uint8_t stack_address;
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

opcode_result_t jci(uint8_t phase, uint1_t stack_index, uint16_t pc, uint8_t sp, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	static uint8_t t8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("            JCI\n");
		result.is_stack_write = 0;
		result.is_stack_read = 1;
		result.stack_address = sp - 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_write = 0;
		result.is_stack_read = 1;
		result.stack_address = sp - 1;
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_write = 0;
		result.is_stack_read = 0;
		result.stack_address = 0;
	}
	else if (phase == 3) {
		result.is_sp_updated = 1;
		result.sp = sp - 1;
	}
	else if (phase == 4) {
		result.is_pc_updated = t8 == 0 ? 1 : 0;
		result.pc = t8 == 0 ? pc + 2 : pc;
	}
	else {
		result.is_sp_updated = 0;
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t jsi(uint8_t phase, uint1_t stack_index, uint16_t pc, uint8_t sp, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// SHIFT( 2) T2_(pc + 2); rr = ram + pc; pc += PEEK2(rr) + 2;
	static uint16_t tmp16 = 0;
	static opcode_result_t result;
	if (phase == 0) {
		printf("            JSI\n");
		result.is_sp_updated = 1;
		result.sp = sp + 2; 		// shift(2)
		tmp16 = pc + 2;
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_updated = 0;
		result.is_stack_write = 1;
		result.is_stack_read = 0;
		result.stack_address = sp - 1;
		result.stack_value = (uint8_t)(tmp16);	// set T2 (low byte)
	}
	else if (phase == 2) {
		result.is_stack_write = 1;
		result.stack_address = sp - 2;
		result.stack_value = (uint8_t)(tmp16 >> 8); // set T2 (high byte)
	}
	else if (phase == 3) {
		result.is_ram_write = 0;
		result.is_ram_read = 1;
		result.ram_addr = pc; // peek RAM (byte 1 of 2) at address equal to PC
	}
	else if (phase == 4) {
		result.is_ram_write = 0;
		result.is_ram_read = 1;
		result.ram_addr = pc;
	}
	else if (phase == 5) {
		tmp16 = (uint16_t)(previous_ram_read);
		tmp16 <= 8;
		result.is_ram_write = 0;
		result.is_ram_read = 1;
		result.ram_addr = pc + 1; // peek RAM (byte 2 of 2) at address equal to PC + 1
	}
	else if (phase == 6) {
		result.is_ram_write = 0;
		result.is_ram_read = 1;
		result.ram_addr = pc + 1;
	}
	else if (phase == 7) {
		tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_ram_read = 0;
		result.ram_addr = 0;
		result.is_pc_updated = 1;
		result.pc = pc + tmp16 + 2; // pc += PEEK2_RAM(pc) + 2
	}
	else {
		result.is_pc_updated = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t lit(uint8_t phase, uint1_t stack_index, uint16_t pc, uint8_t sp, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// SHIFT( 1) T = ram[pc++];
	static uint8_t lit_tmp8 = 0;
	static opcode_result_t result;
	
	if (phase == 0) {
		result.is_sp_updated = 1;
		result.sp = sp + 1; 		// shift(1)
		result.is_opc_done = 0;
	} 
	else if (phase == 1) {
		result.is_sp_updated = 0;
		result.is_ram_write = 0;
		result.is_ram_read = 1;
		result.ram_addr = pc; // peek RAM at at address equal to PC
	}
	else if (phase == 2) {
		result.is_ram_write = 0;
		result.is_ram_read = 1;
		result.ram_addr = pc; // peek RAM at at address equal to PC
	}
	else if (phase == 3) {
		result.is_pc_updated = 1;
		result.pc = pc + 1; // pc += 1
		lit_tmp8 = previous_ram_read;
	}
	else if (phase == 4) {
		result.is_stack_write = 1;
		result.is_stack_read = 0;
		result.stack_address = sp - 1;
		result.stack_value = lit_tmp8; // T = PEEK_RAM(pc);
	}
	else if (phase == 5) {
		result.is_stack_write = 0;
		result.is_ram_read = 0;
		result.ram_addr = 0;
		result.is_pc_updated = 0;
		result.stack_value = 0;
		result.pc = 0; 
		result.is_opc_done = 1;
	}
	
	printf("            LIT  phase 0x%X, sp = 0x%X, pc = 0x%X, previous_ram_read = 0x%X, tmp8 = 0x%X\n", phase, sp, pc, previous_ram_read, lit_tmp8);
	
	return result;
}

opcode_result_t lit2(uint8_t phase, uint1_t stack_index, uint16_t pc, uint8_t sp, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// SHIFT( 2) rr = ram + pc; T2_(PEEK2(rr)) pc += 2;
	static uint16_t lit2_tmp16 = 0;
	static opcode_result_t result;
	
	if (phase == 0) {
		result.is_sp_updated = 1;
		result.sp = sp + 2; 		// shift(2)
		result.is_opc_done = 0;
	} 
	else if (phase == 1) {
		result.is_ram_write = 0;
		result.is_ram_read = 1;
		result.ram_addr = pc; // peek RAM (byte 1 of 2) at address equal to PC
	}
	else if (phase == 2) {
		result.is_ram_write = 0;
		result.is_ram_read = 1;
		result.ram_addr = pc; 
	}
	else if (phase == 3) {
		lit2_tmp16 = (uint16_t)(previous_ram_read);
		lit2_tmp16 <<= 8;
		result.is_ram_write = 0;
		result.is_ram_read = 1;
		result.ram_addr = pc + 1; // peek RAM (byte 2 of 2) at address equal to PC + 1
	}
	else if (phase == 4) {
		result.is_ram_write = 0;
		result.is_ram_read = 1;
		result.ram_addr = pc + 1; 
	}
	else if (phase == 5) {
		lit2_tmp16 |= ((uint16_t)(previous_ram_read));
		result.is_pc_updated = 1;
		result.pc = pc + 2; // pc += 2
	}
	else if (phase == 6) {
		result.is_ram_read = 0;
		result.ram_addr = 0;
		result.is_pc_updated = 0;
		result.is_stack_write = 1;
		result.is_stack_read = 0;
		result.stack_address = sp - 1;
		result.stack_value = (uint8_t)(lit2_tmp16);	// set T2 (low byte) to value of RAM at PC 
	}
	else if (phase == 7) {
		result.is_stack_write = 1;
		result.is_stack_read = 0;
		result.stack_address = sp - 2;
		result.stack_value = (uint8_t)(lit2_tmp16 >> 8); // set T2 (high byte) to value of RAM at PC + 1
	}
	else {
		result.is_stack_write = 0;
		result.stack_address = 0;
		result.is_opc_done = 1;
	}
	
	printf("            LIT2 phase 0x%X, sp = 0x%X, pc = 0x%X, previous_ram_read = 0x%X, tmp16 = 0x%X\n", phase, sp, pc, previous_ram_read, lit2_tmp16);
	
	return result;
}

opcode_result_t pop(uint8_t phase, uint1_t stack_index, uint8_t ins, uint16_t pc, uint8_t sp, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// SET(1,-1)
	static int8_t tmp8;
	static opcode_result_t result;
	if (phase == 0) {
		printf("            POP\n");
		result.is_sp_updated = 1;
		result.sp = (ins & 0x80) ? sp : sp - 1; // x=1;y=(-1); shift amount = ((ins & 0x80) ? x + y : y) ====> 0 or -1
		result.is_opc_done = 0;
	}
	else {
		result.is_sp_updated = 0;
		result.sp = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t pop2(uint8_t phase, uint1_t stack_index, uint8_t ins, uint16_t pc, uint8_t sp, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// SET(2,-2)
	static opcode_result_t result;
	if (phase == 0) {
		printf("            POP2\n");
		// set_sp(2, -2, ins, stack_index);
		result.is_sp_updated = 1;
		result.sp = (ins & 0x80) ? sp : sp - 2; // x=2;y=(-2); shift amount = ((ins & 0x80) ? x + y : y) ====> 0 or -2
		result.is_opc_done = 0;
	}
	else {
		result.is_sp_updated = 0;
		result.sp = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t ovr2(uint8_t phase, uint1_t stack_index, uint8_t ins, uint16_t pc, uint8_t sp, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// t=T2;n=N2;      SET(4, 2) T2_(n) N2_(t) L2_(n) break;
	static uint16_t t16, n16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("            OVR2\n");
		result.is_stack_write = 0;
		result.is_stack_read = 1;
		result.stack_address = sp - 2; // get T2 (byte 1 of 2)
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_read = 1;
		result.stack_address = sp - 2; 
	}
	else if (phase == 2) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.is_stack_read = 1;
		result.stack_address = sp - 1; // get T2 (byte 2 of 2)
	}
	else if (phase == 3) {
		result.is_stack_read = 1;
		result.stack_address = sp - 1; 
	}
	else if (phase == 4) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 1;
		result.stack_address = sp - 4;  // get N2 (byte 1 of 2)
	}
	else if (phase == 5) {
		result.is_stack_read = 1;
		result.stack_address = sp - 4; 
	}
	else if (phase == 6) {
		n16 = (uint16_t)(previous_stack_read);
		n16 <<= 8;
		result.is_stack_read = 1;
		result.stack_address = sp - 3; // get N2 (byte 2 of 2)
	}
	else if (phase == 7) {
		result.is_stack_read = 1;
		result.stack_address = sp - 3; 
	}
	else if (phase == 8) {
		n16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.stack_address = 0;
		result.is_sp_updated = 1;
		result.sp = (ins & 0x80) ? sp + 6 : sp + 2; // x=4;y=2; shift amount = ((ins & 0x80) ? x + y : y) ====> 0 or -2
	}
	else if (phase == 9) {
		result.is_sp_updated = 0;
		result.sp = 0;
		result.is_stack_write = 1;
		result.is_stack_read = 0;
		result.stack_address = sp - 1;
		result.stack_value = (uint8_t)(n16);	// set T2 = previous N2 (low byte)
	}
	else if (phase == 10) {
		result.is_stack_write = 1;
		result.stack_address = sp - 2;
		result.stack_value = (uint8_t)(n16 >> 8); // set T2 = previous N2 (high byte)
	}
	else if (phase == 11) {
		result.is_stack_write = 1;
		result.stack_address = sp - 3;
		result.stack_value = (uint8_t)(t16);	// set N2 = previous T2 (low byte)
	}
	else if (phase == 12) {
		result.is_stack_write = 1;
		result.stack_address = sp - 4;
		result.stack_value = (uint8_t)(t16 >> 8); // set N2 = previous T2 (high byte)
	}
	else if (phase == 13) {
		result.is_stack_write = 1;
		result.stack_address = sp - 5;
		result.stack_value = (uint8_t)(n16);	// set L2 = previous N2 (low byte)
	}
	else if (phase == 14) {
		result.is_stack_write = 1;
		result.stack_address = sp - 6;
		result.stack_value = (uint8_t)(n16 >> 8); // set L2 = previous N2 (high byte)
	}
	else if (phase == 15) {
		result.is_stack_write = 0;
		result.stack_address = 0;
		result.stack_value = 0;
		result.is_opc_done = 1;
	}
	
	return result;
}

opcode_result_t deo(uint8_t phase, uint1_t stack_index, uint8_t ins, uint16_t pc, uint8_t sp, uint8_t previous_stack_read, uint8_t previous_ram_read, uint8_t previous_device_ram_read) {
	// t=T;n=N;        SET(2,-2) DEO(t, n)
	static uint8_t t8, n8;
	static opcode_result_t result;
	static device_out_result_t device_out_result;
	if (phase == 0) {
		printf("            DEO\n");
		result.is_stack_write = 0;
		result.is_stack_read = 1;
		result.stack_address = sp - 1; // get T
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_stack_read = 1;
		result.stack_address = sp - 1; // get T
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 1;
		result.stack_address = sp - 2; // get N
	}
	else if (phase == 3) {
		result.is_stack_read = 1;
		result.stack_address = sp - 2; // get N
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_sp_updated = 1;
		result.sp = (ins & 0x80) ? sp : sp - 2; // x=2;y=(-2); shift amount = ((ins & 0x80) ? x + y : y) ====> 0 or -2
	}
	else {
		printf("                DEO (t8, n8) => (0x%X, 0x%X) phase 0x%X\n", t8, n8, phase - 5);
		result.is_sp_updated = 0;
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

opcode_result_t deo2(uint8_t phase, uint1_t stack_index, uint8_t ins, uint16_t pc, uint8_t sp, uint8_t previous_stack_read, uint8_t previous_ram_read, uint8_t previous_device_ram_read) {
	// t=T;n=N;l=L;    SET(3,-3) DEO(t, l) DEO((t + 1), n)
	// static uint16_t t16, n16;
	static uint8_t t8, n8, l8, current_deo_phase, deo_param0, deo_param1;
	static uint1_t is_second_deo = 0;
	static opcode_result_t result;
	static device_out_result_t device_out_result;
	if (phase == 0) {
		printf("            DEO2\n");
		result.is_stack_write = 0;
		result.is_stack_read = 1;
		result.stack_address = sp - 1; // get T
		result.is_opc_done = 0;
		is_second_deo = 0;
		current_deo_phase = 0;
	}
	else if (phase == 1) {
		result.is_stack_read = 1;
		result.stack_address = sp - 1; 
	}
	else if (phase == 2) {
		t8 = previous_stack_read;
		result.is_stack_read = 1;
		result.stack_address = sp - 2; // get N
	}
	else if (phase == 3) {
		result.is_stack_read = 1;
		result.stack_address = sp - 2; 
	}
	else if (phase == 4) {
		n8 = previous_stack_read;
		result.is_stack_read = 1;
		result.stack_address = sp - 3; // get L
	}
	else if (phase == 5) {
		result.is_stack_read = 1;
		result.stack_address = sp - 3;
	}
	else if (phase == 6) {
		l8 = previous_stack_read;
		result.is_stack_read = 0;
		result.stack_address = 0;
		result.is_sp_updated = 1;
		result.sp = (ins & 0x80) ? sp : sp - 3; // x=3;y=-3; shift amount = ((ins & 0x80) ? x + y : y) ====> 0 or -3
		// device_out(t8, l8, 4);
	}
	else {
		result.is_sp_updated = 0;
		result.sp = 0;
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

opcode_result_t jmp2(uint8_t phase, uint1_t stack_index, uint8_t ins, uint16_t pc, uint8_t sp, uint8_t previous_stack_read, uint8_t previous_ram_read) {
	// SET(2,-2) pc = t;
	static uint16_t t16;
	static opcode_result_t result;
	if (phase == 0) {
		printf("            JMP2\n");
		result.is_sp_updated = 1;
		result.sp = (ins & 0x80) ? sp : sp - 2; // x=2;y=(-2); shift amount = ((ins & 0x80) ? x + y : y) ====> 0 or -2
		result.is_opc_done = 0;
	}
	else if (phase == 1) {
		result.is_sp_updated = 0;
		result.sp = 0;
		result.is_stack_write = 0;
		result.is_stack_read = 1;
		result.stack_address = sp - 2; // get T2 (byte 1 of 2)
	}
	else if (phase == 2) {
		result.is_stack_read = 1;
		result.stack_address = sp - 2; 
	}
	else if (phase == 3) {
		t16 = (uint16_t)(previous_stack_read);
		t16 <<= 8;
		result.is_stack_read = 1;
		result.stack_address = sp - 2; // get T2 (byte 2 of 2)
	}
	else if (phase == 4) {
		result.is_stack_read = 1;
		result.stack_address = sp - 2; 
	}
	else if (phase == 5) {
		t16 |= ((uint16_t)(previous_stack_read));
		result.is_stack_read = 0;
		result.stack_address = 0;
		result.is_pc_updated = 1;
		result.pc = t16; // pc = t16
	}
	else if (phase == 6) {
		result.is_pc_updated = 0;
		result.pc = 0;
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
	static uint8_t sp0, sp1, sp;
	static uint12_t opc;
	static uint1_t stack_index, opc_done;
	static uint1_t is_stack_read = 0;
	static uint1_t is_stack_write = 0;
	static uint16_t stack_address = 0;
	static uint8_t stack_write_value = 0;
	static uint8_t stack_read_value = 0;
	static uint8_t device_ram_read_value = 0;
	static opcode_result_t opc_result = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	static eval_opcode_result_t opc_eval_result = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	opc = ins & 0x1f ? ((uint12_t)(ins & 0x3f)) : ((uint12_t)(ins) << 4);
	stack_index = ins & 0x40 ? 1 : 0;
	sp = stack_index == 0 ? sp0 : sp1;
	opc_done = 1;
	opc_result.is_opc_done = 1;
	
	printf("        EVAL OPCODE: INS = 0x%X, OPC = 0x%X, phase = 0x%X\n", ins, opc, phase);
	
	if      (opc == 0x000 /* BRK   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x200 /* JCI   */) { opc_result = jci(phase, stack_index, pc, sp, stack_read_value, previous_ram_read); }
	else if (opc == 0x400 /* JMI   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x600 /* JSI   */) { opc_result = jsi(phase, stack_index, pc, sp, stack_read_value, previous_ram_read); }
	else if (opc == 0x800 /* LIT   */) { opc_result = lit(phase, stack_index, pc, sp, stack_read_value, previous_ram_read); }
	else if (opc == 0xA00 /* LIT2  */) { opc_result = lit2(phase, stack_index, pc, sp, stack_read_value, previous_ram_read); }
	else if (opc == 0xC00 /* LITr  */) { opc_result = lit(phase, stack_index, pc, sp, stack_read_value, previous_ram_read); }
	else if (opc == 0xE00 /* LIT2r */) { opc_result = lit2(phase, stack_index, pc, sp, stack_read_value, previous_ram_read); }
	else if (opc == 0x001 /* INC   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x021 /* INC2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x002 /* POP   */) { opc_result = pop(phase, stack_index, ins, pc, sp, stack_read_value, previous_ram_read); }
	else if (opc == 0x022 /* POP2  */) { opc_result = pop2(phase, stack_index, ins, pc, sp, stack_read_value, previous_ram_read); }
	else if (opc == 0x003 /* NIP   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x023 /* NIP2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x004 /* SWP   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x024 /* SWP2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x005 /* ROT   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x025 /* ROT2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x006 /* DUP   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x026 /* DUP2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x007 /* OVR   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x027 /* OVR2  */) { opc_result = ovr2(phase, stack_index, ins, pc, sp, stack_read_value, previous_ram_read); }
	else if (opc == 0x008 /* EQU   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x028 /* EQU2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x009 /* NEQ   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x029 /* NEQ2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x00A /* GTH   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x02A /* GHT2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x00B /* LTH   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x02B /* LTH2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x00C /* JMP   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x02C /* JMP2  */) { opc_result = jmp2(phase, stack_index, ins, pc, sp, stack_read_value, previous_ram_read); }
	else if (opc == 0x00D /* JCN   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x02D /* JCN2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x00E /* JSR   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x02E /* JSR2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x00F /* STH   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x02F /* STH2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x010 /* LDZ   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x030 /* LDZ2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x011 /* STZ   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x031 /* STZ2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x012 /* LDR   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x032 /* LDR2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x013 /* STR   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x033 /* STR2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x014 /* LDA   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x034 /* LDA2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x015 /* STA   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x035 /* STA2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x016 /* DEI   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x036 /* DEI2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x017 /* DEO   */) { opc_result = deo(phase, stack_index, ins, pc, sp, stack_read_value, previous_ram_read, device_ram_read_value); }
	else if (opc == 0x037 /* DEO2  */) { opc_result = deo2(phase, stack_index, ins, pc, sp, stack_read_value, previous_ram_read, device_ram_read_value); }
	else if (opc == 0x018 /* ADD   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x038 /* ADD2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x019 /* SUB   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x039 /* SUB2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x01A /* MUL   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x03A /* MUL2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x01B /* DIV   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x03B /* DIV2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x01C /* AND   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x03C /* AND2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x01D /* ORA   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x03D /* ORA2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x01E /* EOR   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x03E /* EOR2  */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x01F /* SFT   */) { opc_result.is_opc_done = 1; }
	else if (opc == 0x03F /* SFT2  */) { opc_result.is_opc_done = 1; }
	
	is_stack_read = opc_result.is_stack_read;
	is_stack_write = opc_result.is_stack_write;
	stack_address = opc_result.stack_address;
	stack_write_value = opc_result.stack_value;
	
	if (stack_index) {
		stack_read_value = stack_r_ram_update(
			stack_address, 
			stack_write_value,
			is_stack_write, // write 0 enable
			0,				// read 0 enable
			stack_address,
			is_stack_read
		);
	} else {
		stack_read_value = stack_w_ram_update(
			stack_address, 
			stack_write_value,
			is_stack_write, // write 0 enable
			0,				// read 0 enable
			stack_address,
			is_stack_read
		);
	}
	
	if (opc_result.is_sp_updated) {
		if (stack_index == 0) {
			sp0 = opc_result.sp;
		} else {
			sp1 = opc_result.sp;
		}
	}
	
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
	printf("   EVAL OPCODE DONE: is_vram_write = 0x%X, vram_address = 0x%X, vram_value = 0x%X\n", opc_result.is_vram_write, opc_result.vram_address, opc_result.vram_value);
	
	return opc_eval_result;
}
