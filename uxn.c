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

	static uint1_t s, pc_nonzero, system_state_zero, should_eval, result;
	static uint8_t k, opc, ins, system_state;
	static uint16_t pc;

	result = 0;
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
		result = eval_opcode(s, opc, ins, k);
	}
	
	return result;
}