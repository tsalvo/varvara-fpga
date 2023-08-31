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

uint1_t opc_brk() {
	return 1;
}

uint1_t opc_jci(uint1_t stack_index) {
	static uint8_t tmp8;
	static uint16_t tmp16;
	stack_pointer_move_old(stack_index, 1, 1);
	tmp8 = stack_data_get(stack_index, stack_pointer_get(stack_index));
	tmp16 = (tmp8 == 0) ? 0 : peek2_ram(pc_get() + 2);
	pc_add(tmp16);
	return 0;
}

uint1_t opc_jmi() {
	pc_add(peek2_ram(pc_get()) + 2);
	return 0;
}

uint1_t opc_jsi(uint8_t ins) {
	static uint8_t tmp8;
	static uint1_t result;
	tmp8 = push2_stack_old(1, ins, pc_get() + 2);
	if (tmp8 > 0) { result = 1; }
	else { pc_add(peek2_ram(pc_get()) + 2); result = 0; }
	return result;
}

uint1_t opc_lit(uint1_t stack_index, uint8_t ins) {
	static uint8_t tmp8;
	static uint1_t result;
	tmp8 = push_stack_old(stack_index, ins, peek_ram(pc_get()));
	if (tmp8 > 0) { result = 1; }
	else { pc_add(1); result = 0; }
	return result;
}

uint1_t opc_lit2(uint1_t stack_index, uint8_t ins) {
	static uint8_t tmp8;
	static uint16_t tmp16;
	static uint1_t result;
	tmp16 = peek2_ram(pc_get());
	tmp8 = push2_stack_old(stack_index, ins, tmp16);
	if (tmp8 > 0) { result = 1; }
	else { pc_add(2); result = 0; }
	return result;
}

uint1_t opc_litr(uint1_t stack_index, uint8_t ins) {
	static uint8_t tmp8;
	static uint1_t result;
	tmp8 = push_stack_old(stack_index, ins, peek_ram(pc_get()));
	if (tmp8 > 0) { result = 1; }
	else { pc_add(1); result = 0; }
	return result;
}

uint1_t opc_lit2r(uint1_t stack_index, uint8_t ins) {
	static uint8_t tmp8;
	static uint16_t tmp16;
	static uint1_t result;
	tmp16 = peek2_ram(pc_get());
	tmp8 = push2_stack_old(stack_index, ins, tmp16);
	if (tmp8 > 0) { result = 1; }
	else { pc_add(2); result = 0; }
	return result;
}

uint1_t opc_inc(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 1, 0);
	if (tmp8 > 0) { result = 1; }
	else { put_stack_old(stack_index, 0, t8 + 1); result = 0; }
	return result;
}

uint1_t opc_inc2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, 0);
	if (tmp8 > 0) { result = 1; }
	else { put2_stack(stack_index, 0, t16 + 1); result = 0; }
	return result;
}

uint1_t opc_pop(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t tmp8;
	static uint1_t result;
	tmp8 = set_old(stack_index, ins, k, 1, -1);
	if (tmp8 > 0) { result = 1; }
	else { result = 0; }
	return result;
}

uint1_t opc_pop2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t tmp8;
	static uint1_t result;
	tmp8 = set_old(stack_index, ins, k, 2, -2);
	if (tmp8 > 0) { result = 1; }
	else { result = 0; }
	return result;
}

uint1_t opc_nip(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else { put_stack_old(stack_index, 0, t8); result = 0; }
	return result;
}

uint1_t opc_nip2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, -2);
	if (tmp8 > 0) { result = 1; }
	else { put2_stack(stack_index, 0, t16); result = 0; }
	return result;
}

uint1_t opc_swp(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, 0);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, n8);
		put_stack_old(stack_index, 1, t8);
		result = 0;
	}
	return result;
}

uint1_t opc_swp2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index); 
	tmp8 = set_old(stack_index, ins, k, 4, 0);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, n16);
		put2_stack(stack_index, 2, t16); 
		result = 0; 
	}
	return result;
}

uint1_t opc_rot(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t l8, n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	l8 = l_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 3, 0);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, l8);
		put_stack_old(stack_index, 1, t8);
		put_stack_old(stack_index, 2, n8);
		result = 0; 
	}
	return result;
}

