#include "uintN_t.h"  // uintN_t types for any N

#include "stack.h"

// SHARED

uint16_t peek2_stack_func(Stack s, uint16_t address) {
	uint16_t mem0 = s.dat[address];
	uint16_t mem1 = s.dat[address + 1];
	return (mem0 << 8) | mem1;
}

// REGISTERS

#pragma MAIN t_register_func
uint8_t t_register_func(Stack s) {
	return s.dat[s.ptr - 1];
}

#pragma MAIN n_register_func
uint8_t n_register_func(Stack s) {
	return s.dat[s.ptr - 2];
}

#pragma MAIN l_register_func
uint8_t l_register_func(Stack s) {
	return s.dat[s.ptr - 3];
}

#pragma MAIN h2_register_func
uint16_t h2_register_func(Stack s) {
	return peek2_stack_func(s, (uint16_t)(s.ptr) - 3);
}

#pragma MAIN t2_register_func
uint16_t t2_register_func(Stack s) {
	return peek2_stack_func(s, (uint16_t)(s.ptr) - 2);
}

#pragma MAIN n2_register_func
uint16_t n2_register_func(Stack s) {
	return peek2_stack_func(s, (uint16_t)(s.ptr) - 4);
}

#pragma MAIN l2_register_func
uint16_t l2_register_func(Stack s) {
	return peek2_stack_func(s, (uint16_t)(s.ptr) - 6);
}