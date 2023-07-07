#include "uintN_t.h"  // uintN_t types for any N
#include "intN_t.h"  // intN_t types for any N

// #include "device.h"
// #include "stack.h"
// #include "registers.h"

uint8_t dev[256];			// 256B of device memory. 16 devices at 16 bytes each
uint8_t ram[65536];			// 64KB RAM
uint16_t pc = 0;			// program counter
uint8_t stack_data[2][255]; // 0 = working stack, 1 = return stack
uint8_t stack_ptr[2];  		// 0 = working stack ptr, 1 = return stack ptr

void boot() {
	
}

// SHARED

#pragma MAIN uxn_halt
uint8_t uxn_halt(uint8_t instr, uint8_t err, uint16_t addr)
{
	// TODO: implement
	uint16_t handler = peek2_dev(0);
	if(handler) {
		stack_ptr[0] = 4;
		stack_data[0][0] = addr >> 0x8;
		stack_data[0][1] = addr & 0xff;
		stack_data[0][2] = instr;
		stack_data[0][3] = err;
		return uxn_eval(handler);
	} else {
		// system_inspect(u);
		// fprintf(stderr, "%s %s, by %02x at 0x%04x.\n", (instr & 0x40) ? "Return-stack" : "Working-stack", errors[err - 1], instr, addr);
		return 0;
	}
}

uint16_t peek2_ram(uint16_t address) {
	uint16_t mem0 = (uint16_t)(ram[address]);
	uint16_t mem1 = (uint16_t)(ram[address + 1]);
	return (mem0 << 8) | mem1;
}

uint16_t peek2_stack(uint1_t stack_index, uint8_t address) {
	// stack_index: 0 = working stack, 1 = return stack
	uint16_t mem0 = (uint16_t)(stack_data[stack_index][address]);
	uint16_t mem1 = (uint16_t)(stack_data[stack_index][address + 1]);
	return (mem0 << 8) | mem1;
}

uint16_t peek2_dev(uint8_t address) {
	uint16_t mem0 = (uint16_t)(dev[address]);
	uint16_t mem1 = (uint16_t)(dev[address + 1]);
	return (mem0 << 8) | mem1;
}

uint8_t push2_stack_func(uint1_t stack_index, uint8_t ins, uint16_t value) {
	static uint16_t push2_tmp;
	if (stack_ptr[stack_index] > 253) {
		return halt(ins, 2);
	}
	
	return 0;
	// stack_index: 0 = working stack, 1 = return stack
	/*
	#define PUSH2(x, v) { z = (x); if(z->ptr > 253) HALT(2) tmp = (v); z->dat[z->ptr] = tmp >> 8; z->dat[z->ptr + 1] = tmp; z->ptr += 2; }
	*/
	
}

#pragma MAIN halt
uint8_t halt(uint8_t ins, uint8_t err) {
	// HALT(c) { return uxn_halt(u, ins, (c), pc - 1); }
	// Ex: HALT(3)
	return uxn_halt(ins, err, pc - 1);
}

// REGISTERS

#pragma MAIN t_register_func
uint8_t t_register_func(uint1_t stack_index) {
	return stack_data[stack_index][stack_ptr[stack_index] - 1];
}

#pragma MAIN n_register_func
uint8_t n_register_func(uint1_t stack_index) {
	return stack_data[stack_index][stack_ptr[stack_index] - 2];
}

#pragma MAIN l_register_func
uint8_t l_register_func(uint1_t stack_index) {
	return stack_data[stack_index][stack_ptr[stack_index] - 3];
}

#pragma MAIN h2_register_func
uint16_t h2_register_func(uint1_t stack_index) {
	return peek2_stack_func(stack_index, stack_ptr[stack_index] - 3);
}

#pragma MAIN t2_register_func
uint16_t t2_register_func(uint1_t stack_index) {
	return peek2_stack_func(stack_index, stack_ptr[stack_index]- 2);
}

#pragma MAIN n2_register_func
uint16_t n2_register_func(uint1_t stack_index) {
	return peek2_stack_func(stack_index, stack_ptr[stack_index] - 4);
}

#pragma MAIN l2_register_func
uint16_t l2_register_func(uint1_t stack_index) {
	return peek2_stack_func(stack_index, stack_ptr[stack_index] - 6);
}

// INSTRUCTION HANDLING

