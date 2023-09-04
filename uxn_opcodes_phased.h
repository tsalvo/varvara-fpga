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

// OPCODES
// The short mode (2) operates on 16-bit shorts, instead of bytes.
// The keep mode (k) operates without consuming items.
// The return mode (r) operates on the return stack.

// Break - Ends the evalutation of the current vector. This opcode has no modes.
uint1_t opc_brk_phased() {
	return 1;
}

// Jump Conditional Instant - Pops a byte from the working stack and if it is not zero, moves the PC to a relative address at a distance equal to the next short in memory, otherwise moves PC+2. This opcode has no modes.
uint1_t opc_jci_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index) {
	static uint8_t tmp8a, tmp8b;
	static uint16_t tmp16a, tmp16b;
	static uint1_t result;
	if (phase == 0) {
		result = 0;
		tmp8a = sp + 1;
		tmp16a = pc + 2;
		stack_pointer_set(stack_index, tmp8a); // START
		tmp8b = stack_data_get(stack_index, tmp8a); // DONE
		tmp16b = peek2_ram(tmp16a); // START
	} else if (phase == 1) {
		tmp16b = peek2_ram(tmp16a); // DONE
	} else if (phase == 2) {
		tmp16a = tmp8b == 0 ? 0 : tmp16b;
		pc_set(pc + tmp16a);	// START
	} else if (phase == 3) {
		result = 1; // DONE
	}
	
	return result;
}

// Jump Instant - Moves the PC to a relative address at a distance equal to the next short in memory. This opcode has no modes.
uint1_t opc_jmi_phased(uint4_t phase, uint16_t pc, uint8_t sp) {
	static uint16_t tmp16a, tmp16b;
	static uint1_t result;
	if (phase == 0) {
		result = 0;
		tmp16a = pc + 2;
		tmp16b = peek2_ram(tmp16a); // START
	} else if (phase == 1) {
		tmp16b = peek2_ram(tmp16a); // DONE
		pc_set(pc + tmp16b); // START
	} else if (phase == 2) {
		result = 1; // DONE
	} 
	return result;
}

// Jump Stash Return Instant - Pushes PC+2 to the return-stack and moves the PC to a relative address at a distance equal to the next short in memory. This opcode has no modes.
uint1_t opc_jsi_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins) {
	static uint8_t tmp8a;
	static uint16_t tmp16a, tmp16b;
	static uint1_t result;

	if (phase == 0) {
		result = sp > 253;
	}
	else if (phase == 1) {
		tmp8a = sp + 2;
		tmp16a = pc + 2;
		tmp16b = peek2_ram(pc) + 2; // START
		stack_pointer_set(stack_index, tmp8a); // DONE
		stack_data_set(stack_index, sp, (uint8_t)(tmp16a >> 8)); // START
	} else if (phase == 2) {
		tmp8a = stack_pointer_get(stack_index); // DONE
		tmp16b = peek2_ram(pc) + 2; // DONE
		stack_data_set(stack_index, sp + 1, (uint8_t)(tmp16a)); // START
		pc_set(pc + tmp16b); // START
	} else if (phase == 3) {
		result = 1; // DONE
	}
	
	return result;
}

// Literal - Pushes the next bytes in memory, and moves the PC+2. The LIT opcode always has the keep mode active. Notice how the 0x00 opcode, with the keep bit toggled, is the location of the literal opcodes. To learn more, see literals.
uint1_t opc_lit_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins) {
	static uint8_t tmp8a, tmp8b;
	static uint1_t result;
	
	if (phase == 0) {
		result = sp > 254;
	}
	else if (phase == 1) {
		tmp8a = peek_ram(pc); // START
		tmp8b = sp + 1;
		pc_set(pc + 1); // START
		stack_pointer_set(stack_index, tmp8b); // DONE
	}
	else if (phase == 2) {
		tmp8a = peek_ram(pc); // DONE
		stack_data_set(stack_index, sp, tmp8a); //START
	}
	else if (phase == 3) {
		result = 1; // DONE
	}
	
	return result;
}

