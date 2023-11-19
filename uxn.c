#include "uintN_t.h"  // uintN_t types for any N
#include "intN_t.h"   // intN_t types for any N

#include "roms/star.h"
#include "uxn_opcodes.h"
#include "uxn_ram_main.h"
#include "uxn_constants.h"

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
	static boot_step_result_t result = {0, 0, 0, 0xFF}; // why ram_address starts at "0xFF"? first ROM byte goes to RAM 0x0100, but keep the RAM address behind by 1 due to latency
	
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
	uint24_t vram_address;
	
	uint1_t is_device_ram_write;
	uint8_t device_ram_address;
	
	uint8_t u8_value;

} cpu_step_result_t;

cpu_step_result_t step_cpu(uint8_t previous_ram_read_value, uint8_t previous_device_ram_read, uint1_t use_vector, uint16_t vector) {
	static uint16_t pc = 0x0100;
	static uint8_t ins = 0;
	static uint8_t step_cpu_phase = 0;
	static uint1_t is_ins_done = 0, is_waiting = 0;
	static eval_opcode_result_t eval_opcode_result;
	static cpu_step_result_t cpu_step_result = {0, 0, 0, 0, 0, 0, 0, 0};
	
	pc = (use_vector & is_waiting) ? vector : pc;
	is_waiting = use_vector ? 0 : is_waiting;
	
	#if DEBUG
	printf("Step CPU phase = 0x%X, pc = 0x%X, is_waiting = 0x%X, use_vector = 0x%X, vector = 0x%X\n", step_cpu_phase, pc, is_waiting, use_vector, vector);
	#endif
	
	if (step_cpu_phase == 0) {
		is_ins_done = 0;
		cpu_step_result.ram_address = pc; // START
		cpu_step_result.is_ram_write = 0;
		cpu_step_result.is_vram_write = 0;
		cpu_step_result.is_device_ram_write = 0;
	}
	else if (step_cpu_phase == 1) {
		pc += 1;
	}
	else {
		ins = step_cpu_phase == 2 ? previous_ram_read_value : ins;
		eval_opcode_result = eval_opcode_phased(step_cpu_phase - 2, ins, pc, previous_ram_read_value, previous_device_ram_read);
		pc = eval_opcode_result.is_pc_updated ? eval_opcode_result.u16_value : pc;
		cpu_step_result.is_ram_write = eval_opcode_result.is_ram_write;
		cpu_step_result.ram_address = eval_opcode_result.u16_value;
		cpu_step_result.is_vram_write = eval_opcode_result.is_vram_write;
		cpu_step_result.vram_write_layer = eval_opcode_result.vram_write_layer;
		cpu_step_result.vram_address = eval_opcode_result.vram_address;
		cpu_step_result.device_ram_address = eval_opcode_result.device_ram_address;
		cpu_step_result.is_device_ram_write = eval_opcode_result.is_device_ram_write;
		cpu_step_result.u8_value = eval_opcode_result.u8_value;
		is_waiting = eval_opcode_result.is_waiting;
		is_ins_done = eval_opcode_result.is_opc_done;
	}
	
	if (is_ins_done | is_waiting) {
		step_cpu_phase = 0;
	} else {
		step_cpu_phase = (pc == 0) ? 0 : (step_cpu_phase + 1);  // stop if PC == 0
	}
	
	return cpu_step_result;
}

typedef struct gpu_step_result_t {
	uint2_t color;
	uint1_t is_active_fill;
	uint1_t is_new_frame;
} gpu_step_result_t;

