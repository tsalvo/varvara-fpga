#include "uintN_t.h"  // uintN_t types for any N
#include "intN_t.h"   // intN_t types for any N

#include "uxn_rom.h"
#include "uxn_opcodes.h"

// RULES:
// - cannot write to a global variable from more than one function (unless you use clock domain crossing)
// - no switch statements (C AST node cannot be parsed to logic)
// - only one return per function
// - no ++ or -- operators

// Build with Docker pipelinec image: 
// docker run -v $(pwd):/workdir pipelinec pipelinec uxn.c

// https://github.com/JulianKemmerer/PipelineC/wiki/Automatically-Generated-Functionality#rams

uint1_t step_boot() {
	static uint16_t ram_address = 0x0100; // begin copying to RAM just after the zeropage
	static uint8_t rom_index, rom_byte;
	static uint1_t boot_phase, boot_result;
	
	if (boot_phase == 0) {
		boot_result = 0;
		rom_byte = read_rom_byte(rom_index); // START
	}
	else if (boot_phase == 1) {
		rom_byte = read_rom_byte(rom_index); // DONE
		poke_ram(ram_address, rom_byte); // START
		rom_index += 1;
		ram_address += 1;
		boot_result = rom_index > 127;
	}
	
	boot_phase += 1;
	if (boot_phase > 1) {
		boot_phase = 0;
	}
	
	if (boot_result) {
		pc_set(0x0100);
	}
	
	return boot_result;
}

void step_cpu() {
	static uint16_t pc = 0x0100;
	static uint8_t ins;
	static uint4_t step_cpu_phase = 0x0;
	static uint1_t step_cpu_result;
	
	if (step_cpu_phase == 0x0) {
		pc = pc_get(); // DONE
		pc_add(1); // DONE
		ins = peek_ram(pc); // START
		step_cpu_result = 0;
	}
	else if (step_cpu_phase == 0x1) {
		ins = peek_ram(pc); // DONE
		step_cpu_result = eval_opcode_phased(0x0, ins);
	}
	else if (step_cpu_phase == 0x2) {
		step_cpu_result = eval_opcode_phased(0x1, ins);
	}
	else if (step_cpu_phase == 0x3) {
		step_cpu_result = eval_opcode_phased(0x2, ins);
	}
	else if (step_cpu_phase == 0x4) {
		step_cpu_result = eval_opcode_phased(0x3, ins);
	}
	else if (step_cpu_phase == 0x5) {
		step_cpu_result = eval_opcode_phased(0x4, ins);
	}
	else if (step_cpu_phase == 0x6) {
		step_cpu_result = eval_opcode_phased(0x5, ins);
	}
	else if (step_cpu_phase == 0x7) {
		step_cpu_result = eval_opcode_phased(0x6, ins);
	}
	else if (step_cpu_phase == 0x8) {
		step_cpu_result = eval_opcode_phased(0x7, ins);
	}
	else if (step_cpu_phase == 0x9) {
		step_cpu_result = eval_opcode_phased(0x8, ins);
	}
	else if (step_cpu_phase == 0xA) {
		step_cpu_result = eval_opcode_phased(0x9, ins);
	}
	else if (step_cpu_phase == 0xB) {
		step_cpu_result = eval_opcode_phased(0xA, ins);
	}
	else if (step_cpu_phase == 0xC) {
		step_cpu_result = eval_opcode_phased(0xB, ins);
	}
	else if (step_cpu_phase == 0xD) {
		step_cpu_result = eval_opcode_phased(0xC, ins);
	}
	else if (step_cpu_phase == 0xE) {
		step_cpu_result = eval_opcode_phased(0xD, ins);
	}
	else if (step_cpu_phase == 0xF) {
		step_cpu_result = eval_opcode_phased(0xE, ins);
	}
	
	if (step_cpu_result == 1) {
		step_cpu_phase = 0;
	} else {
		step_cpu_phase += 1;
	}
}

uint2_t step_gpu(uint1_t is_active_drawing_area) {
	static uint2_t step_gpu_result;
	static uint32_t pixel_counter; // 320x240, max = 76799
	
	step_gpu_result = is_active_drawing_area ? background_vram_update(
		0, 								// port 0 address
		0,								// port 0 write value
		0,								// port 0 write enable
		0,								// port 0 read enable
		pixel_counter					// port 1 read address
	) : 0;
	// Pixel Counter
	if (pixel_counter == 76800 - 1) { // 320 x 240
		pixel_counter = 0;
	} else if (is_active_drawing_area) {
		pixel_counter += 1;
	}
	
	return step_gpu_result;
}

// Top-level module
// 16-bit input message format:
// 0001 UDLR SSBA YXLR  Controls
// 0010 ---- ---- -PHV  (P)Visible Pixel, (H)HBLANK, (V)VBLANK
#pragma MAIN_MHZ uxn_eval 12.287999
uint16_t uxn_eval(uint16_t input) {
	static uint4_t input_code;
	static uint12_t palette_color_values[4] = {0xFFF, 0x000, 0x7DB, 0xF62};
	static uint16_t uxn_eval_result;
	static uint2_t current_pixel_palette_color = 0;
	static uint1_t is_active_drawing_area = 0, is_booted = 0;
	input_code = input >> 12;
	
	if (input_code == 0x2) {
		is_active_drawing_area = input >> 2 & 0x0001;
	} 
	
	if (is_booted) {
		if (!is_active_drawing_area) {
			step_cpu();
		}
	} else {
		is_booted = step_boot();
	}
	
	current_pixel_palette_color = step_gpu(is_active_drawing_area);
	uxn_eval_result = (uint16_t)(palette_color_values[current_pixel_palette_color]);
	
	return uxn_eval_result;
}