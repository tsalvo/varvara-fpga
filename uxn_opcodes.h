#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "intN_t.h"   // intN_t types for any N

#pragma once
#include "uxn_ram_main.h"
#pragma once
#include "uxn_pc.h"
#pragma once
#include "uxn_stack.h"
#pragma once
#include "uxn_device.h"
#pragma once
#include "uxn_registers.h"


// 
// #define DEI(p) (dei_masks[p] ? emu_dei(u, (p)) : u->dev[(p)])
// #define DEO(p, v) { u->dev[p] = v; if(deo_masks[p]) emu_deo(u, p); }
// 
// #define FLIP      { s = ins & 0x40 ? &u->wst : &u->rst; }
// #define SHIFT(y)  { s->ptr += (y); ptr = s->dat + s->ptr - 1; }
// #define SET(x, y) { SHIFT((ins & 0x80) ? x + y : y) }

// lit lit2 deo deo2 jsi pop2r jmp2 ovr2

uint1_t jci(uint4_t phase, uint1_t stack_index) {
	static uint8_t t8;
	static uint16_t pc;
	static uint1_t result;
	if (phase == 0) {
		t8 = get_t_2p(stack_index); // START
		pc = pc_get(); // DONE
		result = 0;
	}
	else if (phase == 1) {
		t8 = get_t_2p(stack_index); // DONE
	}
	else if (phase == 2) {
		shift(-1, stack_index);
	}
	else if (phase == 3) {
		pc_set(t8 == 0 ? pc + 2 : pc);
	}
	else {
		result = 1;
	}
	
	return result;
}

uint1_t jsi(uint4_t phase, uint1_t stack_index) {
	// SHIFT( 2) T2_(pc + 2); rr = ram + pc; pc += PEEK2(rr) + 2;
	static uint16_t pc, tmp16;
	static uint1_t result;
	if (phase == 0) {
		shift(2, stack_index);
		pc = pc_get();
		result = 0;
	}
	else if (phase == 1) {
		set_t2_2p(pc + 2, 0, stack_index); // START
	}
	else if (phase == 2) {
		set_t2_2p(pc + 2, 1, stack_index); // DONE
	}
	else if (phase == 3) {
		tmp16 = peek2_ram(pc); // START
	}
	else if (phase == 4) {
		tmp16 = peek2_ram(pc); // DONE
	}
	else if (phase == 5) {
		pc_set(pc + tmp16 + 2);
	}
	else {
		result = 1;
	}
	
	return result;
}

uint1_t lit(uint4_t phase, uint1_t stack_index) {
	// SHIFT( 1) T = ram[pc++];
	static uint8_t tmp8;
	static uint16_t pc;
	static uint1_t result;
	if (phase == 0) {
		shift(1, stack_index);
		pc = pc_get();
		result = 0;
	} 
	else if (phase == 1) {
		tmp8 = peek_ram(pc); // START
	}
	else if (phase == 2) {
		tmp8 = peek_ram(pc); // DONE
	}
	else if (phase == 3) {
		pc_set(pc + 1);
	}
	else if (phase == 4) {
		set_t(tmp8, stack_index); // START
	}
	else if (phase == 5) {
		result = 1;
	}
	
	return result;
}

uint1_t lit2(uint4_t phase, uint1_t stack_index) {
	// SHIFT( 2) rr = ram + pc; T2_(PEEK2(rr)) pc += 2;
	static uint16_t pc, tmp16;
	static uint1_t result;
	if (phase == 0) {
		shift(2, stack_index);
		pc = pc_get();
		result = 0;
	} 
	else if (phase == 1) {
		tmp16 = peek2_ram(pc); // START
	}
	else if (phase == 2) {
		tmp16 = peek2_ram(pc); // DONE
	}
	else if (phase == 3) {
		pc_set(pc + 2);
	}
	else if (phase == 4) {
		set_t2_2p(tmp16, 0, stack_index); // START
	}
	else if (phase == 5) {
		set_t2_2p(tmp16, 1, stack_index); // DONE
	}
	else {
		result = 1;
	}
	
	return result;
}