uint1_t opc_rot2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t l16, n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	l16 = l2_register(stack_index);
	tmp8 = set_old(stack_index, ins, k, 6, 0);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, l16);
		put2_stack(stack_index, 2, t16);
		put2_stack(stack_index, 4, n16);
		result = 0; 
	}
	return result;
}

uint1_t opc_dup(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 1, 1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, t8);
		put_stack_old(stack_index, 1, t8);
		result = 0;
	}
	return result;
}

uint1_t opc_dup2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, 2);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, t16);
		put2_stack(stack_index, 2, t16);
		result = 0;
	}
	return result;
}

uint1_t opc_ovr(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, 1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, n8);
		put_stack_old(stack_index, 1, t8);
		put_stack_old(stack_index, 2, n8);
		result = 0;
	}
	return result;
}

uint1_t opc_ovr2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, 2);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, n16);
		put2_stack(stack_index, 2, t16);
		put2_stack(stack_index, 4, n16);  
		result = 0;
	}
	return result;
}

uint1_t opc_equ(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, n8 == t8 ? 1 : 0);
		result = 0;
	}
	return result;
}

uint1_t opc_equ2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, -3);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, n16 == t16 ? 1 : 0);
		result = 0;
	}
	return result;
}

uint1_t opc_neq(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, n8 == t8 ? 0 : 1);
		result = 0;
	}
	return result;
}

uint1_t opc_neq2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, -3);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, n16 == t16 ? 0 : 1);
		result = 0;
	}
	return result;
}

uint1_t opc_gth(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, n8 > t8 ? 1 : 0);
		result = 0;
	}
	return result;
}

uint1_t opc_gth2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, -3);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, n16 > t16 ? 1 : 0);
		result = 0;
	}
	return result;
}

uint1_t opc_lth(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, n8 < t8 ? 1 : 0);
		result = 0;
	}
	return result;
}

uint1_t opc_lth2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, -3);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, n16 < t16 ? 1 : 0);
		result = 0;
	}
	return result;
}

uint1_t opc_jmp(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 1, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		pc_add_s8_old((int8_t)(t8));
		result = 0;
	}
	return result;
}

uint1_t opc_jmp2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -2);
	if (tmp8 > 0) { result = 1; }
	else {
		pc_set(t16);
		result = 0;
	}
	return result;
}

uint1_t opc_jcn(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -2);
	if (tmp8 > 0) { result = 1; }
	else if (n8 > 0) {
		pc_add_s8_old((int8_t)(t8));
		result = 0;
	}
	return result;
}

uint1_t opc_jcn2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t n8, tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n8 = l_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 3, -3);
	if (tmp8 > 0) { result = 1; }
	else if (n8 > 0) {
		pc_set(t16);
		result = 0;
	}
	return result;
}

uint1_t opc_jsr(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 1, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		tmp8 = push2_stack_old(1, ins, pc_get());
		if (tmp8 > 0) { result = 1; }
		else {
			pc_add_s8_old((int8_t)(t8));
			result = 0;
		}
	}
	return result;
}

uint1_t opc_jsr2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -2);
	if (tmp8 > 0) { result = 1; }
	else {
		tmp8 = push2_stack_old(1, ins, pc_get());
		if (tmp8 > 0) { result = 1; }
		else {
			pc_set(t16);
			result = 0;
		}
	}
	return result;
}

uint1_t opc_sth(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 1, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		tmp8 = push_stack_old(ins & 0x40 ? 0 : 1, ins, t8);
		if (tmp8 > 0) { result = 1; }
		else { result = 0; }
	}
	return result;
}

uint1_t opc_sth2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -2);
	if (tmp8 > 0) { result = 1; }
	else {
		tmp8 = push2_stack_old(ins & 0x40 ? 0 : 1, ins, t16);
		if (tmp8 > 0) { result = 1; }
		else { result = 0; }
	}
	return result;
}

uint1_t opc_ldz(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	t16 = (uint16_t)(t8);
	tmp8 = set_old(stack_index, ins, k, 1, 0);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, peek_ram(t16));
		result = 0;
	}
	return result;
}

uint1_t opc_ldz2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	t16 = (uint16_t)(t8);
	tmp8 = set_old(stack_index, ins, k, 1, 1);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, peek2_ram(t16));
		result = 0;
	}
	return result;
}

uint1_t opc_stz(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	t16 = (uint16_t)(t8);
	tmp8 = set_old(stack_index, ins, k, 2, -2);
	if (tmp8 > 0) { result = 1; }
	else {
		poke_ram(t16, n8);
		result = 0;
	}
	return result;
}

