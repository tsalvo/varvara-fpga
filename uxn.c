#include "uintN_t.h"  // uintN_t types for any N
#include "intN_t.h"   // intN_t types for any N

#include "uxn_opcodes.h"

// RULES:
// - cannot write to a global variable from more than one function (unless you use clock domain crossing)
// - no switch statements (C AST node cannot be parsed to logic)
// - only one return per function
// - no ++ or -- operators

// Build with Docker pipelinec image: 
// docker run -v $(pwd):/workdir pipelinec pipelinec uxn.c

// Top-level module
#pragma MAIN_MHZ uxn_eval 12.287999
uint8_t uxn_eval() {
	static uint8_t led_error = 0b00010000;
	static uint8_t led_blink = 0b00001000;
	static uint8_t led_r = 0b00000100;
	static uint8_t led_g = 0b00000010;
	static uint8_t led_b = 0b00000001;
	static uint8_t result = 0;
	static uint32_t counter;
	static uint1_t s, pc_nonzero, system_state_zero, should_eval, error;
	static uint8_t k, opc, ins, system_state;
	static uint16_t pc;

	error = 0;
	pc = pc_get();
	pc_nonzero = pc == 0 ? 0 : 1;
	system_state = device_ram_read(15);
	system_state_zero = system_state == 0 ? 1 : 0;
	should_eval = pc_nonzero & system_state_zero;
	
	if (should_eval) {
		ins = main_ram_read(pc) & 0xFF;
		pc_add(1);
		k = ins & 0x80 ? 0xFF : 0x00;
		s = ins & 0x40 ? 1 : 0;
		opc = !(ins & 0x1F) ? (0 - (ins >> 5)) & 0xFF : ins & 0x3F;	
		error = eval_opcode(s, opc, ins, k);
	}
	
	// test blinking LEDs
	if (counter == (12287999 - 1)) { // If reached 1 second
		
		result = result ^ led_blink;
		result = result ^ led_r;
		result = result ^ led_g;
		result = result ^ led_b;
		counter = 0; // Reset counter
	} else {
		counter += 1;
	}
	
	if (counter == (9216000 - 1)) { // If reached 0.75 second 
		result = result ^ led_b;
	}
	
	if (counter == (6144000 - 1)) { // If reached 0.5 second 
		result = result ^ led_r;
		result = result ^ led_b;
	}
	
	if (counter == (4096000 - 1)) { // If reached 0.333 second 
		result = result ^ led_g;
	}
	
	if (counter == (8192000 - 1)) { // If reached 0.666 second 
		result = result ^ led_g;
	}
	
	if (counter == (3072000 - 1)) { // If reached 0.25 second 
		result = result ^ led_b;
	}
	
	result = (result & 0b00001111) | (error ? led_error : 0x00);
	
	return result;
}