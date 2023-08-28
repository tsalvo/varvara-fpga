#include "uintN_t.h"  // uintN_t types for any N
#include "intN_t.h"   // intN_t types for any N

#include "uxn_opcodes_phased.h"

// RULES:
// - cannot write to a global variable from more than one function (unless you use clock domain crossing)
// - no switch statements (C AST node cannot be parsed to logic)
// - only one return per function
// - no ++ or -- operators

// Build with Docker pipelinec image: 
// docker run -v $(pwd):/workdir pipelinec pipelinec uxn.c

// https://github.com/JulianKemmerer/PipelineC/wiki/Automatically-Generated-Functionality#rams


// 16-bit input message format:
// 0001 UDLR SSBA YXLR  Controls
// 0010 ---- ---- -PHV  (P)Visible Pixel, (H)HBLANK, (V)VBLANK

void step_cpu() {
	static uint16_t pc;
	static uint8_t sp, sp0, sp1, k, opc, ins, system_state;
	static uint4_t cpu_phase;
	static uint1_t stack_index, pc_nonzero, system_state_zero, should_cpu_eval, eval_result;
	
	if (cpu_phase == 0x0) {
		eval_result = 0;
		pc = pc_get(); // DONE
		system_state = peek_dev(15); // START
		sp0 = stack_pointer_get(0); // START
		ins = peek_ram(pc) & 0xFF; // START
		eval_result = 0;
	}
	else if (cpu_phase == 0x1) {
		sp0 = stack_pointer_get(1); // DONE / START
		ins = peek_ram(pc) & 0xFF; // DONE
		system_state = peek_dev(15); // DONE
		k = ins & 0x80 ? 0xFF : 0x00;
		stack_index = ins & 0x40 ? 1 : 0;
		opc = !(ins & 0x1F) ? (0 - (ins >> 5)) & 0xFF : ins & 0x3F;
		system_state_zero = system_state == 0 ? 1 : 0;
		should_cpu_eval = pc_nonzero & system_state_zero;
		pc_add((uint16_t)(should_cpu_eval)); // DONE
		eval_result = ~should_cpu_eval;
	}
	else if (cpu_phase == 0x2) {
		sp1 = stack_pointer_get(1); // DONE
		sp = stack_index == 0 ? sp0 : sp1;
		pc = pc_get();
		eval_result = 0;
	}
	else if (cpu_phase == 0x3) {
		eval_result = eval_opcode_phased(0x0, pc, sp, stack_index, opc, ins, k);
	}
	else if (cpu_phase == 0x4) {
		eval_result = eval_opcode_phased(0x1, pc, sp, stack_index, opc, ins, k);
	}
	else if (cpu_phase == 0x5) {
		eval_result = eval_opcode_phased(0x2, pc, sp, stack_index, opc, ins, k);
	}
	else if (cpu_phase == 0x6) {
		eval_result = eval_opcode_phased(0x3, pc, sp, stack_index, opc, ins, k);
	}
	else if (cpu_phase == 0x7) {
		eval_result = eval_opcode_phased(0x4, pc, sp, stack_index, opc, ins, k);
	}
	else if (cpu_phase == 0x8) {
		eval_result = eval_opcode_phased(0x5, pc, sp, stack_index, opc, ins, k);
	}
	else if (cpu_phase == 0x9) {
		eval_result = eval_opcode_phased(0x6, pc, sp, stack_index, opc, ins, k);
	}
	else if (cpu_phase == 0xA) {
		eval_result = eval_opcode_phased(0x7, pc, sp, stack_index, opc, ins, k);
	}
	else if (cpu_phase == 0xB) {
		eval_result = eval_opcode_phased(0x8, pc, sp, stack_index, opc, ins, k);
	}
	else if (cpu_phase == 0xC) {
		eval_result = eval_opcode_phased(0x9, pc, sp, stack_index, opc, ins, k);
	}
	else if (cpu_phase == 0xD) {
		eval_result = eval_opcode_phased(0xA, pc, sp, stack_index, opc, ins, k);
	}
	else if (cpu_phase == 0xE) {
		eval_result = eval_opcode_phased(0xB, pc, sp, stack_index, opc, ins, k);
	}
	else if (cpu_phase == 0xF) {
		eval_result = eval_opcode_phased(0xC, pc, sp, stack_index, opc, ins, k);
	}
	
	if (eval_result == 1) {
		cpu_phase = 0;
	} else {
		cpu_phase += 1;
	}
}

uint2_t step_gpu(uint1_t is_active_drawing_area, uint32_t seconds_counter) {
	static uint2_t result;
	static uint32_t pixel_counter;
	if (is_active_drawing_area) {
		result = background_vram_update(
			pixel_counter, 					// port 0 address
			(uint2_t)(seconds_counter),		// port 0 write value
			1,								// port 0 write enable
			0,								// port 0 read enable
			pixel_counter					// port 1 read address
		);
	} else {
		result = 0;
	}
	
	// Pixel Counter
	if (pixel_counter == 76800 - 1) { // 320 x 240
		pixel_counter = 0;
	} else if (is_active_drawing_area) {
		pixel_counter += 1;
	}
	
	return result;
}

// Top-level module
#pragma MAIN_MHZ uxn_eval 12.287999
uint16_t uxn_eval(uint16_t input) {
	static uint4_t input_code;
	static uint12_t palette_color_values[4] = {0xFFF, 0x000, 0x7DB, 0xF62};
	static uint32_t pixel_counter = 0; // 320x240, max = 76799
	static uint32_t clock_cycle_counter = 0;
	static uint32_t seconds_counter = 0;
	static uint16_t result;
	static uint2_t current_pixel_palette_color;
	static uint1_t is_active_drawing_area;
	input_code = (uint4_t)(input >> 12);
	
	if (input_code == 0x2) {
		is_active_drawing_area = input >> 2 & 0x0001;
	} 
	
	if (~is_active_drawing_area) {
		step_cpu();
	}
	
	current_pixel_palette_color = step_gpu(is_active_drawing_area, seconds_counter);
	result = (uint16_t)(palette_color_values[current_pixel_palette_color]);

	// Clock Counter
	if (clock_cycle_counter == (12287999 - 1)) { // If reached 1 second
		seconds_counter += 1;
		clock_cycle_counter = 0; // Reset counter
	} else {
		clock_cycle_counter += 1;
	}
	
	return result;
}