uint1_t opc_lit2_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins) {
	static uint8_t tmp8a;
	static uint16_t tmp16a;
	static uint1_t result;
	
	if (phase == 0) {
		result = sp > 253;
	}
	else if (phase == 1) {
		tmp8a = sp + 2;
		tmp16a = peek2_ram(pc); // START
		pc_set(pc + 2); // START
		stack_pointer_set(stack_index, tmp8a); // DONE
	}
	else if (phase == 2) {
		tmp16a = peek2_ram(pc); // DONE
		stack_data_set(stack_index, sp, (uint8_t)(tmp16a >> 8)); // START
	}
	else if (phase == 3) {
		stack_data_set(stack_index, sp + 1, (uint8_t)(tmp16a)); // START
	}
	else if (phase == 4) {
		result = 1; // DONE
	}
	
	return result;
}

// Increment - Increments the value at the top of the stack, by 1.
uint1_t opc_inc_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 1, 0);                   
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = t_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		set(sp, stack_index, ins, k, 1, 0); // START
	}
	else if (phase == 4) {
		put_stack(sp, stack_index, 0, t8 + 1); // START
	}
	else if (phase == 5) {
		result = 1; // DONE
	}
	
	return result;
}

// Pop - Removes the value at the top of the stack.
uint1_t opc_pop_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint1_t set_will_succeed, result;
	if (phase == 0) {
		result = set_will_fail(sp, k, 1, -1);                  
	}
	else if (phase == 1) {
		set(sp, stack_index, ins, k, 1, -1); // START
	}
	else if (phase == 2) {
		result = 1; // DONE
	}
	
	return result;
}

uint1_t opc_pop2_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint1_t set_will_succeed, result;
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -2);                  
	}
	else if (phase == 1) {
		set(sp, stack_index, ins, k, 2, -2); // START
	}
	else if (phase == 2) {
		result = 1; // DONE
	}
	
	return result;
}

// Nip - Removes the second value from the stack. This is practical to convert a short into a byte.
uint1_t opc_nip_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = t_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 4) {
		put_stack(sp, stack_index, 0, t8); // START
	}
	else if (phase == 5) {
		result = 1; // DONE
	}
	
	return result;
}

// Swap - Exchanges the first and second values at the top of the stack.
uint1_t opc_swp_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, 0);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, 0); // START
	}
	else if (phase == 5) {
		put_stack(sp, stack_index, 0, n8); // START
	}
	else if (phase == 6) {
		put_stack(sp, stack_index, 1, t8); // START
	}
	else if (phase == 7) {
		result = 1; // DONE
	}
	
	return result;
}

// Rotate - Rotates three values at the top of the stack, to the left, wrapping around.
uint1_t opc_rot_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, l8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 3, 0);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE / START
	}
	else if (phase == 3) {
		n8 = l_register(stack_index, sp); // DONE / START
	}
	else if (phase == 4) {
		l8 = l_register(stack_index, sp); // DONE
	}
	else if (phase == 5) {
		set(sp, stack_index, ins, k, 3, 0); // START
	}
	else if (phase == 6) {
		put_stack(sp, stack_index, 0, l8); // START
	}
	else if (phase == 7) {
		put_stack(sp, stack_index, 1, t8); // START
	}
	else if (phase == 8) {
		put_stack(sp, stack_index, 2, n8); // START
	}
	else if (phase == 9) {
		result = 1; // DONE
	}
	
	return result;
}

// Duplicate - Duplicates the value at the top of the stack.
uint1_t opc_dup_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 1, 1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = t_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		set(sp, stack_index, ins, k, 1, 1); // START
	}
	else if (phase == 4) {
		put_stack(sp, stack_index, 0, t8); // START
	}
	else if (phase == 5) {
		put_stack(sp, stack_index, 1, t8); // START
	}
	else if (phase == 6) {
		result = 1; // DONE
	}
	
	return result;
}

// Over - Duplicates the second value at the top of the stack.
uint1_t opc_ovr_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, 1);		                   
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE / START
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE / START
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, 1); // START
	}
	else if (phase == 5) {
		put_stack(sp, stack_index, 0, n8); // START
	}
	else if (phase == 6) {
		put_stack(sp, stack_index, 1, t8); // START
	}
	else if (phase == 7) {
		put_stack(sp, stack_index, 2, n8); // START
	}
	else if (phase == 8) {
		result = 1; // DONE
	}
	
	return result;
}

