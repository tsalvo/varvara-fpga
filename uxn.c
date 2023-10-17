#include "uintN_t.h"  // uintN_t types for any N
#include "intN_t.h"   // intN_t types for any N

#include "roms/mandelbrot_fast.h"
#include "uxn_opcodes.h"
#include "uxn_ram_main.h"

// RULES:
// - cannot write to a global variable from more than one function (unless you use clock domain crossing)
// - no switch statements (C AST node cannot be parsed to logic)
// - only one return per function
// - no ++ or -- operators

// Build with Docker pipelinec image: 
// docker run -v $(pwd):/workdir pipelinec pipelinec uxn.c

// note about RAMs
// https://github.com/JulianKemmerer/PipelineC/wiki/Automatically-Generated-Functionality#rams

typedef struct boot_step_result_t {
	uint1_t is_valid_byte;  // is the most recent byte valid?
	uint1_t is_finished; 	// is the last byte read?
	uint8_t rom_byte;  		// the most recent byte read
	uint16_t ram_address; 	// RAM address to copy byte to (if valid)
} boot_step_result_t;

boot_step_result_t step_boot() {
	static uint1_t boot_phase = 0;
	static uint16_t rom_address = 0;
	static boot_step_result_t result = {0, 0, 0, 0xFE}; // why ram_address starts at "0xFE"? first ROM byte goes to RAM 0x0100, but keep the RAM address behind by 1 due to latency, and 1 more behind due to early ram_address incrementing
	
	result.rom_byte = read_rom_byte(rom_address);
	rom_address += boot_phase; // increase when boot phase is 1, not when it's zero (allow for two reads)
	result.ram_address += boot_phase;
	result.is_finished = boot_phase == 0 ? 0 : (rom_address > (ROM_SIZE - 1) ? 1 : 0);
	result.is_valid_byte = boot_phase;
	boot_phase += 1;
	
	return result;
}

typedef struct cpu_step_result_t {
	uint1_t is_ram_write;
	uint16_t ram_address;
	
	uint1_t is_vram_write;
	uint1_t vram_write_layer;
	uint32_t vram_address;
	
	uint8_t u8_value;

} cpu_step_result_t;

cpu_step_result_t step_cpu(uint8_t ram_read_value) {
	static uint16_t pc = 0x0100;
	static uint8_t ins = 0;
	static uint8_t step_cpu_phase = 0;
	static uint1_t is_ins_done = 0;
	static eval_opcode_result_t eval_opcode_result;
	static cpu_step_result_t cpu_step_result = {0, 0, 0, 0, 0, 0};
	if (step_cpu_phase == 0) {
		is_ins_done = 0;
		cpu_step_result.ram_address = pc; // START
		cpu_step_result.is_ram_write = 0;
		cpu_step_result.is_vram_write = 0;
	}
	else if (step_cpu_phase == 1) {
		pc += 1;
	}
	else {
		ins = step_cpu_phase == 2 ? ram_read_value : ins;
		eval_opcode_result = eval_opcode_phased(step_cpu_phase - 2, ins, pc, ram_read_value);
		pc = eval_opcode_result.is_pc_updated ? eval_opcode_result.u16_value : pc;
		cpu_step_result.is_ram_write = eval_opcode_result.is_ram_write;
		cpu_step_result.ram_address = eval_opcode_result.u16_value;
		cpu_step_result.is_vram_write = eval_opcode_result.is_vram_write;
		cpu_step_result.vram_write_layer = eval_opcode_result.vram_write_layer;
		cpu_step_result.vram_address = eval_opcode_result.vram_address;
		cpu_step_result.u8_value = eval_opcode_result.u8_value;
		is_ins_done = eval_opcode_result.is_opc_done;
	}
	
	if (is_ins_done == 1) {
		step_cpu_phase = 0;
	} else {
		step_cpu_phase = (pc == 0) ? 0 : (step_cpu_phase + 1);  // stop if PC == 0
	}
	
	return cpu_step_result;
}

typedef struct gpu_step_result_t {
	uint2_t color;
	uint1_t is_active_fill;
} gpu_step_result_t;