#pragma MAIN uxn_eval
uint1_t uxn_eval(uint16_t pc) {

	static uint8_t k, tmp, opc, ins;
	static uint1_t s;

	if(pc == 0 || dev[0x0f] != 0) {
		return 0;
	}
	
	for(;;) {
		ins = ram[pc++] & 0xff;
		k = ins & 0x80 ? 0xff : 0;
		s = ins & 0x40 ? 1 : 0;
		opc = !(ins & 0x1f) ? (0 - (ins >> 5)) & 0xff : ins & 0x3f;	
		return eval_opcode(s, opc, t, nl, l, k, tmp, ins);
	}
}

#pragma MAIN eval_opcode
uint1_t eval_opcode(
	uint1_t stack_index,
	uint8_t opcode,
	uint8_t ins,
	uint8_t k
) {
	static uint8_t 	t, n, l, tmp;

	switch(opcode) {
		/* IMM */
		case 0x00: /* BRK   */ return 1;
		case 0xff: /* JCI   */ pc += !!stack_data[stack_index][--stack_ptr[stack_index]] * peek2_ram_func(pc) + 2; break;
		case 0xfe: /* JMI   */ pc += peek2_ram_func(pc) + 2; break;
		case 0xfd: /* JSI   */ PUSH2(&u->rst, pc + 2) pc += PEEK2(ram + pc) + 2; break;
		case 0xfc: /* LIT   */ PUSH(s, ram[pc++]) break;
		case 0xfb: /* LIT2  */ PUSH2(s, PEEK2(ram + pc)) pc += 2; break;
		case 0xfa: /* LITr  */ PUSH(s, ram[pc++]) break;
		case 0xf9: /* LIT2r */ PUSH2(s, PEEK2(ram + pc)) pc += 2; break;
		/* ALU */
		case 0x01: /* INC  */ t=T;            SET(1, 0) PUT(0, t + 1) break;
		case 0x21:            t=T2;           SET(2, 0) PUT2(0, t + 1) break;
		case 0x02: /* POP  */                 SET(1,-1) break;
		case 0x22:                            SET(2,-2) break;
		case 0x03: /* NIP  */ t=T;            SET(2,-1) PUT(0, t) break;
		case 0x23:            t=T2;           SET(4,-2) PUT2(0, t) break;
		case 0x04: /* SWP  */ t=T;n=N;        SET(2, 0) PUT(0, n) PUT(1, t) break;
		case 0x24:            t=T2;n=N2;      SET(4, 0) PUT2(0, n) PUT2(2, t) break;
		case 0x05: /* ROT  */ t=T;n=N;l=L;    SET(3, 0) PUT(0, l) PUT(1, t) PUT(2, n) break;
		case 0x25:            t=T2;n=N2;l=L2; SET(6, 0) PUT2(0, l) PUT2(2, t) PUT2(4, n) break;
		case 0x06: /* DUP  */ t=T;            SET(1, 1) PUT(0, t) PUT(1, t) break;
		case 0x26:            t=T2;           SET(2, 2) PUT2(0, t) PUT2(2, t) break;
		case 0x07: /* OVR  */ t=T;n=N;        SET(2, 1) PUT(0, n) PUT(1, t) PUT(2, n) break;
		case 0x27:            t=T2;n=N2;      SET(4, 2) PUT2(0, n) PUT2(2, t) PUT2(4, n) break;
		case 0x08: /* EQU  */ t=T;n=N;        SET(2,-1) PUT(0, n == t) break;
		case 0x28:            t=T2;n=N2;      SET(4,-3) PUT(0, n == t) break;
		case 0x09: /* NEQ  */ t=T;n=N;        SET(2,-1) PUT(0, n != t) break;
		case 0x29:            t=T2;n=N2;      SET(4,-3) PUT(0, n != t) break;
		case 0x0a: /* GTH  */ t=T;n=N;        SET(2,-1) PUT(0, n > t) break;
		case 0x2a:            t=T2;n=N2;      SET(4,-3) PUT(0, n > t) break;
		case 0x0b: /* LTH  */ t=T;n=N;        SET(2,-1) PUT(0, n < t) break;
		case 0x2b:            t=T2;n=N2;      SET(4,-3) PUT(0, n < t) break;
		case 0x0c: /* JMP  */ t=T;            SET(1,-1) pc += (Sint8)t; break;
		case 0x2c:            t=T2;           SET(2,-2) pc = t; break;
		case 0x0d: /* JCN  */ t=T;n=N;        SET(2,-2) pc += !!n * (Sint8)t; break;
		case 0x2d:            t=T2;n=L;       SET(3,-3) if(n) pc = t; break;
		case 0x0e: /* JSR  */ t=T;            SET(1,-1) PUSH2(&u->rst, pc) pc += (Sint8)t; break;
		case 0x2e:            t=T2;           SET(2,-2) PUSH2(&u->rst, pc) pc = t; break;
		case 0x0f: /* STH  */ t=T;            SET(1,-1) PUSH((ins & 0x40 ? &u->wst : &u->rst), t) break;
		case 0x2f:            t=T2;           SET(2,-2) PUSH2((ins & 0x40 ? &u->wst : &u->rst), t) break;
		case 0x10: /* LDZ  */ t=T;            SET(1, 0) PUT(0, ram[t]) break;
		case 0x30:            t=T;            SET(1, 1) PUT2(0, PEEK2(ram + t)) break;
		case 0x11: /* STZ  */ t=T;n=N;        SET(2,-2) ram[t] = n; break;
		case 0x31:            t=T;n=H2;       SET(3,-3) POKE2(ram + t, n) break;
		case 0x12: /* LDR  */ t=T;            SET(1, 0) PUT(0, ram[pc + (Sint8)t]) break;
		case 0x32:            t=T;            SET(1, 1) PUT2(0, PEEK2(ram + pc + (Sint8)t)) break;
		case 0x13: /* STR  */ t=T;n=N;        SET(2,-2) ram[pc + (Sint8)t] = n; break;
		case 0x33:            t=T;n=H2;       SET(3,-3) POKE2(ram + pc + (Sint8)t, n) break;
		case 0x14: /* LDA  */ t=T2;           SET(2,-1) PUT(0, ram[t]) break;
		case 0x34:            t=T2;           SET(2, 0) PUT2(0, PEEK2(ram + t)) break;
		case 0x15: /* STA  */ t=T2;n=L;       SET(3,-3) ram[t] = n; break;
		case 0x35:            t=T2;n=N2;      SET(4,-4) POKE2(ram + t, n) break;
		case 0x16: /* DEI  */ t=T;            SET(1, 0) DEI(0, t) break;
		case 0x36:            t=T;            SET(1, 1) DEI(1, t) DEI(0, t + 1) break;
		case 0x17: /* DEO  */ t=T;n=N;        SET(2,-2) DEO(t, n) break;
		case 0x37:            t=T;n=N;l=L;    SET(3,-3) DEO(t, l) DEO(t + 1, n) break;
		case 0x18: /* ADD  */ t=T;n=N;        SET(2,-1) PUT(0, n + t) break;
		case 0x38:            t=T2;n=N2;      SET(4,-2) PUT2(0, n + t) break;
		case 0x19: /* SUB  */ t=T;n=N;        SET(2,-1) PUT(0, n - t) break;
		case 0x39:            t=T2;n=N2;      SET(4,-2) PUT2(0, n - t) break;
		case 0x1a: /* MUL  */ t=T;n=N;        SET(2,-1) PUT(0, n * t) break;
		case 0x3a:            t=T2;n=N2;      SET(4,-2) PUT2(0, n * t) break;
		case 0x1b: /* DIV  */ t=T;n=N;        SET(2,-1) if(!t) HALT(3) PUT(0, n / t) break;
		case 0x3b:            t=T2;n=N2;      SET(4,-2) if(!t) HALT(3) PUT2(0, n / t) break;
		case 0x1c: /* AND  */ t=T;n=N;        SET(2,-1) PUT(0, n & t) break;
		case 0x3c:            t=T2;n=N2;      SET(4,-2) PUT2(0, n & t) break;
		case 0x1d: /* ORA  */ t=T;n=N;        SET(2,-1) PUT(0, n | t) break;
		case 0x3d:            t=T2;n=N2;      SET(4,-2) PUT2(0, n | t) break;
		case 0x1e: /* EOR  */ t=T;n=N;        SET(2,-1) PUT(0, n ^ t) break;
		case 0x3e:            t=T2;n=N2;      SET(4,-2) PUT2(0, n ^ t) break;
		case 0x1f: /* SFT  */ t=T;n=N;        SET(2,-1) PUT(0, n >> (t & 0xf) << (t >> 4)) break;
		case 0x3f:            t=T;n=H2;       SET(3,-1) PUT2(0, n >> (t & 0xf) << (t >> 4)) break;
		default: break;
	}
	
	return 0;
}