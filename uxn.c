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
uint16_t uxn_eval(uint16_t input) {
	static uint12_t palette_color_values[4] = {0xFFF, 0x000, 0x7DB, 0xF62};
	static uint32_t pixel_counter = 0; // 512*400, max = 204799
	static uint32_t clock_cycle_counter = 0;
	static uint32_t seconds_counter = 0;
	static uint16_t result;
	static uint2_t current_pixel_palette_color;
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
	
	current_pixel_palette_color = screen_ram_update(
		pixel_counter, 					// write address
		(uint2_t)(seconds_counter),		// write value
		pixel_counter					// read address
	);
	
	result = (uint16_t)(palette_color_values[current_pixel_palette_color]);
	
	// Pixel Counter
	if (pixel_counter == 204800 - 1) {
		pixel_counter = 0;
	} else {
		pixel_counter += 1;
	}
	
	// Clock Counter
	if (clock_cycle_counter == (12287999 - 1)) { // If reached 1 second
		seconds_counter += 1;
		clock_cycle_counter = 0; // Reset counter
	} else {
		clock_cycle_counter += 1;
	}
	
	return result;
}