uint1_t opc_ovr2_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t n16, t16;
	static uint8_t tmp8;
	static uint1_t result;
	
	if (phase == 0x0) {
		result = set_will_fail(sp, k, 4, 2);		                   
	}
	else if (phase == 0x1) {
		t16 = t2_register(stack_index, sp); // START
	}
	else if (phase == 0x2) {
		t16 = n2_register(stack_index, sp); // DONE / START
	}
	else if (phase == 0x3) {
		n16 = n2_register(stack_index, sp); // DONE / START
	}
	else if (phase == 0x4) {
		stack_data_set(stack_index, sp - 2, (uint8_t)(n16 >> 8));
	}
	else if (phase == 0x5) {
		stack_data_set(stack_index, sp - 1, (uint8_t)(n16));
	}
	else if (phase == 0x6) {
		stack_data_set(stack_index, sp - 4, (uint8_t)(t16 >> 8));
	}
	else if (phase == 0x7) {
		stack_data_set(stack_index, sp - 3, (uint8_t)(t16));
	}
	else if (phase == 0x8) {
		stack_data_set(stack_index, sp - 6, (uint8_t)(n16 >> 8));
	}
	else if (phase == 0x9) {
		stack_data_set(stack_index, sp - 5, (uint8_t)(n16));
	}
	else if (phase == 0xA) {
		result = 1;
	}
	
	return result;
}

// Equal - Pushes 01 to the stack if the two values at the top of the stack are equal, 00 otherwise.
uint1_t opc_equ_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);                   
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE / START
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE / START
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 5) {
		put_stack(sp, stack_index, 0, n8 == t8 ? 1 : 0); // START
	}
	else if (phase == 6) {
		result = 1; // DONE
	}
	
	return result;
}

// Not Equal - Pushes 01 to the stack if the two values at the top of the stack are not equal, 00 otherwise.
uint1_t opc_neq_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE / START
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE / START
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 5) {
		put_stack(sp, stack_index, 0, n8 == t8 ? 0 : 1); // START
	}
	else if (phase == 6) {
		result = 1; // DONE
	}
	
	return result;
}

// Greater Than - Pushes 01 to the stack if the second value at the top of the stack is greater than the value at the top of the stack, 00 otherwise.
uint1_t opc_gth_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 1) {
		t8 = n_register(stack_index, sp); // DONE / START
	}
	else if (phase == 2) {
		n8 = n_register(stack_index, sp); // DONE / START
	}
	else if (phase == 3) {
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 4) {
		put_stack(sp, stack_index, 0, n8 > t8 ? 1 : 0); // START
	}
	else if (phase == 5) {
		result = 1; // DONE
	}
	
	return result;
}

// Less Than - Pushes 01 to the stack if the second value at the top of the stack is lesser than the value at the top of the stack, 00 otherwise.
uint1_t opc_lth_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE / START
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE / START
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 5) {
		put_stack(sp, stack_index, 0, n8 < t8 ? 1 : 0); // START
	}
	else if (phase == 6) {
		result = 1; // DONE
	}
	
	return result;
}

// Jump - Moves the PC by a relative distance equal to the signed byte on the top of the stack, or to an absolute address in short mode.
uint1_t opc_jmp_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8, tmp8;
	static uint1_t result;
	if (phase == 0) {
		result = set_will_fail(sp, k, 1, -1);                
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = t_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		set(sp, stack_index, ins, k, 1, -1); // START
		pc_add_s8(pc, (int8_t)(t8)); // DONE
	}
	else if (phase == 4) {
		result = 1; // DONE
	}
	
	return result;
}

uint1_t opc_jmp2_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint1_t result;
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -2);                
	}
	else if (phase == 1) {
		t16 = t2_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t16 = t2_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		set(sp, stack_index, ins, k, 2, -2); // START
		pc_set(t16);
	}
	else if (phase == 4) {
		result = 1; // DONE
	}
	
	return result;
}

// Jump Conditional - If the byte preceding the address is not 00, moves the PC by a signed value equal to the byte on the top of the stack, or to an absolute address in short mode.
uint1_t opc_jcn_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -2);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE / START
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -2); // START
		pc_add_s8(pc, (int8_t)(t8)); // DONE
	}
	else if (phase == 5) {
		result = 1; // DONE
	}
	
	return result;
}

