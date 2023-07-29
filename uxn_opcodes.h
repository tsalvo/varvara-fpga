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

uint1_t eval_opcode(
	uint1_t stack_index,
	uint8_t opcode,
	uint8_t ins,
	uint8_t k
) {
	static uint8_t 	t8, n8, l8, tmp8;
	static uint16_t t16, n16, l16, tmp16;
	static uint1_t result;
	
	result = 0;
	
	if (opcode == 0x00      /* BRK */) {
		result = 1;
	}
	else if (opcode == 0xFF /* JCI */) {
		stack_pointer_move(stack_index, 1, 1);
		tmp8 = stack_data_get(stack_index, stack_pointer_get(stack_index));
		tmp16 = (tmp8 == 0) ? 0 : peek2_ram(pc_get() + 2);
		pc_add(tmp16);
	}
	else if (opcode == 0xFE /* JMI */) {    
		pc_add(peek2_ram(pc_get()) + 2);
	}
	else if (opcode == 0xFD /* JSI */) {
		tmp8 = push2_stack(1, ins, pc_get() + 2);
		if (tmp8 > 0) { result = 1; }
		else { pc_add(peek2_ram(pc_get()) + 2); }
	}
	else if (opcode == 0xFC /* LIT */) {
		tmp8 = push_stack(stack_index, ins, main_ram_read(pc_get()));
		if (tmp8 > 0) { result = 1; }
		else { pc_add(1); }
	}
	else if (opcode == 0xFB /* LIT2 */) {
		tmp16 = peek2_ram(pc_get());
		tmp8 = push2_stack(stack_index, ins, tmp16);
		if (tmp8 > 0) { result = 1; }
		else { pc_add(2); }
	}
	else if (opcode == 0xFA /* LITr */) {
		tmp8 = push_stack(stack_index, ins, main_ram_read(pc_get()));
		if (tmp8 > 0) { result = 1; }
		else { pc_add(1); }
	}
	else if (opcode == 0xF9 /* LIT2r */) {
		tmp16 = peek2_ram(pc_get());
		tmp8 = push2_stack(stack_index, ins, tmp16);
		if (tmp8 > 0) { result = 1; }
		else { pc_add(2); }
	}
	else if (opcode == 0x01 /* INC */) {
		t8 = t_register(stack_index);
		tmp8 = set(stack_index, ins, k, 1, 0);
		if (tmp8 > 0) { result = 1; }
		else { put_stack(stack_index, 0, t8 + 1); }
	}
	else if (opcode == 0x21 /*  */) {
		t16 = t2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, 0);
		if (tmp8 > 0) { result = 1; }
		else { put2_stack(stack_index, 0, t16 + 1); }
	}
	else if (opcode == 0x02 /* POP */) {
		tmp8 = set(stack_index, ins, k, 1, -1);
		if (tmp8 > 0) { result = 1; }
	}
	else if (opcode == 0x22 /*  */) {
		tmp8 = set(stack_index, ins, k, 2, -2);
		if (tmp8 > 0) { result = 1; }
	}
	else if (opcode == 0x03 /* NIP */) {
		t8 = t_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else { put_stack(stack_index, 0, t8); }
	}
	else if (opcode == 0x23 /*  */) {
		t16 = t2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, -2);
		if (tmp8 > 0) { result = 1; }
		else { put2_stack(stack_index, 0, t16); }
	}
	else if (opcode == 0x04 /* SWP */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, 0);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, n8);
			put_stack(stack_index, 1, t8);
		} // stack overflow
	}
	else if (opcode == 0x24 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index); 
		tmp8 = set(stack_index, ins, k, 4, 0);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, n16);
			put2_stack(stack_index, 2, t16);  
		}
	}
	else if (opcode == 0x05 /* ROT */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		l8 = l_register(stack_index);
		tmp8 = set(stack_index, ins, k, 3, 0);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, l8);
			put_stack(stack_index, 1, t8);
			put_stack(stack_index, 2, n8);
		}
	}
	else if (opcode == 0x25 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		l16 = l2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 6, 0);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, l16);
			put2_stack(stack_index, 2, t16);
			put2_stack(stack_index, 4, n16);
		}
	}
	else if (opcode == 0x06 /* DUP */) {
		t8 = t_register(stack_index);
		tmp8 = set(stack_index, ins, k, 1, 1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, t8);
			put_stack(stack_index, 1, t8);
		}
	}
	else if (opcode == 0x26 /*  */) {
		t16 = t2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, 2);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, t16);
			put2_stack(stack_index, 2, t16);    
		}
	}
	else if (opcode == 0x07 /* OVR */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, 1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, n8);
			put_stack(stack_index, 1, t8);
			put_stack(stack_index, 2, n8);
		}
	}
	else if (opcode == 0x27 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, 2);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, n16);
			put2_stack(stack_index, 2, t16);
			put2_stack(stack_index, 4, n16);  
		}
	}
	else if (opcode == 0x08 /* EQU */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, n8 == t8 ? 1 : 0);
		}
	}
	else if (opcode == 0x28 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, -3);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, n16 == t16 ? 1 : 0);
		}
	}
	else if (opcode == 0x09 /* NEQ */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, n8 == t8 ? 0 : 1);
		}
	}
	else if (opcode == 0x29 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, -3);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, n16 == t16 ? 0 : 1);
		}
	}
	else if (opcode == 0x0A /* GTH */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, n8 > t8 ? 1 : 0);
		}
	}
	else if (opcode == 0x2A /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, -3);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, n16 > t16 ? 1 : 0);
		}
	}
	else if (opcode == 0x0B /* LTH */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, n8 < t8 ? 1 : 0);
		}
	}
	else if (opcode == 0x2B /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, -3);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, n16 < t16 ? 1 : 0);
		}
	}
	else if (opcode == 0x0C /* JMP */) {
		t8 = t_register(stack_index);
		tmp8 = set(stack_index, ins, k, 1, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			pc_add_s8((int8_t)(t8));
		}
	}
	else if (opcode == 0x2C /*  */) {
		t16 = t2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -2);
		if (tmp8 > 0) { result = 1; }
		else {
			pc_set(t16);
		}
	}
	else if (opcode == 0x0D /* JCN */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -2);
		if (tmp8 > 0) { result = 1; }
		else if (n8 > 0) {
			pc_add_s8((int8_t)(t8));
		}
	}
	else if (opcode == 0x2D /*  */) {
		t16 = t2_register(stack_index);
		n8 = l_register(stack_index);
		tmp8 = set(stack_index, ins, k, 3, -3);
		if (tmp8 > 0) { result = 1; }
		else if (n8 > 0) {
			pc_set(t16);
		}
	}
	else if (opcode == 0x0E /* JSR */) {
		t8 = t_register(stack_index);
		tmp8 = set(stack_index, ins, k, 1, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			tmp8 = push2_stack(1, ins, pc_get());
			if (tmp8 > 0) { result = 1; }
			else {
				pc_add_s8((int8_t)(t8));
			}
		}
	}
	else if (opcode == 0x2E /*  */) {
		t16 = t2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -2);
		if (tmp8 > 0) { result = 1; }
		else {
			tmp8 = push2_stack(1, ins, pc_get());
			if (tmp8 > 0) { result = 1; }
			else {
				pc_set(t16);
			}
		}
	}
	else if (opcode == 0x0F /* STH */) {
		t8 = t_register(stack_index);
		tmp8 = set(stack_index, ins, k, 1, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			tmp8 = push_stack(ins & 0x40 ? 0 : 1, ins, t8);
			if (tmp8 > 0) { result = 1; }
		}
	}
	else if (opcode == 0x2F /*  */) {
		t16 = t2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -2);
		if (tmp8 > 0) { result = 1; }
		else {
			tmp8 = push2_stack(ins & 0x40 ? 0 : 1, ins, t16);
			if (tmp8 > 0) { result = 1; }
		}
	}
	else if (opcode == 0x10 /* LDZ */) {
		t8 = t_register(stack_index);
		t16 = (uint16_t)(t8);
		tmp8 = set(stack_index, ins, k, 1, 0);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, main_ram_read(t16));
		}
	}
	else if (opcode == 0x30 /*  */) {
		t8 = t_register(stack_index);
		t16 = (uint16_t)(t8);
		tmp8 = set(stack_index, ins, k, 1, 1);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, peek2_ram(t16));
		}
	}
	else if (opcode == 0x11 /* STZ */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		t16 = (uint16_t)(t8);
		tmp8 = set(stack_index, ins, k, 2, -2);
		if (tmp8 > 0) { result = 1; }
		else {
			main_ram_write(t16, n8);
		}
	}
	else if (opcode == 0x31 /*  */) {
		t8 = t_register(stack_index);
		t16 = (uint16_t)(t8);
		n16 = h2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 3, -3);
		if (tmp8 > 0) { result = 1; }
		else {
			poke2_ram(t16, n16);
		}
	}
	else if (opcode == 0x12 /* LDR */) {
		t8 = t_register(stack_index);
		tmp8 = set(stack_index, ins, k, 1, 0);
		if (tmp8 > 0) { result = 1; }
		else {
			t16 = pc_get() + ((int8_t)(t8));
			put_stack(stack_index, 0, main_ram_read(t16));
		}
	}
	else if (opcode == 0x32 /*  */) {
		t8 = t_register(stack_index);
		tmp8 = set(stack_index, ins, k, 1, 1);
		if (tmp8 > 0) { result = 1; }
		else {
			t16 = pc_get() + ((int8_t)(t8));
			put2_stack(stack_index, 0, peek2_ram(t16));
		}
	}
	else if (opcode == 0x13 /* STR */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -2);
		if (tmp8 > 0) { result = 1; }
		else {
			t16 = pc_get() + ((int8_t)(t8));
			main_ram_write(t16, n8);
		}
	}
	else if (opcode == 0x33 /*  */) {
		t8 = t_register(stack_index);
		n16 = h2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 3, -3);
		if (tmp8 > 0) { result = 1; }
		else {
			t16 = pc_get() + ((int8_t)(t8));
			poke2_ram(t16, n16);
		}
	}
	else if (opcode == 0x14 /* LDA */) {
		t16 = t2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, main_ram_read(t16));
		}
	}
	else if (opcode == 0x34 /*  */) {
		t16 = t2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, 0);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 9, peek2_ram(t16));
		}
	}
	else if (opcode == 0x15 /* STA */) {
		t16 = t2_register(stack_index);
		n8 = l_register(stack_index);
		tmp8 = set(stack_index, ins, k, 3, -3);
		if (tmp8 > 0) { result = 1; }
		else {
			main_ram_write(t16, n8);
		}
	}
	else if (opcode == 0x35 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, -4);
		if (tmp8 > 0) { result = 1; }
		else {
			poke2_ram(t16, n16);
		}
	}
	else if (opcode == 0x16 /* DEI */) {
		t8 = t_register(stack_index);
		tmp8 = set(stack_index, ins, k, 1, 0);
		if (tmp8 > 0) { result = 1; }
		else {
			dei(stack_index, 0, t8);
		}
	}
	else if (opcode == 0x36 /*  */) {
		t8 = t_register(stack_index);
		tmp8 = set(stack_index, ins, k, 1, 1);
		if (tmp8 > 0) { result = 1; }
		else {
			dei(stack_index, 1, t8);
			dei(stack_index, 0, t8 + 1);
		}
	}
	else if (opcode == 0x17 /* DEO */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -2);
		if (tmp8 > 0) { result = 1; }
		else {
			deo(t8, n8);
		}
	}
	else if (opcode == 0x37 /*  */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		l8 = l_register(stack_index);
		tmp8 = set(stack_index, ins, k, 3, -3);
		if (tmp8 > 0) { result = 1; }
		else {
			deo(t8, l8);
			deo(t8 + 1, n8);
		}
	}
	else if (opcode == 0x18 /* ADD */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, n8 + t8);
		}
	}
	else if (opcode == 0x38 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, -2);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, n16 + t16);
		}
	}
	else if (opcode == 0x19 /* SUB */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, n8 - t8);
		}
	}
	else if (opcode == 0x39 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, -2);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, n16 - t16);
		}
	}
	else if (opcode == 0x1A /* MUL */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, n8 * t8);
		}
	}
	else if (opcode == 0x3A /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, -2);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, n16 * t16);
		}
	}
	else if (opcode == 0x1B /* DIV */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else if (t8 == 0) {
			tmp8 = halt(ins, 3);
			if (tmp8 > 0) { result = 1; }
		} else {
			put_stack(stack_index, 0, n8 / t8);
		}
	}
	else if (opcode == 0x3B /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, -2);
		if (tmp8 > 0) { result = 1; }
		else if (t16 == 0) {
			tmp8 = halt(ins, 3);
			if (tmp8 > 0) { result = 1; }
		} else {
			put2_stack(stack_index, 0, n16 / t16);
		}
	}
	else if (opcode == 0x1C /* AND */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, n8 & t8);
		}
	}
	else if (opcode == 0x3C /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, -2);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, n16 & t16);
		}
	}
	else if (opcode == 0x1D /* ORA */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, n8 | t8);
		}
	}
	else if (opcode == 0x3D /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, -2);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, n16 | t16);
		}
	}
	else if (opcode == 0x1E /* EOR */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, n8 ^ t8);
		}
	}
	else if (opcode == 0x3E /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 4, -2);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, n16 ^ t16);
		}
	}
	else if (opcode == 0x1F /* SFT */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		tmp8 = set(stack_index, ins, k, 2, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			put_stack(stack_index, 0, (n8 >> (t8 & 0x0F)) << (t8 >> 4));
		}
	}
	else if (opcode == 0x3F /*  */) {
		t8 = t_register(stack_index);
		n16 = h2_register(stack_index);
		tmp8 = set(stack_index, ins, k, 3, -1);
		if (tmp8 > 0) { result = 1; }
		else {
			put2_stack(stack_index, 0, (n16 >> (t8 & 0x0F)) << (t8 >> 4));
		}
	}
	
	return result;
}