gpu_step_result_t step_gpu(uint1_t is_active_drawing_area, uint1_t is_vram_write, uint1_t vram_write_layer, uint24_t vram_address, uint2_t vram_value) {
	static gpu_step_result_t result = {0, 0, 0};
	static uint4_t vram_code = 0;
	static uint24_t adjusted_vram_address;
	
	// fill
	static uint24_t fill_pixels_remaining;
	static uint16_t fill_x0, fill_y0, fill_x1, fill_y1;
	static uint2_t fill_color;
	static uint1_t is_fill_active, is_fill_left, is_fill_top, is_fill_pixel0, is_fill_pixel1, fill_layer, is_fill_code;
	
	static uint2_t fg_pixel_color, bg_pixel_color;
	static uint18_t pixel_counter = 0; // 400x360, max = 143999
	static uint16_t x, y;
	
	vram_code = (uint4_t)(vram_address >> 20);
	is_fill_code = vram_code == 0xF ? 1 : 0;
	
	// 0bCCCCTLPPPPPPPPPPPPPPPPPP (CCCC = VRAM Code, T = fill from top, L = fill from left, P = pixel number)
	if (is_fill_code & ~is_fill_active) {
		is_fill_active = 1;
		is_fill_top = vram_address >> 19;
		is_fill_left = vram_address >> 18;
		fill_pixels_remaining = vram_address & 0x03FFFF;
		fill_y0 = fill_pixels_remaining / 400;
		fill_x0 = fill_pixels_remaining - (fill_y0 * 400);
		fill_y1 = is_fill_top ? fill_y0 : 359;
		fill_x1 = is_fill_left ? fill_x0 : 399;
		fill_y0 = is_fill_top ? 0 : fill_y0;
		fill_x0 = is_fill_left ? 0 : fill_x0;
		fill_layer = vram_write_layer;
		fill_color = vram_value;
		fill_pixels_remaining = (fill_x1 - fill_x0) * (fill_y1 - fill_y0);
		y = fill_y0;
		x = fill_x0;
		#if DEBUG
		printf("NEW FILL: x0=0x%X y0=0x%X x1=0x%X y1=0x%X color=0x%X\n", fill_x0, fill_y0, fill_x1, fill_y1, fill_color);
		#endif
	}
	
	adjusted_vram_address = is_fill_active ? (((uint24_t)(y) * (uint24_t)(400)) + ((uint24_t)(x))) : (vram_address & 0x03FFFF);
	
	is_fill_left = (x == fill_x1) ? 1 : 0;
	y = is_fill_left ? (y + 1) : y;
	x = is_fill_left ? fill_x0 : x + 1;
	
	is_fill_pixel0 = is_fill_active & (~fill_layer);
	is_fill_pixel1 = is_fill_active & fill_layer;
	
	bg_pixel_color = bg_vram_update(
		pixel_counter,							                                           // read address
		adjusted_vram_address,                                                             // write address
		is_fill_pixel0 ? fill_color : vram_value,					                       // write value
		is_fill_pixel0 | (~is_fill_active & is_vram_write & (~vram_write_layer))		   // write enable
	);
	
	fg_pixel_color = fg_vram_update(
		pixel_counter,							                    					// read address
		adjusted_vram_address,                                                          // write address
		is_fill_pixel1 ? fill_color : vram_value,										// write value
		is_fill_pixel1 | (~is_fill_active & is_vram_write & vram_write_layer)		    // write enable
	);
	
	fill_pixels_remaining = is_fill_active ? fill_pixels_remaining - 1 : 0;
	is_fill_active = fill_pixels_remaining == 0 ? 0 : 1;
	pixel_counter = (pixel_counter == 143999) ? 0 : (is_active_drawing_area ? (pixel_counter + 1) : pixel_counter);
	result.is_new_frame = (pixel_counter == 143999) ? 1 : 0;
	result.color = fg_pixel_color == 0 ? bg_pixel_color : fg_pixel_color;
	result.is_active_fill = is_fill_active;

	return result;
}


uint16_t vector_snoop(uint8_t device_ram_address, uint8_t device_ram_value, uint1_t is_device_ram_write) {
	static uint16_t screen_vector = 0;
	
	if (is_device_ram_write) {
		if (device_ram_address == 0x20) {
			screen_vector &= 0x00FF;
			screen_vector |= ((uint16_t)(device_ram_value) << 8);
		}
		else if (device_ram_address == 0x21) {
			screen_vector &= 0xFF00;
			screen_vector |= ((uint16_t)(device_ram_value));
		}
	}
	
	return screen_vector;
}

uint16_t palette_snoop(uint8_t device_ram_address, uint8_t device_ram_value, uint1_t is_device_ram_write, uint2_t gpu_step_color) {
	static uint12_t color0 = 0xFFF;
	static uint12_t color1 = 0x000;
	static uint12_t color2 = 0x7DB;
	static uint12_t color3 = 0xF62;
	static uint4_t color_cmp_0 = 0, color_cmp_1 = 0;
	static uint12_t tmp12 = 0;
	static uint16_t result = 0;
	
	if (is_device_ram_write) {
		color_cmp_0 = (uint4_t)(device_ram_value >> 4);
		color_cmp_1 = (uint4_t)(device_ram_value);
		if (device_ram_address == 0x08) {
			tmp12 = color_cmp_0;
			tmp12 <<= 8;
			color0 &= 0x0FF;
			color0 |= tmp12;
			
			tmp12 = color_cmp_1;
			tmp12 <<= 8;
			color1 &= 0x0FF;
			color1 |= tmp12;
		}
		else if (device_ram_address == 0x09) {
			tmp12 = color_cmp_0;
			tmp12 <<= 8;
			color2 &= 0x0FF;
			color2 |= tmp12;
			
			tmp12 = color_cmp_1;
			tmp12 <<= 8;
			color3 &= 0x0FF;
			color3 |= tmp12;
		}
		else if (device_ram_address == 0x0A) {
			tmp12 = color_cmp_0;
			tmp12 <<= 4;
			color0 &= 0xF0F;
			color0 |= tmp12;
			
			tmp12 = color_cmp_1;
			tmp12 <<= 4;
			color1 &= 0xF0F;
			color1 |= tmp12;
		}
		else if (device_ram_address == 0x0B) {
			tmp12 = color_cmp_0;
			tmp12 <<= 4;
			color2 &= 0xF0F;
			color2 |= tmp12;
			
			tmp12 = color_cmp_1;
			tmp12 <<= 4;
			color3 &= 0xF0F;
			color3 |= tmp12;
		}
		else if (device_ram_address == 0x0C) {
			tmp12 = (uint12_t)color_cmp_0;
			color0 &= 0xFF0;
			color0 |= tmp12;
			
			tmp12 = (uint12_t)color_cmp_1;
			color1 &= 0xFF0;
			color1 |= tmp12;
		}
		else if (device_ram_address == 0x0D) {
			tmp12 = (uint12_t)color_cmp_0;
			color2 &= 0xFF0;
			color2 |= tmp12;
			
			tmp12 = (uint12_t)color_cmp_1;
			color3 &= 0xFF0;
			color3 |= tmp12;
		}
	}
	
	if (gpu_step_color == 0) {
		result = color0;
	} else if (gpu_step_color == 1) {
		result = color1;
	} else if (gpu_step_color == 2) {
		result = color2;
	} else {
		result = color3;
	}
	
	return result;
}