gpu_step_result_t step_gpu(uint1_t is_active_drawing_area, uint1_t is_vram_write, uint1_t vram_write_layer, uint32_t vram_address, uint2_t vram_value) {
	static gpu_step_result_t result = {0, 0};
	static uint4_t vram_code = 0;
	static uint32_t adjusted_vram_address;
	
	// fill
	static uint32_t fill_pixels_remaining;
	static uint16_t fill_x0, fill_y0, fill_x1, fill_y1;
	static uint2_t fill_color;
	static uint1_t is_fill_active, is_fill_left, is_fill_top, is_fill_pixel, is_fill_pixel0, is_fill_pixel1, fill_layer, fill_isect_l, fill_isect_r, fill_isect_t, fill_isect_b, is_fill_code;
	
	static uint2_t fg_pixel_color, bg_pixel_color;
	static uint32_t pixel_counter = 0; // 400x360, max = 143999
	static uint16_t x, y;
	
	adjusted_vram_address = vram_address > 143999 ? 0 : vram_address;
	vram_code = (uint4_t)(vram_address >> 28);
	is_fill_code = vram_code == 0xF ? 1 : 0;
	y = pixel_counter / 400;
	x = pixel_counter - (y * 400);
	
	// 0b11110000 00TL00PP PPPPPPPP PPPPPPPP (P = pixel number)
	if (is_fill_code & ~is_fill_active) {
		is_fill_active = 1;
		is_fill_left = vram_address >> 20;
		is_fill_top = vram_address >> 21;
		fill_pixels_remaining = vram_address & 0x0003FFFF;
		fill_y0 = fill_pixels_remaining / 400;
		fill_x0 = fill_pixels_remaining - (fill_y0 * 400);
		fill_y1 = is_fill_top ? fill_y0 : 359;
		fill_x1 = is_fill_left ? fill_x0 : 399;
		fill_y0 = is_fill_top ? 0 : fill_y0;
		fill_x0 = is_fill_left ? 0 : fill_x0;
		fill_layer = vram_write_layer;
		fill_color = vram_value;
		fill_pixels_remaining = 143999;
	}
	
	is_fill_active = fill_pixels_remaining == 0 ? 0 : 1;
	
	fill_isect_l = x > fill_x0;
	fill_isect_r = x < fill_x1;
	fill_isect_t = y > fill_y0;
	fill_isect_b = y < fill_y1;
	
	is_fill_pixel = is_fill_active & fill_isect_l & fill_isect_r & fill_isect_t & fill_isect_b;
	is_fill_pixel0 = is_fill_pixel & (~fill_layer);
	is_fill_pixel1 = is_fill_pixel & fill_layer;
	
	bg_pixel_color = bg_vram_update(
		pixel_counter,							                                           // read address
		is_fill_pixel0 ? pixel_counter : (is_fill_active ? 0 : adjusted_vram_address),     // write address
		is_fill_pixel0 ? fill_color : vram_value,					                       // write value
		is_fill_pixel0 | (~is_fill_active & is_vram_write & (~vram_write_layer))		   // write enable
	);
	
	fg_pixel_color = fg_vram_update(
		pixel_counter,							                    					// read address
		is_fill_pixel1 ? pixel_counter : (is_fill_active ? 0 : adjusted_vram_address),  // write address
		is_fill_pixel1 ? fill_color : vram_value,										// write value
		is_fill_pixel1 | (~is_fill_active & is_vram_write & vram_write_layer)		    // write enable
	);

	// Pixel Counter
	if (pixel_counter == 143999) { // 400x360
		pixel_counter = 0;
	} else if (is_active_drawing_area) {
		pixel_counter += 1;
		fill_pixels_remaining = fill_pixels_remaining == 0 ? 0 : fill_pixels_remaining - 1;
	}
		
	result.color = fg_pixel_color == 0 ? bg_pixel_color : fg_pixel_color;
	result.is_active_fill = is_fill_active;

	return result;
}

// Top-level module
// 16-bit input message format:
// 0001 UDLR SSBA YXLR  Controls
// 0010 ---- ---- -PHV  (P)Visible Pixel, (H)HBLANK, (V)VBLANK
#pragma MAIN_MHZ uxn_eval 18.0
uint16_t uxn_eval(uint16_t input) {
	static uint32_t main_clock_cycle = 0;
	static uint4_t input_code;
	static uint12_t palette_color_values[4] = {0xFFF, 0x000, 0x7DB, 0xF62};
	static uint16_t uxn_eval_result = 0;
	static uint2_t current_pixel_palette_color = 0;
	static uint1_t is_active_drawing_area = 0, is_booted = 0;
	
	static uint1_t is_active_fill = 0;
	static uint1_t is_ram_write = 0;
	static uint16_t ram_address = 0;
	static uint8_t ram_write_value = 0;
	static uint8_t ram_read_value = 0;
	static uint1_t is_vram_write = 0;
	static uint1_t vram_write_layer = 0;
	static uint32_t vram_address = 0;
	static uint2_t vram_value = 0;
	input_code = input >> 12;
		
	if (input_code == 0x2) {
		is_active_drawing_area = input >> 2 & 0x0001;
	} 
	
	if (~is_booted) {
		boot_step_result_t boot_step_result = step_boot();
		is_ram_write = boot_step_result.is_valid_byte;
		ram_address = boot_step_result.ram_address;
		ram_write_value = boot_step_result.rom_byte;
		is_booted = boot_step_result.is_finished;
	} else if (~is_active_fill) {
		cpu_step_result_t cpu_step_result = step_cpu(ram_read_value);
		is_ram_write = cpu_step_result.is_ram_write;
		ram_address = cpu_step_result.ram_address;
		ram_write_value = cpu_step_result.u8_value;
		is_vram_write = cpu_step_result.is_vram_write;
		vram_write_layer = cpu_step_result.vram_write_layer;
		vram_address = cpu_step_result.vram_address;
		vram_value = (uint2_t)cpu_step_result.u8_value;
	}
	
	ram_read_value = main_ram_update(
		ram_address,
		ram_write_value,
		is_ram_write
	);
	
	gpu_step_result_t gpu_step_result = step_gpu(is_active_drawing_area, is_vram_write, vram_write_layer, vram_address, vram_value);
	is_active_fill = gpu_step_result.is_active_fill;
	uxn_eval_result = (uint16_t)(palette_color_values[gpu_step_result.color]);
	main_clock_cycle += 1;
	
	return uxn_eval_result;
}