// Jump Stash Return - Pushes the PC to the return-stack and moves the PC by a signed value equal to the byte on the top of the stack, or to an absolute address in short mode.
uint1_t opc_jsr_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8, tmp8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 1, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = t_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		set(sp, stack_index, ins, k, 1, -1); // START
	}
	else if (phase == 4) {
		tmp8 = stack_pointer_get(1); // DONE
		result = tmp8 > 253;
	}
	else if (phase == 5) {
		stack_data_set(1, tmp8, (uint8_t)(pc >> 8)); // START
	}
	else if (phase == 6) {
		stack_data_set(1, tmp8 + 1, (uint8_t)(pc)); // START
	}
	else if (phase == 7) {
		stack_pointer_set(1, tmp8 + 2); // DONE
		pc_add_s8(pc, (int8_t)(t8)); // DONE
		result = 1;
	}
	
	return result;
}

// Stash - Moves the value at the top of the stack, to the return stack.
uint1_t opc_sth_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8, tmp8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 1, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = t_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		set(sp, stack_index, ins, k, 1, -1); // START
	}
	else if (phase == 4) {
		result = sp > 254;
	}
	else if (phase == 5) {
		stack_data_set(stack_index, sp, t8);
	}
	else if (phase == 6) {
		stack_pointer_set(stack_index, sp + 1); // DONE
		result = 1; // DONE
	}

	return result;
}

// Load Zero-Page -  Pushes the value at an address within the first 256 bytes of memory, to the top of the stack.
uint1_t opc_ldz_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t8;
	static uint8_t ram8_at_tmp16, tmp16;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 1, 0);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = t_register(stack_index, sp); // DONE
		tmp16 = (uint16_t)(t8);
		ram8_at_tmp16 = peek_ram(tmp16); // START
	}
	else if (phase == 3) {
		ram8_at_tmp16 = peek_ram(tmp16); // DONE
		set(sp, stack_index, ins, k, 1, 0); // START
	}
	else if (phase == 4) {
		put_stack(sp, stack_index, 0, ram8_at_tmp16); // START
	}
	else if (phase == 5) {
		result = 1; // DONE
	}
	
	return result;
}

// Store Zero-Page - Writes a value to an address within the first 256 bytes of memory.
uint1_t opc_stz_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t tmp16;
	static uint8_t t8, n8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -2);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE / START
		tmp16 = (uint16_t)(t8);
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -2); // START
		poke_ram(tmp16, n8); // START
	}
	else if (phase == 5) {
		result = 1; // DONE
	}
	
	return result;
}

// Load Relative - Pushes a value at a relative address in relation to the PC, within a range between -128 and +127 bytes, to the top of the stack.
uint1_t opc_ldr_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t8;
	static uint8_t ram8_at_tmp16, tmp16;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 1, 0);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = t_register(stack_index, sp); // DONE
		tmp16 = pc + ((int8_t)(t8));
		ram8_at_tmp16 = peek_ram(tmp16); // START
	}
	else if (phase == 3) {
		ram8_at_tmp16 = peek_ram(tmp16); // DONE
		set(sp, stack_index, ins, k, 1, 0); // START
	}
	else if (phase == 4) {
		put_stack(sp, stack_index, 0, ram8_at_tmp16); // START
	}
	else if (phase == 5) {
		result = 1; // DONE
	}
	
	return result;
}

// Store Relative - Writes a value to a relative address in relation to the PC, within a range between -128 and +127 bytes.
uint1_t opc_str_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t tmp16;
	static uint8_t t8, n8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -2);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE
		tmp16 = pc + ((int8_t)(t8));
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -2); // START
		poke_ram(tmp16, n8); // START
	}
	else if (phase == 5) {
		result = 1; // DONE
	}
	
	return result;
}

// Load Absolute - Pushes the value at a absolute address, to the top of the stack.
uint1_t opc_lda_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t ram8_at_t16;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);
	}
	else if (phase == 1) {
		t16 = t2_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t16 = t2_register(stack_index, sp); // DONE
		ram8_at_t16 = peek_ram(t16); // START
	}
	else if (phase == 3) {
		ram8_at_t16 = peek_ram(t16); // DONE
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 4) {
		put_stack(sp, stack_index, 0, ram8_at_t16); // START
	}
	else if (phase == 5) {
		result = 1; // DONE
	}
	
	return result;
}

// Store Absolute - Writes a value to a absolute address.
uint1_t opc_sta_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint16_t t16;
	static uint8_t l8;
	static uint1_t set_will_succeed, result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 3, -3);
	}
	else if (phase == 1) {
		t16 = t2_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t16 = t2_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		l8 = l_register(stack_index, sp); // START
	}
	else if (phase == 4) {
		l8 = l_register(stack_index, sp); // DONE
	}
	else if (phase == 5) {
		set(sp, stack_index, ins, k, 3, -3); // START
		poke_ram(t16, l8); // START
	}
	else if (phase == 6) {
		result = 1; // DONE
	}
	
	return result;
}