uint1_t opc_stz2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	t16 = (uint16_t)(t8);
	n16 = h2_register(stack_index);
	tmp8 = set_old(stack_index, ins, k, 3, -3);
	if (tmp8 > 0) { result = 1; }
	else {
		poke2_ram(t16, n16);
		result = 0;
	}
	return result;
}

uint1_t opc_ldr(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 1, 0);
	if (tmp8 > 0) { result = 1; }
	else {
		t16 = pc_get() + ((int8_t)(t8));
		put_stack_old(stack_index, 0, peek_ram(t16));
		result = 0;
	}
	return result;
}

uint1_t opc_ldr2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 1, 1);
	if (tmp8 > 0) { result = 1; }
	else {
		t16 = pc_get() + ((int8_t)(t8));
		put2_stack(stack_index, 0, peek2_ram(t16));
		result = 0;
	}
	return result;
}

uint1_t opc_str(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -2);
	if (tmp8 > 0) { result = 1; }
	else {
		t16 = pc_get() + ((int8_t)(t8));
		poke_ram(t16, n8);
		result = 0;
	}
	return result;
}

uint1_t opc_str2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n16 = h2_register(stack_index);
	tmp8 = set_old(stack_index, ins, k, 3, -3);
	if (tmp8 > 0) { result = 1; }
	else {
		t16 = pc_get() + ((int8_t)(t8));
		poke2_ram(t16, n16);
		result = 0;
	}
	return result;
}

uint1_t opc_lda(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, peek_ram(t16));
		result = 0;
	}
	return result;
}

uint1_t opc_lda2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, 0);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 9, peek2_ram(t16));
		result = 0;
	}
	return result;
}

uint1_t opc_sta(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t n8, tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n8 = l_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 3, -3);
	if (tmp8 > 0) { result = 1; }
	else {
		poke_ram(t16, n8);
		result = 0;
	}
	return result;
}

uint1_t opc_sta2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, -4);
	if (tmp8 > 0) { result = 1; }
	else {
		poke2_ram(t16, n16);
		result = 0;
	}
	return result;
}

uint1_t opc_dei(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 1, 0);
	if (tmp8 > 0) { result = 1; }
	else {
		dei(stack_index, 0, t8);
		result = 0;
	}
	return result;
}

uint1_t opc_dei2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 1, 1);
	if (tmp8 > 0) { result = 1; }
	else {
		dei(stack_index, 1, t8);
		dei(stack_index, 0, t8 + 1);
		result = 0;
	}
	return result;
}

uint1_t opc_deo(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -2);
	if (tmp8 > 0) { result = 1; }
	else {
		deo(t8, n8);
		result = 0;
	}
	return result;
}

uint1_t opc_deo2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t l8, n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	l8 = l_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 3, -3);
	if (tmp8 > 0) { result = 1; }
	else {
		deo(t8, l8);
		deo(t8 + 1, n8);
		result = 0;
	}
	return result;
}

uint1_t opc_add(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, n8 + t8);
		result = 0;
	}
	return result;
}

uint1_t opc_add2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, -2);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, n16 + t16);
		result = 0;
	}
	return result;
}

uint1_t opc_sub(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, n8 - t8);
		result = 0;
	}
	return result;
}

uint1_t opc_sub2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, -2);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, n16 - t16);
		result = 0;
	}
	return result;
}

uint1_t opc_mul(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, n8 * t8);
		result = 0;
	}
	return result;
}

uint1_t opc_mul2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, -2);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, n16 * t16);
		result = 0;
	}
	return result;
}

uint1_t opc_div(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else if (t8 == 0) {
		tmp8 = halt(ins, 3);
		if (tmp8 > 0) { result = 1; }
		else { result = 0; }
	} else {
		put_stack_old(stack_index, 0, n8 / t8);
		result = 0;
	}
	return result;
}

uint1_t opc_div2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, -2);
	if (tmp8 > 0) { result = 1; }
	else if (t16 == 0) {
		tmp8 = halt(ins, 3);
		if (tmp8 > 0) { result = 1; }
		else { result = 0; }
	} else {
		put2_stack(stack_index, 0, n16 / t16);
		result = 0;
	}
	return result;
}

uint1_t opc_and(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, n8 & t8);
		result = 0;
	}
	return result;
}

uint1_t opc_and2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, -2);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, n16 & t16);
		result = 0;
	}
	return result;
}

