#pragma once
#include "uintN_t.h"   // uintN_t types for any N

#pragma once
#include "uxn_stack.h"

/* Registers
[ Z ][ Y ][ X ][ L ][ N ][ T ] <
[ . ][ . ][ . ][   H2   ][ T ] <
[   L2   ][   N2   ][   T2   ] <
*/
// 
// #define T *(ptr)
// #define N *(ptr - 1)
// #define L *(ptr - 2)
// #define X *(ptr - 3)
// #define Y *(ptr - 4)
// #define Z *(ptr - 5)
// #define T2 (N << 8 | T)
// #define H2 (L << 8 | N)
// #define N2 (X << 8 | L)
// #define L2 (Z << 8 | Y)
// #define T2_(v) { r = (v); T = r; N = r >> 8; }
// #define N2_(v) { r = (v); L = r; X = r >> 8; }
// #define L2_(v) { r = (v); Y = r; Z = r >> 8; }

uint1_t flip_stack_index(uint1_t stack_index) {
	return ~stack_index;
}

void shift(int8_t y, uint1_t stack_index) {
	static uint8_t sp;
	sp = stack_pointer_get(stack_index); // DONE
	stack_pointer_set(stack_index, sp + y); // DONE
}

void set_sp(uint8_t x, int8_t y, uint8_t ins, uint1_t stack_index) {
	static uint8_t shift_amount;
	shift_amount = (ins & 0x80) ? x + y : y;
	shift(shift_amount, stack_index);
}

void set_t(uint8_t value, uint1_t stack_index) {
	static uint8_t sp;
	sp = stack_pointer_get(stack_index); // DONE
	stack_data_set(stack_index, sp - 1, value); // START
}

void set_t2_2p(uint16_t value, uint4_t phase, uint1_t stack_index) {
	static uint8_t sp;
	if (phase == 0) { // T
		sp = stack_pointer_get(stack_index); // DONE
		stack_data_set(stack_index, sp - 1, (uint8_t)(value)); // START
	} else if (phase == 1) { // N
		stack_data_set(stack_index, sp - 2, (uint8_t)(value >> 8)); // START
	}
}

void set_n2_2p(uint16_t value, uint4_t phase, uint1_t stack_index) {
	static uint8_t sp;
	if (phase == 0) { // L
		sp = stack_pointer_get(stack_index); // DONE
		stack_data_set(stack_index, sp - 3, (uint8_t)(value)); // START
	} else if (phase == 1) { // X
		stack_data_set(stack_index, sp - 4, (uint8_t)(value >> 8)); // START
	}
}

void set_l2_2p(uint16_t value, uint4_t phase, uint1_t stack_index) {
	static uint8_t sp;
	if (phase == 0) { // Y
		sp = stack_pointer_get(stack_index); // DONE
		stack_data_set(stack_index, sp - 5, (uint8_t)(value)); // START
	} else if (phase == 1) { // Z
		stack_data_set(stack_index, sp - 6, (uint8_t)(value >> 8)); // START
	}
}

uint16_t get_t2_2p(uint4_t phase, uint1_t stack_index) {
	static uint8_t sp;
	sp = stack_pointer_get(stack_index); // DONE
	return peek2_stack(stack_index, sp - 2); // T2 (a.k.a. [ N ][ T ])
}

uint16_t get_h2_2p(uint4_t phase, uint1_t stack_index) {
	static uint8_t sp;
	sp = stack_pointer_get(stack_index); // DONE
	return peek2_stack(stack_index, sp - 3); // H2 (a.k.a. [ L ][ N ])
}

uint16_t get_n2_2p(uint4_t phase, uint1_t stack_index) {
	static uint8_t sp;
	sp = stack_pointer_get(stack_index); // DONE
	return peek2_stack(stack_index, sp - 4); // N2 (a.k.a. [ X ][ L ])
}

uint16_t get_l2_2p(uint4_t phase, uint1_t stack_index) {
	static uint8_t sp;
	sp = stack_pointer_get(stack_index); // DONE
	return peek2_stack(stack_index, sp - 6);
}

uint8_t get_t_2p(uint1_t stack_index) {
	static uint8_t sp;
	sp = stack_pointer_get(stack_index); // DONE
	return stack_data_get(stack_index, sp - 1);
}

uint8_t get_n_2p(uint1_t stack_index) {
	static uint8_t sp;
	sp = stack_pointer_get(stack_index); // DONE
	return stack_data_get(stack_index, sp - 2);
}

uint8_t get_l_2p(uint1_t stack_index) {
	static uint8_t sp;
	sp = stack_pointer_get(stack_index); // DONE
	return stack_data_get(stack_index, sp - 3);
}

uint8_t get_x_2p(uint1_t stack_index) {
	static uint8_t sp;
	sp = stack_pointer_get(stack_index); // DONE
	return stack_data_get(stack_index, sp - 4);
}

uint8_t get_y_2p(uint1_t stack_index) {
	static uint8_t sp;
	sp = stack_pointer_get(stack_index); // DONE
	return stack_data_get(stack_index, sp - 5);
}

uint8_t get_z_2p(uint1_t stack_index) {
	static uint8_t sp;
	sp = stack_pointer_get(stack_index); // DONE
	return stack_data_get(stack_index, sp - 6);
}