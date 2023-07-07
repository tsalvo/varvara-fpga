#include "uintN_t.h"  // uintN_t types for any N

#include "stack.h"
// uint8_t stack_data[2][255]; // 0 = working stack, 1 = return stack
// uint8_t stack_ptr[2];  // 0 = working stack ptr, 1 = return stack ptr

// SHARED

uint16_t peek2_stack_func(uint1_t stack_index, uint8_t offset) {
	// stack_index: 0 = working stack, 1 = return stack
	uint16_t mem0 = stack_data[stack_index][offset];
	uint16_t mem1 = stack_data[stack_index][offset + 1];
	return (mem0 << 8) | mem1;
}

#define HALT(c) { return uxn_halt(u, ins, (c), pc - 1); }

uint16_t push2_stack_func(uint1_t stack_index, uint8_t a, uint8_t b) {
	// stack_index: 0 = working stack, 1 = return stack
	
}