uint1_t opc_ora(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, n8 | t8);
		result = 0;
	}
	return result;
}

uint1_t opc_ora2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, -2);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, n16 | t16);
		result = 0;
	}
	return result;
}

uint1_t opc_eor(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, n8 ^ t8);
		result = 0;
	}
	return result;
}

uint1_t opc_eor2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	t16 = t2_register_old(stack_index);
	n16 = n2_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 4, -2);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, n16 ^ t16);
		result = 0;
	}
	return result;
}

uint1_t opc_sft(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n8 = n_register_old(stack_index);
	tmp8 = set_old(stack_index, ins, k, 2, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		put_stack_old(stack_index, 0, (n8 >> (t8 & 0x0F)) << (t8 >> 4));
		result = 0;
	}
	return result;
}

uint1_t opc_sft2(uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16;
	static uint8_t t8, tmp8;
	static uint1_t result;
	t8 = t_register_old(stack_index);
	n16 = h2_register(stack_index);
	tmp8 = set_old(stack_index, ins, k, 3, -1);
	if (tmp8 > 0) { result = 1; }
	else {
		put2_stack(stack_index, 0, (n16 >> (t8 & 0x0F)) << (t8 >> 4));
		result = 0;
	}
	return result;
}

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
	
	if      (opcode == 0x00 /* BRK   */) { result = opc_brk(); }
	else if (opcode == 0xFF /* JCI   */) { result = opc_jci(stack_index); }
	else if (opcode == 0xFE /* JMI   */) { result = opc_jmi(); }
	else if (opcode == 0xFD /* JSI   */) { result = opc_jsi(ins); }
	else if (opcode == 0xFC /* LIT   */) { result = opc_lit(stack_index, ins); }
	else if (opcode == 0xFB /* LIT2  */) { result = opc_lit2(stack_index, ins); }
	else if (opcode == 0xFA /* LITr  */) { result = opc_litr(stack_index, ins); }
	else if (opcode == 0xF9 /* LIT2r */) { result = opc_lit2r(stack_index, ins); }
	else if (opcode == 0x01 /* INC   */) { result = opc_inc(stack_index, ins, k); }
	else if (opcode == 0x21 /* INC2  */) { result = opc_inc2(stack_index, ins, k); }
	else if (opcode == 0x02 /* POP   */) { result = opc_pop(stack_index, ins, k); }
	else if (opcode == 0x22 /* POP2  */) { result = opc_pop2(stack_index, ins, k); }
	else if (opcode == 0x03 /* NIP   */) { result = opc_nip(stack_index, ins, k); }
	else if (opcode == 0x23 /* NIP2  */) { result = opc_nip2(stack_index, ins, k); }
	else if (opcode == 0x04 /* SWP   */) { result = opc_swp(stack_index, ins, k); }
	else if (opcode == 0x24 /* SWP2  */) { result = opc_swp2(stack_index, ins, k); }
	else if (opcode == 0x05 /* ROT   */) { result = opc_rot(stack_index, ins, k); }
	else if (opcode == 0x25 /* ROT2  */) { result = opc_rot2(stack_index, ins, k); }
	else if (opcode == 0x06 /* DUP   */) { result = opc_dup(stack_index, ins, k); }
	else if (opcode == 0x26 /* DUP2  */) { result = opc_dup2(stack_index, ins, k); }
	else if (opcode == 0x07 /* OVR   */) { result = opc_ovr(stack_index, ins, k); }
	else if (opcode == 0x27 /* OVR2  */) { result = opc_ovr2(stack_index, ins, k); }
	else if (opcode == 0x08 /* EQU   */) { result = opc_equ(stack_index, ins, k); }
	else if (opcode == 0x28 /* EQU2  */) { result = opc_equ2(stack_index, ins, k); }
	else if (opcode == 0x09 /* NEQ   */) { result = opc_neq(stack_index, ins, k); }
	else if (opcode == 0x29 /* NEQ2  */) { result = opc_neq2(stack_index, ins, k); }
	else if (opcode == 0x0A /* GTH   */) { result = opc_gth(stack_index, ins, k); }
	else if (opcode == 0x2A /* GHT2  */) { result = opc_gth2(stack_index, ins, k); }
	else if (opcode == 0x0B /* LTH   */) { result = opc_lth(stack_index, ins, k); }
	else if (opcode == 0x2B /* LTH2  */) { result = opc_lth2(stack_index, ins, k); }
	else if (opcode == 0x0C /* JMP   */) { result = opc_jmp(stack_index, ins, k); }
	else if (opcode == 0x2C /* JMP2  */) { result = opc_jmp2(stack_index, ins, k); }
	else if (opcode == 0x0D /* JCN   */) { result = opc_jcn(stack_index, ins, k); }
	else if (opcode == 0x2D /* JCN2  */) { result = opc_jcn2(stack_index, ins, k); }
	else if (opcode == 0x0E /* JSR   */) { result = opc_jsr(stack_index, ins, k); }
	else if (opcode == 0x2E /* JSR2  */) { result = opc_jsr2(stack_index, ins, k); }
	else if (opcode == 0x0F /* STH   */) { result = opc_sth(stack_index, ins, k); }
	else if (opcode == 0x2F /* STH2  */) { result = opc_sth2(stack_index, ins, k); }
	else if (opcode == 0x10 /* LDZ   */) { result = opc_ldz(stack_index, ins, k); }
	else if (opcode == 0x30 /* LDZ2  */) { result = opc_ldz2(stack_index, ins, k); }
	else if (opcode == 0x11 /* STZ   */) { result = opc_stz(stack_index, ins, k); }
	else if (opcode == 0x31 /* STZ2  */) { result = opc_stz2(stack_index, ins, k); }
	else if (opcode == 0x12 /* LDR   */) { result = opc_ldr(stack_index, ins, k); }
	else if (opcode == 0x32 /* LDR2  */) { result = opc_ldr2(stack_index, ins, k); }
	else if (opcode == 0x13 /* STR   */) { result = opc_str(stack_index, ins, k); }
	else if (opcode == 0x33 /* STR2  */) { result = opc_str2(stack_index, ins, k); }
	else if (opcode == 0x14 /* LDA   */) { result = opc_lda(stack_index, ins, k); }
	else if (opcode == 0x34 /* LDA2  */) { result = opc_lda2(stack_index, ins, k); }
	else if (opcode == 0x15 /* STA   */) { result = opc_sta(stack_index, ins, k); }
	else if (opcode == 0x35 /* STA2  */) { result = opc_sta2(stack_index, ins, k); }
	else if (opcode == 0x16 /* DEI   */) { result = opc_dei(stack_index, ins, k); }
	else if (opcode == 0x36 /* DEI2  */) { result = opc_dei2(stack_index, ins, k); }
	else if (opcode == 0x17 /* DEO   */) { result = opc_deo(stack_index, ins, k); }
	else if (opcode == 0x37 /* DEO2  */) { result = opc_deo2(stack_index, ins, k); }
	else if (opcode == 0x18 /* ADD   */) { result = opc_add(stack_index, ins, k); }
	else if (opcode == 0x38 /* ADD2  */) { result = opc_add2(stack_index, ins, k); }
	else if (opcode == 0x19 /* SUB   */) { result = opc_sub(stack_index, ins, k); }
	else if (opcode == 0x39 /* SUB2  */) { result = opc_sub2(stack_index, ins, k); }
	else if (opcode == 0x1A /* MUL   */) { result = opc_mul(stack_index, ins, k); }
	else if (opcode == 0x3A /* MUL2  */) { result = opc_mul2(stack_index, ins, k); }
	else if (opcode == 0x1B /* DIV   */) { result = opc_div(stack_index, ins, k); }
	else if (opcode == 0x3B /* DIV2  */) { result = opc_div2(stack_index, ins, k); }
	else if (opcode == 0x1C /* AND   */) { result = opc_and(stack_index, ins, k); }
	else if (opcode == 0x3C /* AND2  */) { result = opc_and2(stack_index, ins, k); }
	else if (opcode == 0x1D /* ORA   */) { result = opc_ora(stack_index, ins, k); }
	else if (opcode == 0x3D /* ORA2  */) { result = opc_ora2(stack_index, ins, k); }
	else if (opcode == 0x1E /* EOR   */) { result = opc_eor(stack_index, ins, k); }
	else if (opcode == 0x3E /* EOR2  */) { result = opc_eor2(stack_index, ins, k); }
	else if (opcode == 0x1F /* SFT   */) { result = opc_sft(stack_index, ins, k); }
	else if (opcode == 0x3F /* SFT2  */) { result = opc_sft2(stack_index, ins, k); }
	
	return result;
}