#pragma MAIN_MHZ uxn_top 14.746
uint16_t uxn_top(
	uint1_t is_visible_pixel,
	uint1_t rom_load_valid_byte,
	uint16_t rom_load_address,
	uint8_t rom_load_value
) {
	static uint24_t boot_check = 0;
	static uint16_t uxn_eval_result = 0;
	static uint1_t is_booted = 0;
	
	static gpu_step_result_t gpu_step_result;
	static cpu_step_result_t cpu_step_result;
	static uint1_t is_active_fill = 0;
	static uint1_t is_ram_write = 0;
	static uint16_t ram_address = 0x00FF;
	static uint16_t screen_vector = 0;
	static uint8_t ram_write_value = 0;
	static uint8_t ram_read_value = 0;
	static uint8_t device_ram_address = 0;
	static uint8_t device_ram_read_value = 0;
	static uint1_t is_device_ram_write = 0;
	static uint1_t is_vram_write = 0;
	static uint1_t vram_write_layer = 0;
	static uint24_t vram_address = 0;
	static uint2_t vram_value = 0;
	
	if (~is_booted) {
		boot_check = rom_load_valid_byte ? 0 : ((ram_address > 0x00FF) ? boot_check + 1 : 0);
		is_booted = (boot_check == 0xFFFFFF) ? 1 : 0;
		is_ram_write = (rom_load_valid_byte | is_booted);
		ram_address += (rom_load_valid_byte | is_booted) ? 1 : 0;
		ram_write_value = is_booted ? 0 : rom_load_value;
		
		// OLD (C-Array-Style)
		// boot_step_result_t boot_step_result = step_boot();
		// is_ram_write = boot_step_result.is_valid_byte;
		// ram_address = boot_step_result.ram_address;
		// ram_write_value = boot_step_result.rom_byte;
		// is_booted = boot_step_result.is_finished;
	} else if (~is_active_fill) {
		cpu_step_result = step_cpu(ram_read_value, device_ram_read_value, gpu_step_result.is_new_frame, screen_vector);
		is_ram_write = cpu_step_result.is_ram_write;
		ram_address = cpu_step_result.ram_address;
		device_ram_address = cpu_step_result.device_ram_address;
		is_device_ram_write = cpu_step_result.is_device_ram_write;
		ram_write_value = cpu_step_result.u8_value;
		is_vram_write = cpu_step_result.is_vram_write;
		vram_write_layer = cpu_step_result.vram_write_layer;
		vram_address = cpu_step_result.vram_address;
		vram_value = (uint2_t)cpu_step_result.u8_value;
	}
	
	ram_read_value = main_ram_update(
		ram_address,
		ram_write_value, // shared register, write only to either ram or device ram in one cycle
		is_ram_write
	);
	
	device_ram_read_value = device_ram_update(
		device_ram_address,
		ram_write_value, // shared register, write only to either ram or device ram in one cycle
		is_device_ram_write
	);
	
	gpu_step_result = step_gpu(is_visible_pixel, is_vram_write, vram_write_layer, vram_address, vram_value);
	is_active_fill = gpu_step_result.is_active_fill;
	uxn_eval_result = palette_snoop(device_ram_address, ram_write_value, is_device_ram_write, gpu_step_result.color);
	screen_vector = vector_snoop(device_ram_address, ram_write_value, is_device_ram_write);
	
	return uxn_eval_result;
}