uint1_t pop(uint4_t phase, uint1_t stack_index, uint8_t ins) {
	// SET(1,-1)
	static uint1_t result;
	if (phase == 0) {
		set_sp(1, -1, ins, stack_index);
		result = 0;
	}
	else {
		result = 1;
	}
	
	return result;
}

uint1_t pop2(uint4_t phase, uint1_t stack_index, uint8_t ins) {
	// SET(2,-2)
	static uint1_t result;
	if (phase == 0) {
		set_sp(2, -2, ins, stack_index);
		result = 0;
	}
	else {
		result = 1;
	}
	
	return result;
}

uint1_t ovr2(uint4_t phase, uint1_t stack_index, uint8_t ins) {
	// t=T2;n=N2;      SET(4, 2) T2_(n) N2_(t) L2_(n) break;
	static uint16_t t16, n16;
	static uint1_t result;
	if (phase == 0) {
		t16 = get_t2_2p(0, stack_index); // START
		result = 0;
	}
	else if (phase == 1) {
		t16 = get_t2_2p(1, stack_index); // DONE
	}
	else if (phase == 2) {
		n16 = get_n2_2p(0, stack_index); // START
	}
	else if (phase == 3) {
		n16 = get_n2_2p(1, stack_index); // DONE
	}
	else if (phase == 4) {
		set_sp(4, 2, ins, stack_index);
	}
	else if (phase == 5) {
		set_t2_2p(n16, 0, stack_index); // START
	}
	else if (phase == 6) {
		set_t2_2p(n16, 1, stack_index); // DONE
	}
	else if (phase == 7) {
		set_n2_2p(t16, 0, stack_index); // START
	}
	else if (phase == 8) {
		set_n2_2p(t16, 1, stack_index); // DONE
	}
	else if (phase == 9) {
		set_l2_2p(n16, 0, stack_index); // START
	}
	else if (phase == 10) {
		set_l2_2p(n16, 1, stack_index); // DONE
	}
	else {
		result = 1;
	}
	
	return result;
}

uint1_t deo(uint4_t phase, uint1_t stack_index, uint8_t ins) {
	// t=T;n=N;        SET(2,-2) DEO(t, n)
	static uint16_t t16;
	static uint8_t t8, n8;
	static uint1_t result;
	if (phase == 0) {
		t16 = get_t2_2p(0, stack_index); // START
		result = 0;
	}
	else if (phase == 1) {
		t16 = get_t2_2p(1, stack_index); // DONE
		t8 = (uint8_t)(t16);
		n8 = (uint8_t)(t16 >> 8);
		set_sp(2, -2, ins, stack_index);
	}
	else if (phase == 2) {
		result = device_out(t8, n8, 0);
	}
	else if (phase == 3) {
		result = device_out(t8, n8, 1);
	}
	else if (phase == 4) {
		result = device_out(t8, n8, 2);
	}
	else if (phase == 5) {
		result = device_out(t8, n8, 3);
	}
	else if (phase == 6) {
		result = device_out(t8, n8, 4);
	}
	else if (phase == 7) {
		result = device_out(t8, n8, 5);
	}
	else if (phase == 8) {
		result = device_out(t8, n8, 6);
	}
	else {
		result = 1;
	}
	
	return result;
}

