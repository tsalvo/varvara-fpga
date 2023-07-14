#include "uintN_t.h"  // uintN_t types for any N
#include "intN_t.h"  // intN_t types for any N

// TODO: RULES:
// - cannot write to a gloal variable from more than one function
// - no switch statements (C AST node cannot be parsed to logic)
// only one return per function
// - no ++ or -- operators

uint8_t dev[256];           // 256B of device memory. 16 devices at 16 bytes each
uint8_t ram[65536];         // 64KB RAM
uint16_t pc = 0;            // program counter
uint8_t stack_data[2][255]; // 0 = working stack, 1 = return stack
uint8_t stack_ptr[2];       // 0 = working stack ptr, 1 = return stack ptr

// SHARED


// Global Access 
void pc_add(uint16_t adjustment) {
	pc += adjustment;
}

void stack_data_set(uint1_t stack_index, uint8_t index, uint8_t value) {
	stack_data[stack_index][index] = value;
}

void stack_pointer_set(uint1_t stack_index, uint8_t value) {
	stack_ptr[stack_index] = value;
}

void stack_pointer_move(uint1_t stack_index, uint8_t adjustment, uint1_t is_negative) {
	static uint8_t stack_ptr_existing;
	stack_ptr_existing = stack_ptr[stack_index];
	if (is_negative) {
		stack_ptr[stack_index] = stack_ptr_existing - adjustment;
	} else {
		stack_ptr[stack_index] = stack_ptr_existing + adjustment;
	}
}