// Device Input - Pushes a value from the device page, to the top of the stack. The target device might capture the reading to trigger an I/O event.
uint1_t opc_dei_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 1, 0);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = t_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		set(sp, stack_index, ins, k, 1, 0); // START
	}
	else if (phase == 4) {
		result = dei_phased(0x0, sp, stack_index, 0, t8);
	}
	else if (phase == 5) {
		result = dei_phased(0x1, sp, stack_index, 0, t8);
	}
	else if (phase == 6) {
		result = dei_phased(0x2, sp, stack_index, 0, t8);
	}
	else if (phase == 7) {
		result = dei_phased(0x3, sp, stack_index, 0, t8);
	}
	else if (phase == 8) {
		result = dei_phased(0x4, sp, stack_index, 0, t8);
	}
	else if (phase == 9) {
		result = dei_phased(0x5, sp, stack_index, 0, t8);
	}
	else if (phase == 10) {
		result = dei_phased(0x6, sp, stack_index, 0, t8);
	}
	else if (phase == 11) {
		result = dei_phased(0x7, sp, stack_index, 0, t8);
	}
	else if (phase == 12) {
		result = dei_phased(0x8, sp, stack_index, 0, t8);
	}
	
	return result;
}

// Device Output - Writes a value to the device page. The target device might capture the writing to trigger an I/O event.
uint1_t opc_deo_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint1_t result;
	
	if (phase == 0x0) {
		result = set_will_fail(sp, k, 2, -2);
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 0x1) {
		t8 = n_register(stack_index, sp); // DONE / START
	}
	else if (phase == 0x2) {
		n8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 0x3) {
		set(sp, stack_index, ins, k, 2, -2); // START
	}
	else if (phase == 0x4) {
		result = deo_phased(0x0, t8, n8);
	}
	else if (phase == 0x5) {
		result = deo_phased(0x1, t8, n8);
	}
	else if (phase == 0x6) {
		result = deo_phased(0x2, t8, n8);
	}
	else if (phase == 0x7) {
		result = deo_phased(0x3, t8, n8);
	}
	else if (phase == 0x8) {
		result = deo_phased(0x4, t8, n8);
	}
	else if (phase == 0x9) {
		result = deo_phased(0x5, t8, n8);
	}
	else if (phase == 0xA) {
		result = deo_phased(0x6, t8, n8);
	}
	else if (phase == 0xB) {
		result = deo_phased(0x7, t8, n8);
	}
	else if (phase == 0xC) {
		result = deo_phased(0x8, t8, n8);
	}
	else if (phase == 0xD) {
		result = deo_phased(0x9, t8, n8);
	}
	
	return result;
}

uint1_t opc_deo2_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8, tmp8;
	static uint8_t h16;
	static uint1_t result;
	
	if (phase == 0x0) {
		result = set_will_fail(sp, k, 3, -3);
		t8 = t_register(stack_index, sp); // START t
	}
	else if (phase == 0x1) {
		t8 = h2_register(stack_index, sp); // DONE t / START h2
	}
	else if (phase == 0x2) {
		h16 = h2_register(stack_index, sp); // DONE h2 (a.k.a. [l8][n8])
		l8 = (uint8_t)(h16 >> 8);
		n8 = (uint8_t)(h16); 
		result = deo_phased(0x0, t8, l8);
	}
	else if (phase == 0x3) {
		set(sp, stack_index, ins, k, 3, -3); // START
		result = deo_phased(0x1, t8, l8);
	}
	else if (phase == 0x4) {
		result = deo_phased(0x2, t8, l8);
	}
	else if (phase == 0x5) {
		result = deo_phased(0x3, t8, l8);
	}
	else if (phase == 0x6) {
		result = deo_phased(0x4, t8, l8);
	}
	else if (phase == 0x7) {
		result = deo_phased(0x5, t8, l8);
	}
	else if (phase == 0x8) {
		result = deo_phased(0x0, t8 + 1, n8);
	}
	else if (phase == 0x9) {
		result = deo_phased(0x1, t8 + 1, n8);
	}
	else if (phase == 0xA) {
		result = deo_phased(0x2, t8 + 1, n8);
	}
	else if (phase == 0xB) {
		result = deo_phased(0x3, t8 + 1, n8);
	}
	else if (phase == 0xC) {
		result = deo_phased(0x4, t8 + 1, n8);
	}
	else if (phase == 0xD) {
		result = deo_phased(0x5, t8 + 1, n8);
	}
	
	return result;
}

