#pragma once
#include "uintN_t.h"   // uintN_t types for any N

#pragma once
#include "uxn_stack.h"

uint8_t t_register(uint1_t stack_index) {
	return stack_data_get(stack_index, stack_pointer_get(stack_index) - 1);
}

uint8_t n_register(uint1_t stack_index) {
	return stack_data_get(stack_index, stack_pointer_get(stack_index) - 2);
}

uint8_t l_register(uint1_t stack_index) {
	return stack_data_get(stack_index, stack_pointer_get(stack_index) - 3);
}

uint16_t h2_register(uint1_t stack_index) {
	return peek2_stack(stack_index, stack_pointer_get(stack_index) - 3);
}

uint16_t t2_register(uint1_t stack_index) {
	return peek2_stack(stack_index, stack_pointer_get(stack_index) - 2);
}

uint16_t n2_register(uint1_t stack_index) {
	return peek2_stack(stack_index, stack_pointer_get(stack_index) - 4);
}

uint16_t l2_register(uint1_t stack_index) {
	return peek2_stack(stack_index, stack_pointer_get(stack_index) - 6);
}