uint1_t deo2(uint4_t phase, uint1_t stack_index, uint8_t ins) {
	// t=T;n=N;l=L;    SET(3,-3) DEO(t, l) DEO((t + 1), n)
	static uint16_t t16, n16;
	static uint8_t sp, t8, n8, l8;
	static uint1_t result;
	if (phase == 0) {
		sp = stack_pointer_get(stack_index);
		t16 = peek2_stack(stack_index, sp - 2); // START T2
		result = 0;
	}
	else if (phase == 1) {
		t16 = peek2_stack(stack_index, sp - 4); // DONE T2 / START N2
		t8 = (uint8_t)(t16);
		n8 = (uint8_t)(t16 >> 8);	
		device_out(t8 + 1, n8, 0); // second DEO call at phase 0 with dev memory write (t and n are ready)
	}
	else if (phase == 2) {
		n16 = peek2_stack(stack_index, sp - 4); // DONE N2
		l8 = (uint8_t)(n16);
		set_sp(3, -3, ins, stack_index);
		device_out(t8, l8, 0); // first DEO call at phase 0 with dev memory write
	}
	else if (phase == 3) {
		device_out(t8, l8, 1);
	}
	else if (phase == 4) {
		device_out(t8, l8, 2);
	}
	else if (phase == 5) {
		device_out(t8, l8, 3);
	}
	else if (phase == 6) {
		device_out(t8, l8, 4);
	}
	else if (phase == 7) {
		device_out(t8, l8, 5);
	}
	else if (phase == 8) {
		device_out(t8, l8, 6);
	}
	else if (phase == 9) {
		result = device_out(t8 + 1, n8, 1);
	}
	else if (phase == 10) {
		result = device_out(t8 + 1, n8, 2);
	}
	else if (phase == 11) {
		result = device_out(t8 + 1, n8, 3);
	}
	else if (phase == 12) {
		result = device_out(t8 + 1, n8, 4);
	}
	else if (phase == 13) {
		result = device_out(t8 + 1, n8, 5);
	}
	else if (phase == 14) {
		device_out(t8 + 1, n8, 6);
		result = 1;
	}
	
	return result;
}

uint1_t jmp2(uint4_t phase, uint1_t stack_index, uint8_t ins) {
	// SET(2,-2) pc = t;
	static uint1_t result;
	static uint16_t pc;
	if (phase == 0) {
		set_sp(2, -2, ins, stack_index);
	}
	else if (phase == 1) {
		pc = pc_get();
		result = 0;
	}
	else if (phase == 2) {
		pc_set(pc + 2);
		result = 1;
	}
	
	return result;
}
	