// Add - Pushes the sum of the two values at the top of the stack.
uint1_t opc_add_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 5) {
		put_stack(sp, stack_index, 0, n8 + t8); // START
	}
	else if (phase == 6) {
		result = 1; // DONE
	}
	
	return result;
}

// Subtract - Pushes the difference of the first value minus the second, to the top of the stack.
uint1_t opc_sub_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 5) {
		put_stack(sp, stack_index, 0, n8 - t8); // START
	}
	else if (phase == 6) {
		result = 1; // DONE
	}
	
	return result;
}

// Multiply - Pushes the product of the first and second values at the top of the stack.
uint1_t opc_mul_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 5) {
		put_stack(sp, stack_index, 0, n8 * t8); // START
	}
	else if (phase == 6) {
		result = 1; // DONE
	}
	
	return result;
}

// Divide - Pushes the quotient of the first value over the second, to the top of the stack.
uint1_t opc_div_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 5) {
		result = t8 == 0;	// fail out early before trying to divide by zero
	}
	else if (phase == 6) {
		put_stack(sp, stack_index, 0, n8 / t8); // START
	}
	else if (phase == 7) {
		result = 1; // DONE
	}
	
	return result;
}

// And - Pushes the result of the bitwise operation AND, to the top of the stack.
uint1_t opc_and_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 5) {
		put_stack(sp, stack_index, 0, n8 & t8); // START
	}
	else if (phase == 6) {
		result = 1; // DONE
	}
	
	return result;
}

// Or - Pushes the result of the bitwise operation OR, to the top of the stack.
uint1_t opc_ora_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 5) {
		put_stack(sp, stack_index, 0, n8 | t8); // START
	}
	else if (phase == 6) {
		result = 1; // DONE
	}
	
	return result;
}

// Exclusive Or - Pushes the result of the bitwise operation XOR, to the top of the stack.
uint1_t opc_eor_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 5) {
		put_stack(sp, stack_index, 0, n8 ^ t8); // START
	}
	else if (phase == 6) {
		result = 1; // DONE
	}
	
	return result;
}

// Shift - Shifts the bits of the second value of the stack to the left or right, depending on the control value at the top of the stack. The high nibble of the control value indicates how many bits to shift left, and the low nibble how many bits to shift right. The rightward shift is done first.
uint1_t opc_sft_phased(uint4_t phase, uint16_t pc, uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k) {
	static uint8_t n8, t8;
	static uint1_t result;
	
	if (phase == 0) {
		result = set_will_fail(sp, k, 2, -1);
	}
	else if (phase == 1) {
		t8 = t_register(stack_index, sp); // START
	}
	else if (phase == 2) {
		t8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 3) {
		n8 = n_register(stack_index, sp); // DONE
	}
	else if (phase == 4) {
		set(sp, stack_index, ins, k, 2, -1); // START
	}
	else if (phase == 5) {
		put_stack(sp, stack_index, 0, (n8 >> (t8 & 0x0F)) << (t8 >> 4)); // START
	}
	else if (phase == 6) {
		result = 1; // DONE
	}
	
	return result;
}

