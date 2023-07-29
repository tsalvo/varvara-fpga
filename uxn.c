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
#pragma MAIN_MHZ uxn_eval 1.0
uint1_t uxn_eval() {

	static uint8_t k, opc, ins;
	static uint1_t s, should_return, result;

	result = 0;
	should_return = 0;

	if(pc_get() == 0) {
		should_return = 1;
	}
	
	if (device_ram_read(15) != 0) {
		should_return = 1;
	}
	
	if (!should_return) {
		ins = main_ram_read(pc_get()) & 0xFF;
		pc_add(1);
		k = ins & 0x80 ? 0xFF : 0x00;
		s = ins & 0x40 ? 1 : 0;
		opc = !(ins & 0x1F) ? (0 - (ins >> 5)) & 0xFF : ins & 0x3F;	
		result = eval_opcode(s, opc, ins, k);
	}
	
	return result;
}