uint1_t eval_opcode_phased(
	uint4_t phase,
	uint8_t ins
) {
	static uint12_t opc;
	static uint1_t stack_index, opc_result;
	opc = ins & 0x1f ? ((uint12_t)(ins & 0x3f)) : ((uint12_t)(ins) << 4);
	stack_index = ins & 0x40 ? 1 : 0;
	opc_result = 1;
	
	if      (opc == 0x000 /* BRK   */) { opc_result = 1; }
	else if (opc == 0x200 /* JCI   */) { opc_result = jci(phase, stack_index); }
	else if (opc == 0x400 /* JMI   */) { opc_result = 1; }
	else if (opc == 0x600 /* JSI   */) { opc_result = jsi(phase, stack_index); }
	else if (opc == 0x800 /* LIT   */) { opc_result = lit(phase, stack_index); }
	else if (opc == 0xA00 /* LIT2  */) { opc_result = lit2(phase, stack_index); }
	else if (opc == 0xC00 /* LITr  */) { opc_result = lit(phase, stack_index); }
	else if (opc == 0xE00 /* LIT2r */) { opc_result = lit2(phase, stack_index); }
	else if (opc == 0x001 /* INC   */) { opc_result = 1; }
	else if (opc == 0x021 /* INC2  */) { opc_result = 1; }
	else if (opc == 0x002 /* POP   */) { opc_result = pop(phase, stack_index, ins); }
	else if (opc == 0x022 /* POP2  */) { opc_result = pop2(phase, stack_index, ins); }
	else if (opc == 0x003 /* NIP   */) { opc_result = 1; }
	else if (opc == 0x023 /* NIP2  */) { opc_result = 1; }
	else if (opc == 0x004 /* SWP   */) { opc_result = 1; }
	else if (opc == 0x024 /* SWP2  */) { opc_result = 1; }
	else if (opc == 0x005 /* ROT   */) { opc_result = 1; }
	else if (opc == 0x025 /* ROT2  */) { opc_result = 1; }
	else if (opc == 0x006 /* DUP   */) { opc_result = 1; }
	else if (opc == 0x026 /* DUP2  */) { opc_result = 1; }
	else if (opc == 0x007 /* OVR   */) { opc_result = 1; }
	else if (opc == 0x027 /* OVR2  */) { opc_result = ovr2(phase, stack_index, ins); }
	else if (opc == 0x008 /* EQU   */) { opc_result = 1; }
	else if (opc == 0x028 /* EQU2  */) { opc_result = 1; }
	else if (opc == 0x009 /* NEQ   */) { opc_result = 1; }
	else if (opc == 0x029 /* NEQ2  */) { opc_result = 1; }
	else if (opc == 0x00A /* GTH   */) { opc_result = 1; }
	else if (opc == 0x02A /* GHT2  */) { opc_result = 1; }
	else if (opc == 0x00B /* LTH   */) { opc_result = 1; }
	else if (opc == 0x02B /* LTH2  */) { opc_result = 1; }
	else if (opc == 0x00C /* JMP   */) { opc_result = 1; }
	else if (opc == 0x02C /* JMP2  */) { opc_result = jmp2(phase, stack_index, ins); }
	else if (opc == 0x00D /* JCN   */) { opc_result = 1; }
	else if (opc == 0x02D /* JCN2  */) { opc_result = 1; }
	else if (opc == 0x00E /* JSR   */) { opc_result = 1; }
	else if (opc == 0x02E /* JSR2  */) { opc_result = 1; }
	else if (opc == 0x00F /* STH   */) { opc_result = 1; }
	else if (opc == 0x02F /* STH2  */) { opc_result = 1; }
	else if (opc == 0x010 /* LDZ   */) { opc_result = 1; }
	else if (opc == 0x030 /* LDZ2  */) { opc_result = 1; }
	else if (opc == 0x011 /* STZ   */) { opc_result = 1; }
	else if (opc == 0x031 /* STZ2  */) { opc_result = 1; }
	else if (opc == 0x012 /* LDR   */) { opc_result = 1; }
	else if (opc == 0x032 /* LDR2  */) { opc_result = 1; }
	else if (opc == 0x013 /* STR   */) { opc_result = 1; }
	else if (opc == 0x033 /* STR2  */) { opc_result = 1; }
	else if (opc == 0x014 /* LDA   */) { opc_result = 1; }
	else if (opc == 0x034 /* LDA2  */) { opc_result = 1; }
	else if (opc == 0x015 /* STA   */) { opc_result = 1; }
	else if (opc == 0x035 /* STA2  */) { opc_result = 1; }
	else if (opc == 0x016 /* DEI   */) { opc_result = 1; }
	else if (opc == 0x036 /* DEI2  */) { opc_result = 1; }
	else if (opc == 0x017 /* DEO   */) { opc_result = deo(phase, stack_index, ins); }
	else if (opc == 0x037 /* DEO2  */) { opc_result = deo2(phase, stack_index, ins); }
	else if (opc == 0x018 /* ADD   */) { opc_result = 1; }
	else if (opc == 0x038 /* ADD2  */) { opc_result = 1; }
	else if (opc == 0x019 /* SUB   */) { opc_result = 1; }
	else if (opc == 0x039 /* SUB2  */) { opc_result = 1; }
	else if (opc == 0x01A /* MUL   */) { opc_result = 1; }
	else if (opc == 0x03A /* MUL2  */) { opc_result = 1; }
	else if (opc == 0x01B /* DIV   */) { opc_result = 1; }
	else if (opc == 0x03B /* DIV2  */) { opc_result = 1; }
	else if (opc == 0x01C /* AND   */) { opc_result = 1; }
	else if (opc == 0x03C /* AND2  */) { opc_result = 1; }
	else if (opc == 0x01D /* ORA   */) { opc_result = 1; }
	else if (opc == 0x03D /* ORA2  */) { opc_result = 1; }
	else if (opc == 0x01E /* EOR   */) { opc_result = 1; }
	else if (opc == 0x03E /* EOR2  */) { opc_result = 1; }
	else if (opc == 0x01F /* SFT   */) { opc_result = 1; }
	else if (opc == 0x03F /* SFT2  */) { opc_result = 1; }
	
	return opc_result;
}