uint8_t uxn_halt(uint8_t instr, uint8_t err, uint16_t addr)
{
	// TODO: implement
	uint16_t handler = peek2_dev(0);
	if(handler) {
		stack_ptr_set(0, 4);
		stack_data_set(0, 0, (uint8_t)(addr >> 8));
		stack_data_set(0, 1, (uint8_t)(addr & 0x00FF));
		stack_data_set(0, 2, instr);
		stack_data_set(0, 3, err);
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
	static uint8_t ret_value;
	static_uint1_t halt_return;
	
	if (stack_ptr[stack_index] > 253) {
		ret_value = halt(ins, 2);
		halt_return = 1;
	} else {
		ret_value = 0;
		halt_return = 0;
	}
	
	if (halt_return == 0) {
		push2_tmp = value;
		stack_data_set(stack_index, stack_ptr[stack_index], (uint8_t)(push2_tmp >> 8));
		stack_data_set(stack_index, stack_ptr[stack_index] + 1, (uint8_t)(push2_tmp));
		stack_ptr_move(stack_index, 2, 0);
	}
	
	return ret_value;
}

uint8_t push_stack_func(uint1_t stack_index, uint8_t ins, uint8_t value) {
	
	static uint8_t ret_value;
	static_uint1_t halt_return;
	
	if (stack_ptr[stack_index] > 254) {
		ret_value = halt(ins, 2);
		halt_return = 1;
	} else {
		ret_value = 0;
		halt_return = 0;
	}
	
	if (halt_return == 0) {
		stack_data_set(stack_index, stack_ptr[stack_index], value);
		stack_ptr_move(stack_index, 1, 0);
	}
	
	return ret_value;
}

uint8_t set_func(uint1_t stack_index, uint8_t ins, uint8_t k, uint8_t mul, int8_t add) {
	// SET(mul, add) { if(mul > s->ptr) HALT(1) tmp = (mul & k) + add + s->ptr; if(tmp > 254) HALT(2) s->ptr = tmp; }
	// Example: SET(2,1)
	
	static uint8_t ret_value;
	static_uint1_t halt_return;
	
	static uint8_t set_tmp, ret_value;
	if (mul > stack_ptr[stack_index]) {
		ret_value = halt(ins, 1);
		halt_return = 1;
	} else {
		ret_value = 0;
		halt_return = 0;
	}
	
	set_tmp = (mul & k) + add + stack_ptr[stack_index];
	if (set_tmp > 254) {
		ret_value = halt(ins, 2);
		halt_return = 1;
	}
	
	if (halt_return == 0) {
		stack_ptr_set(stack_index, set_tmp);
	}
	
	return 0;
}

void put_stack_func(uint1_t stack_index, uint8_t offset, uint8_t value) {
	// PUT(o, v) { s->dat[(Uint8)(s->ptr - 1 - (o))] = (v); }
	static uint8_t put_tmp;
	put_tmp = stack_ptr[stack_index] - 1 - offset;
	stack_data_set(stack_index, put_tmp, value);
}

void put2_stack_func(uint1_t stack_index, uint8_t offset, uint16_t value) {
	// PUT2(o, v) { tmp = (v); s->dat[(Uint8)(s->ptr - o - 2)] = tmp >> 8; s->dat[(Uint8)(s->ptr - o - 1)] = tmp; }
	static uint16_t put2_tmp;
	static uint8_t put2_tmp8 = stack_ptr[stack_index] - offset - 2;
	put2_tmp = value;
	stack_data_set(stack_index, put2_tmp8, (uint8_t)(tmp >> 8));
	stack_data_set(stack_index, put2_tmp8 + 1, (uint8_t)(tmp));
} 

uint8_t halt(uint8_t ins, uint8_t err) {
	// HALT(c) { return uxn_halt(u, ins, (c), pc - 1); }
	// Ex: HALT(3)
	return uxn_halt(ins, err, pc - 1);
}

// REGISTERS

uint8_t t_register_func(uint1_t stack_index) {
	return stack_data[stack_index][stack_ptr[stack_index] - 1];
}

uint8_t n_register_func(uint1_t stack_index) {
	return stack_data[stack_index][stack_ptr[stack_index] - 2];
}

uint8_t l_register_func(uint1_t stack_index) {
	return stack_data[stack_index][stack_ptr[stack_index] - 3];
}

uint16_t h2_register_func(uint1_t stack_index) {
	return peek2_stack_func(stack_index, stack_ptr[stack_index] - 3);
}

uint16_t t2_register_func(uint1_t stack_index) {
	return peek2_stack_func(stack_index, stack_ptr[stack_index]- 2);
}

uint16_t n2_register_func(uint1_t stack_index) {
	return peek2_stack_func(stack_index, stack_ptr[stack_index] - 4);
}

uint16_t l2_register_func(uint1_t stack_index) {
	return peek2_stack_func(stack_index, stack_ptr[stack_index] - 6);
}

// INSTRUCTION HANDLING



uint1_t eval_opcode(
	uint1_t stack_index,
	uint8_t opcode,
	uint8_t ins,
	uint8_t k
) {
	static uint8_t 	t8, n8, l8, tmp;
	static uint16_t t16, n16, l16, tmp16;
	static uint1_t eval_opcode_ret_value;
	
	eval_opcode_ret_value = 0;

	switch(opcode) {
		/* IMM */
		case 0x00: /* BRK   */ 
			return 1;
		case 0xff: /* JCI   */ 
			stack_ptr_move(stack_index, 1, 1);
			tmp = stack_data[stack_index][stack_ptr[stack_index]];
			tmp16 = (tmp == 0) ? 0 : peek2_ram_func(pc) + 2;
			pc_adjust(tmp16, 0);
			break;
			/* pc += !!s->dat[--s->ptr] * PEEK2(ram + pc) + 2; break; */
		case 0xfe: /* JMI   */ 
			pc_adjust(peek2_ram_func(pc) + 2, 0);
			break;
		case 0xfd: /* JSI   */ 
			tmp = push2_stack_func(1, ins, pc + 2);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			pc_adjust(peek2_ram_func(pc) + 2, 0);
			break;
		case 0xfc: /* LIT   */ 
			tmp = push_stack_func(stack_index, ins, ram[pc]);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			pc_adjust(1, 0);
			break;
		case 0xfb: /* LIT2  */ 
			tmp16 = peek2_ram_func(pc);
			tmp = push2_stack_func(stack_index, ins, tmp16);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			pc_adjust(2, 0);
			break;
			/* PUSH2(s, PEEK2(ram + pc)) pc += 2; break; */
		case 0xfa: /* LITr  */ 
			tmp = push_stack_func(stack_index, ins, ram[pc]);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			pc_adjust(1, 0);
			break;
			/* PUSH(s, ram[pc++]) break; */
		case 0xf9: /* LIT2r */ 
			tmp16 = peek2_ram_func(pc);
			tmp = push2_stack_func(stack_index, ins, tmp16);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			pc_adjust(2, 0);
			break;
			/* PUSH2(s, PEEK2(ram + pc)) pc += 2; break; */
		/* ALU */
		case 0x01: /* INC  */ 
			t8 = t_register_func(stack_index);
			tmp = set_func(stack_index, ins, k, 1, 0);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			put_stack_func(stack_index, 0, t8 + 1);
			break;
			/* t=T; SET(1, 0) PUT(0, t + 1) break; */
		case 0x21: 
			t16 = t2_register_func(stack_index);
			tmp = set_func(stack_index, ins, k, 2, 0);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			put2_stack_func(stack_index, 0, t16 + 1);
			break;         
			/* t=T2; SET(2, 0) PUT2(0, t + 1) break; */
		case 0x02: /* POP  */  
			tmp = set_func(stack_index, 1, -1);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			break;               
			/* SET(1,-1) break; */
		case 0x22:
			tmp = set_func(stack_index, 2, -2);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			break;                            
			/* SET(2,-2) break; */
		case 0x03: /* NIP  */ 
			t8 = t_register_func(stack_index);
			tmp = set_func(stack_index, ins, k, 2, -1);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			put_stack_func(stack_index, 0, t8);
			break;
			/* t=T; SET(2,-1) PUT(0, t) break; */
		case 0x23:
			t16 = t2_register_func(stack_index);
			tmp = set_func(stack_index, ins, k, 4, -2);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			put2_stack_func(stack_index, 0, t16);
			break;
			/* t=T2;           SET(4,-2) PUT2(0, t) break;*/
		case 0x04: /* SWP  */
			t8 = t_register_func(stack_index);
			n8 = n_register_func(stack_index);
			tmp = set_func(stack_index, ins, k, 2, 0);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			put_stack_func(stack_index, 0, n8);
			put_stack_func(stack_index, 1, t8);
			break;
			/* t=T;n=N;        SET(2, 0) PUT(0, n) PUT(1, t) break; */
		case 0x24: 
			t16 = t2_register_func(stack_index);
			n16 = n2_register_func(stack_index); 
			tmp = set_func(stack_index, ins, k, 4, 0);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			put2_stack_func(stack_index, 0, n16);
			put2_stack_func(stack_index, 2, t16);          
			break; 
			/* t=T2;n=N2;      SET(4, 0) PUT2(0, n) PUT2(2, t) break; */
		case 0x05: /* ROT  */ 
			t8 = t_register_func(stack_index);
			n8 = n_register_func(stack_index);
			l8 = l_register_func(stack_index);
			tmp = set_func(stack_index, ins, k, 3, 0);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			put_stack_func(stack_index, 0, l8);
			put_stack_func(stack_index, 1, t8);
			put_stack_func(stack_index, 2, n8);
			break; 
			/* t=T;n=N;l=L;    SET(3, 0) PUT(0, l) PUT(1, t) PUT(2, n) break; */
		case 0x25:  
			t16 = t2_register_func(stack_index);
			n16 = n2_register_func(stack_index);
			l16 = l2_register_func(stack_index);
			tmp = set_func(stack_index, ins, k, 6, 0);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			put2_stack_func(stack_index, 0, l16);
			put2_stack_func(stack_index, 2, t16);
			put2_stack_func(stack_index, 4, n16);
			break; 
			/* t=T2;n=N2;l=L2; SET(6, 0) PUT2(0, l) PUT2(2, t) PUT2(4, n) break; */
		case 0x06: /* DUP  */ 
			t8 = t_register_func(stack_index);
			tmp = set_func(stack_index, ins, k, 1, 1);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			put_stack_func(stack_index, 0, t8);
			put_stack_func(stack_index, 1, t8);
			break; 
			/* t=T;            SET(1, 1) PUT(0, t) PUT(1, t) break; */
		case 0x26:
			t16 = t2_register_func(stack_index);
			tmp = set_func(stack_index, ins, k, 2, 2);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			put2_stack_func(stack_index, 0, t16);
			put2_stack_func(stack_index, 2, t16);       
			break; 
			/* t=T2;           SET(2, 2) PUT2(0, t) PUT2(2, t) break; */
		case 0x07: /* OVR  */ 
			t8 = t_register_func(stack_index);
			n8 = n_register_func(stack_index);
			tmp = set_func(stack_index, ins, k, 2, 1);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			put_stack_func(stack_index, 0, n8);
			put_stack_func(stack_index, 1, t8);
			put_stack_func(stack_index, 2, n8);
			break; 
			/* t=T;n=N;        SET(2, 1) PUT(0, n) PUT(1, t) PUT(2, n) break; */
		case 0x27:  
			t16 = t2_register_func(stack_index);
			n16 = n2_register_func(stack_index);
			tmp = set_func(stack_index, ins, k, 4, 2);
			if (tmp > 0) { eval_opcode_ret_value = 1; break; } // stack overflow
			put2_stack_func(stack_index, 0, n16);
			put2_stack_func(stack_index, 2, t16);
			put2_stack_func(stack_index, 4, n16);     
			break; 
			/* t=T2;n=N2;      SET(4, 2) PUT2(0, n) PUT2(2, t) PUT2(4, n) break; */
		case 0x08: /* EQU  */ 
			break;
			/* t=T;n=N;        SET(2,-1) PUT(0, n == t) break; */
		case 0x28:            
			break; 
			/* t=T2;n=N2;      SET(4,-3) PUT(0, n == t) break; */
		case 0x09: /* NEQ  */ 
			break;
			/* t=T;n=N;        SET(2,-1) PUT(0, n != t) break; */
		case 0x29:            
			break;
			/* t=T2;n=N2;      SET(4,-3) PUT(0, n != t) break; */
		case 0x0a: /* GTH  */ 
			break;
			/* t=T;n=N;        SET(2,-1) PUT(0, n > t) break; */
		case 0x2a:            
			break;
			/* t=T2;n=N2;      SET(4,-3) PUT(0, n > t) break; */
		case 0x0b: /* LTH  */ 
			break;
			/* t=T;n=N;        SET(2,-1) PUT(0, n < t) break; */
		case 0x2b:            
			break;
			/* t=T2;n=N2;      SET(4,-3) PUT(0, n < t) break; */
		case 0x0c: /* JMP  */ 
			break;
			/* t=T;            SET(1,-1) pc += (Sint8)t; break; */
		case 0x2c:            
			break;
			/* t=T2;           SET(2,-2) pc = t; break; */
		case 0x0d: /* JCN  */ 
			break;
			/* t=T;n=N;        SET(2,-2) pc += !!n * (Sint8)t; break; */
		case 0x2d:            
			break;
			/* t=T2;n=L;       SET(3,-3) if(n) pc = t; break; */
		case 0x0e: /* JSR  */ 
			break;
			/* t=T;            SET(1,-1) PUSH2(&u->rst, pc) pc += (Sint8)t; break; */
		case 0x2e:            
			break;
			/* t=T2;           SET(2,-2) PUSH2(&u->rst, pc) pc = t; break; */
		case 0x0f: /* STH  */ 
			break;
			/* t=T;            SET(1,-1) PUSH((ins & 0x40 ? &u->wst : &u->rst), t) break; */
		case 0x2f:            
			break;
			/* t=T2;           SET(2,-2) PUSH2((ins & 0x40 ? &u->wst : &u->rst), t) break; */
		case 0x10: /* LDZ  */ 
			break;
			/* t=T;            SET(1, 0) PUT(0, ram[t]) break; */
		case 0x30:            
			break;
			/* t=T;            SET(1, 1) PUT2(0, PEEK2(ram + t)) break; */
		case 0x11: /* STZ  */ 
			break;
			/* t=T;n=N;        SET(2,-2) ram[t] = n; break; */
		case 0x31:            
			break;
			/* t=T;n=H2;       SET(3,-3) POKE2(ram + t, n) break; */
		case 0x12: /* LDR  */ 
			break;
			/* t=T;            SET(1, 0) PUT(0, ram[pc + (Sint8)t]) break; */
		case 0x32:            
			break;
			/* t=T;            SET(1, 1) PUT2(0, PEEK2(ram + pc + (Sint8)t)) break; */
		case 0x13: /* STR  */ 
			break;
			/* t=T;n=N;        SET(2,-2) ram[pc + (Sint8)t] = n; break; */
		case 0x33:            
			break;
			/* t=T;n=H2;       SET(3,-3) POKE2(ram + pc + (Sint8)t, n) break; */
		case 0x14: /* LDA  */ 
			break;
			/* t=T2;           SET(2,-1) PUT(0, ram[t]) break; */
		case 0x34:            
			break;
			/* t=T2;           SET(2, 0) PUT2(0, PEEK2(ram + t)) break; */
		case 0x15: /* STA  */ 
			break;
			/* t=T2;n=L;       SET(3,-3) ram[t] = n; break; */
		case 0x35:            
			break;
			/* t=T2;n=N2;      SET(4,-4) POKE2(ram + t, n) break; */
		case 0x16: /* DEI  */ 
			break;
			/* t=T;            SET(1, 0) DEI(0, t) break; */
		case 0x36:            
			break;
			/* t=T;            SET(1, 1) DEI(1, t) DEI(0, t + 1) break; */
		case 0x17: /* DEO  */ 
			break;
			/* t=T;n=N;        SET(2,-2) DEO(t, n) break; */
		case 0x37:            
			break;
			/* t=T;n=N;l=L;    SET(3,-3) DEO(t, l) DEO(t + 1, n) break; */
		case 0x18: /* ADD  */ 
			break;
			/* t=T;n=N;        SET(2,-1) PUT(0, n + t) break; */
		case 0x38:            
			break;
			/* t=T2;n=N2;      SET(4,-2) PUT2(0, n + t) break; */
		case 0x19: /* SUB  */ 
			break;
			/* t=T;n=N;        SET(2,-1) PUT(0, n - t) break; */
		case 0x39:            
			break;
			/* t=T2;n=N2;      SET(4,-2) PUT2(0, n - t) break; */
		case 0x1a: /* MUL  */ 
			break;
			/* t=T;n=N;        SET(2,-1) PUT(0, n * t) break; */
		case 0x3a:            
			break;
			/* t=T2;n=N2;      SET(4,-2) PUT2(0, n * t) break; */
		case 0x1b: /* DIV  */ 
			break;
			/* t=T;n=N;        SET(2,-1) if(!t) HALT(3) PUT(0, n / t) break; */
		case 0x3b:            
			break;
			/* t=T2;n=N2;      SET(4,-2) if(!t) HALT(3) PUT2(0, n / t) break; */
		case 0x1c: /* AND  */ 
			break;
			/* t=T;n=N;        SET(2,-1) PUT(0, n & t) break; */
		case 0x3c:            
			break; 
			/* t=T2;n=N2;      SET(4,-2) PUT2(0, n & t) break; */
		case 0x1d: /* ORA  */ 
			break;
			/* t=T;n=N;        SET(2,-1) PUT(0, n | t) break; */
		case 0x3d:            
			break;
			/* t=T2;n=N2;      SET(4,-2) PUT2(0, n | t) break; */
		case 0x1e: /* EOR  */ 
			break;
			/* t=T;n=N;        SET(2,-1) PUT(0, n ^ t) break; */
		case 0x3e:            
			break;
			/* t=T2;n=N2;      SET(4,-2) PUT2(0, n ^ t) break; */
		case 0x1f: /* SFT  */ 
			break;
			/* t=T;n=N;        SET(2,-1) PUT(0, n >> (t & 0xf) << (t >> 4)) break; */
		case 0x3f:            
			break;
			/* t=T;n=H2;       SET(3,-1) PUT2(0, n >> (t & 0xf) << (t >> 4)) break; */
		default: break;
	}
	
	return eval_opcode_ret_value;
}

#pragma MAIN_MHZ uxn_eval 1.0
uint1_t uxn_eval() {

	static uint8_t k, opc, ins;
	static uint1_t s, uxn_eval_should_return, uxn_eval_ret_value;

	uxn_eval_ret_value = 0;
	uxn_eval_should_return = 0;

	if(pc == 0) {
		uxn_eval_should_return = 1;
	}
	
	if (dev[15] != 0) {
		uxn_eval_should_return = 1;
	}
	
	if (!uxn_eval_should_return) {
		ins = ram[pc] & 0xff;
		pc_adjust(1, 0);
		k = ins & 0x80 ? 0xff : 0;
		s = ins & 0x40 ? 1 : 0;
		opc = !(ins & 0x1f) ? (0 - (ins >> 5)) & 0xff : ins & 0x3f;	
		uxn_eval_should_return = eval_opcode(s, opc, ins, k);
	}
	
	return uxn_eval_ret_value;
}