uint1_t eval_opcode_phased(
	uint4_t phase,
	uint16_t pc,
	uint8_t sp,
	uint1_t stack_index,
	uint8_t opcode,
	uint8_t ins,
	uint8_t k
) {
	static uint8_t 	t8, n8, l8, tmp8;
	static uint16_t t16, n16, l16, tmp16;
	static uint1_t result;
	
	result = 0;
	
	if      (opcode == 0x00 /* BRK   */) { result = opc_brk_phased(); }
	else if (opcode == 0xFF /* JCI   */) { result = opc_jci_phased(phase, pc, sp, stack_index); }
	else if (opcode == 0xFE /* JMI   */) { result = opc_jmi_phased(phase, pc, sp); }
	else if (opcode == 0xFD /* JSI   */) { result = opc_jsi_phased(phase, pc, sp, stack_index, ins); }
	else if (opcode == 0xFC /* LIT   */) { result = opc_lit_phased(phase, pc, sp, stack_index, ins); }
	else if (opcode == 0xFB /* LIT2  */) { result = opc_lit2_phased(phase, pc, sp, stack_index, ins); }
	else if (opcode == 0xFA /* LITr  */) { result = opc_lit_phased(phase, pc, sp, stack_index, ins); }
	else if (opcode == 0xF9 /* LIT2r */) { result = opc_lit2_phased(phase, pc, sp, stack_index, ins); }
	else if (opcode == 0x01 /* INC   */) { result = opc_inc_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x21 /* INC2  */) { result = 1; }
	else if (opcode == 0x02 /* POP   */) { result = opc_pop_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x22 /* POP2  */) { result = opc_pop2_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x03 /* NIP   */) { result = opc_nip_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x23 /* NIP2  */) { result = 1; }
	else if (opcode == 0x04 /* SWP   */) { result = opc_swp_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x24 /* SWP2  */) { result = 1; }
	else if (opcode == 0x05 /* ROT   */) { result = opc_rot_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x25 /* ROT2  */) { result = 1; }
	else if (opcode == 0x06 /* DUP   */) { result = opc_dup_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x26 /* DUP2  */) { result = 1; }
	else if (opcode == 0x07 /* OVR   */) { result = opc_ovr_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x27 /* OVR2  */) { result = opc_ovr2_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x08 /* EQU   */) { result = opc_equ_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x28 /* EQU2  */) { result = 1; }
	else if (opcode == 0x09 /* NEQ   */) { result = opc_neq_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x29 /* NEQ2  */) { result = 1; }
	else if (opcode == 0x0A /* GTH   */) { result = opc_gth_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x2A /* GHT2  */) { result = 1; }
	else if (opcode == 0x0B /* LTH   */) { result = opc_lth_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x2B /* LTH2  */) { result = 1; }
	else if (opcode == 0x0C /* JMP   */) { result = opc_jmp_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x2C /* JMP2  */) { result = opc_jmp2_phased(phase, pc, sp, stack_index, ins, k);}
	else if (opcode == 0x0D /* JCN   */) { result = opc_jcn_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x2D /* JCN2  */) { result = 1; }
	else if (opcode == 0x0E /* JSR   */) { result = opc_jsr_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x2E /* JSR2  */) { result = 1; }
	else if (opcode == 0x0F /* STH   */) { result = opc_sth_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x2F /* STH2  */) { result = 1; }
	else if (opcode == 0x10 /* LDZ   */) { result = opc_ldz_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x30 /* LDZ2  */) { result = 1; }
	else if (opcode == 0x11 /* STZ   */) { result = opc_stz_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x31 /* STZ2  */) { result = 1; }
	else if (opcode == 0x12 /* LDR   */) { result = opc_ldr_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x32 /* LDR2  */) { result = 1; }
	else if (opcode == 0x13 /* STR   */) { result = opc_str_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x33 /* STR2  */) { result = 1; }
	else if (opcode == 0x14 /* LDA   */) { result = opc_lda_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x34 /* LDA2  */) { result = 1; }
	else if (opcode == 0x15 /* STA   */) { result = opc_sta_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x35 /* STA2  */) { result = 1; }
	else if (opcode == 0x16 /* DEI   */) { result = opc_dei_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x36 /* DEI2  */) { result = 1; }
	else if (opcode == 0x17 /* DEO   */) { result = opc_deo_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x37 /* DEO2  */) { result = opc_deo2_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x18 /* ADD   */) { result = opc_add_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x38 /* ADD2  */) { result = 1; }
	else if (opcode == 0x19 /* SUB   */) { result = opc_sub_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x39 /* SUB2  */) { result = 1; }
	else if (opcode == 0x1A /* MUL   */) { result = opc_mul_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x3A /* MUL2  */) { result = 1; }
	else if (opcode == 0x1B /* DIV   */) { result = opc_div_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x3B /* DIV2  */) { result = 1; }
	else if (opcode == 0x1C /* AND   */) { result = opc_and_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x3C /* AND2  */) { result = 1; }
	else if (opcode == 0x1D /* ORA   */) { result = opc_ora_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x3D /* ORA2  */) { result = 1; }
	else if (opcode == 0x1E /* EOR   */) { result = opc_eor_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x3E /* EOR2  */) { result = 1; }
	else if (opcode == 0x1F /* SFT   */) { result = opc_sft_phased(phase, pc, sp, stack_index, ins, k); }
	else if (opcode == 0x3F /* SFT2  */) { result = 1; }
	
	return result;
}