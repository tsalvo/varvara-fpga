#include "uintN_t.h"  // uintN_t types for any N
#include "intN_t.h"   // intN_t types for any N

#include "uxn_opcodes.h"
#include "uxn_ram_main.h"
#include "uxn_constants.h"
#include <stdint.h>

#if DEBUG
#include "roms/screen_blending.h"
#endif

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
	uint16_t u16_addr; // ram address read / write, or vram address write
	
	uint1_t is_vram_write;
	uint1_t vram_write_layer;
	
	uint1_t is_device_ram_write;
	uint8_t device_ram_address;
	
	uint1_t swap_buffers;
	
	uint8_t u8_value;

} cpu_step_result_t;

cpu_step_result_t step_cpu(uint8_t previous_ram_read_value, uint8_t previous_device_ram_read, uint8_t controller0_buttons, uint1_t is_new_frame, uint16_t screen_vector, uint16_t controller_vector) {
	static uint16_t pc = 0x0100;
	static uint8_t ins = 0;
	static uint12_t step_cpu_phase = 0;
	static uint1_t is_ins_done = 0, is_waiting = 0, pending_controller = 0, pending_frame = 0;
	static uint8_t last_controller0 = 0;
	static cpu_step_result_t cpu_step_result = {0, 0, 0, 0, 0, 0, 0, 0};
	
	if (controller0_buttons != last_controller0 && controller_vector(15, 8) != 0) {
		pending_controller = 1;
	}
	
	if (is_new_frame & (screen_vector(15, 8) == 0 ? 0 : 1)) {
		pending_frame = 1;
	}
	
	cpu_step_result.swap_buffers = pending_frame & is_waiting;
	pc = is_waiting ? (pending_frame ? screen_vector : (pending_controller ? controller_vector : pc)) : pc;
	is_waiting = pending_frame | pending_controller ? 0 : is_waiting;
	pending_controller = pc == controller_vector ? 0 : pending_controller;
	pending_frame = 0;
	
	last_controller0 = controller0_buttons;
	
	if (step_cpu_phase == 0) {
		is_ins_done = 0;
		cpu_step_result.u16_addr = pc; // START
		cpu_step_result.is_ram_write = 0;
		cpu_step_result.is_vram_write = 0;
		cpu_step_result.is_device_ram_write = 0;
	}
	else if (step_cpu_phase == 1) {
		pc += 1;
	}
	else {
		ins = step_cpu_phase == 2 ? previous_ram_read_value : ins;
		eval_opcode_result_t eval_opcode_result = eval_opcode_phased(step_cpu_phase - 2, ins, pc, controller0_buttons, previous_ram_read_value, previous_device_ram_read);
		pc = eval_opcode_result.is_pc_updated ? eval_opcode_result.u16_value : pc;
		cpu_step_result.is_ram_write = eval_opcode_result.is_ram_write;
		cpu_step_result.u16_addr = eval_opcode_result.u16_value;
		cpu_step_result.is_vram_write = eval_opcode_result.is_vram_write;
		cpu_step_result.vram_write_layer = eval_opcode_result.vram_write_layer;
		cpu_step_result.device_ram_address = eval_opcode_result.device_ram_address;
		cpu_step_result.is_device_ram_write = eval_opcode_result.is_device_ram_write;
		cpu_step_result.u8_value = eval_opcode_result.u8_value;
		is_waiting = eval_opcode_result.is_waiting;
		is_ins_done = eval_opcode_result.is_opc_done;
	}
	
	if (is_ins_done | is_waiting) {
		step_cpu_phase = 0;
	} else {
		step_cpu_phase = (pc(15, 8) == 0) ? 0 : (step_cpu_phase + 1);  // stop if PC == 0
	}
	
	return cpu_step_result;
}

typedef struct gpu_step_result_t {
	uint2_t color;
	uint1_t is_new_frame;
} gpu_step_result_t;

typedef struct draw_command_t {
	uint16_t vram_address;
	uint2_t color;
	uint1_t layer;
	uint1_t is_fill;
	uint1_t fill_left;
	uint1_t fill_top;
	uint1_t is_valid;
} draw_command_t;

gpu_step_result_t step_gpu(uint1_t is_active_drawing_area, uint1_t is_vram_write, uint1_t vram_write_layer, uint16_t vram_address, uint8_t vram_value, uint32_t cycle, uint1_t enable_buffer_swap, uint1_t swap_buffers) {
	static gpu_step_result_t result = {0, 0};
	static uint10_t queue_read_ptr = 0;
	static uint10_t queue_write_ptr = 0;
	static draw_command_t current_queue_item = {0, 0, 0, 0, 0, 0, 0};
	static uint24_t queue_write_value = 0;
	static uint24_t queue_read_value = 0;
	static uint1_t queue_write_enable = 0;
	static uint2_t queue_phase = 0;
	static uint1_t is_buffer_swapped = 0;
	
	// current fill
	static uint16_t fill_x0, fill_y0, fill_x1, fill_y1;
	static uint2_t fill_color;
	static uint1_t is_new_fill_row, is_last_fill_col, is_fill_active, fill_layer, is_fill_top, is_fill_left;
	
	static uint16_t pixel_counter = 0; // 256*240, max = 61439
	static uint16_t x, y;
	
	static uint1_t is_caught_up = 0, is_read_ready = 0;
	
	is_buffer_swapped ^= swap_buffers;
	is_caught_up = queue_read_ptr == queue_write_ptr ? 1 : 0;
	is_read_ready = queue_phase == 3 ? 1 : 0;

	if (~current_queue_item.is_valid & ~is_caught_up & is_read_ready) { // ready for next item
		current_queue_item.vram_address = queue_read_value(15, 0);
		current_queue_item.color = queue_read_value(17, 16);
		current_queue_item.fill_top = queue_read_value(18);
		current_queue_item.fill_left = queue_read_value(19);
		current_queue_item.is_fill = queue_read_value(20);
		current_queue_item.layer = queue_read_value(21);
		current_queue_item.is_valid = 1;
		queue_phase = 0;
		queue_read_ptr += 1;
	}

	if (is_vram_write) { // queue up new draw command (if given)
		queue_write_value = uint32_uint16_0(0, vram_address);
		queue_write_value = uint32_uint5_16(queue_write_value, vram_value); // 0b000FLTCC (F = Fill, L = Left, T = Top, C = Color)
		queue_write_value = uint32_uint1_21(queue_write_value, vram_write_layer);
		queue_phase = 0;
		queue_write_ptr += 1;
	}
	
	queue_write_enable = is_vram_write;
	queue_phase = queue_phase == 3 ? 3 : queue_phase + 1;

	if (current_queue_item.is_valid & current_queue_item.is_fill & ~is_fill_active) {
		is_fill_active = 1;
		is_fill_top = current_queue_item.fill_top;
		is_fill_left = current_queue_item.fill_left;
		fill_y1 = is_fill_top ? current_queue_item.vram_address >> 8 : 239;
		fill_x1 = is_fill_left ? current_queue_item.vram_address & 0x00FF : 255;
		fill_y0 = is_fill_top ? 0 : current_queue_item.vram_address >> 8;
		fill_x0 = is_fill_left ? 0 : current_queue_item.vram_address & 0x00FF;
		fill_layer = current_queue_item.layer;
		fill_color = current_queue_item.color;
		is_new_fill_row = 0;
		is_last_fill_col = 0;
		y = fill_y0;
		x = fill_x0;
	} 
	
	uint17_t adjusted_read_address = pixel_counter | (is_buffer_swapped & enable_buffer_swap ? 0b10000000000000000 : 0);
	uint17_t adjusted_write_address = (is_fill_active ? ((y << 8) + x) : current_queue_item.vram_address) | (~is_buffer_swapped & enable_buffer_swap ? 0b10000000000000000 : 0);
	
	is_new_fill_row = (x == fill_x1) ? 1 : 0;
	is_last_fill_col = (y == fill_y1) ? 1 : 0;
	y = is_new_fill_row ? (y + 1) : y;
	x = is_new_fill_row ? fill_x0 : x + 1;
	
	uint1_t is_fill_pixel0 = is_fill_active & (~fill_layer);
	uint1_t is_fill_pixel1 = is_fill_active & fill_layer;
	
	uint2_t bg_pixel_color = bg_vram_update(
		adjusted_read_address,							                                           // read address
		adjusted_write_address,                                                             // write address
		is_fill_pixel0 ? fill_color : current_queue_item.color,					                       // write value
		is_fill_pixel0 | (~is_fill_active & current_queue_item.is_valid & (~current_queue_item.layer))		   // write enable
	);
	
	uint2_t fg_pixel_color = fg_vram_update(
		adjusted_read_address,							                    					// read address
		adjusted_write_address,                                                          // write address
		is_fill_pixel1 ? fill_color : current_queue_item.color,										// write value
		is_fill_pixel1 | (~is_fill_active & current_queue_item.is_valid & current_queue_item.layer)		    // write enable
	);
	
	queue_read_value = draw_queue_update(
		queue_read_ptr,			// read address
		queue_write_ptr,		// write address
		queue_write_value,		// write value
		queue_write_enable		// write enable
	);
	
	is_fill_active = is_fill_active ? ~(is_new_fill_row & is_last_fill_col) : 0;
	current_queue_item.is_valid = is_fill_active;
	pixel_counter = (pixel_counter == 61439) ? 0 : (is_active_drawing_area ? (pixel_counter + 1) : pixel_counter);
	result.is_new_frame = (pixel_counter == 61439) ? 1 : 0;
	result.color = fg_pixel_color == 0 ? bg_pixel_color : fg_pixel_color;

	return result;
}

typedef struct vector_snoop_result_t {
	uint16_t screen;
	uint16_t controller;
} vector_snoop_result_t;

vector_snoop_result_t vector_snoop(uint8_t device_ram_address, uint8_t device_ram_value, uint1_t is_device_ram_write) {
	static vector_snoop_result_t vectors = {0, 0};
	
	if (is_device_ram_write) {
		if (device_ram_address == 0x20) {
			vectors.screen &= 0x00FF;
			vectors.screen |= ((uint16_t)(device_ram_value) << 8);
		} else if (device_ram_address == 0x21) {
			vectors.screen &= 0xFF00;
			vectors.screen |= ((uint16_t)(device_ram_value));
		} else if (device_ram_address == 0x80) {
			vectors.controller &= 0x00FF;
			vectors.controller |= ((uint16_t)(device_ram_value) << 8);
		} else if (device_ram_address == 0x81) {
			vectors.controller &= 0xFF00;
			vectors.controller |= ((uint16_t)(device_ram_value));
		}
	}
	
	return vectors;
}

uint16_t palette_snoop(uint8_t device_ram_address, uint8_t device_ram_value, uint1_t is_device_ram_write, uint2_t gpu_step_color) {
	static uint12_t color[4] = {0xFFF, 0x000, 0x7DB, 0xF62};
	uint1_t is_palette_range = (device_ram_address >> 4) == 0 ? 1 : 0;
	
	if (is_device_ram_write & is_palette_range) {
		uint12_t tmp12;
		uint4_t addr_low = (uint4_t)device_ram_address;
		uint4_t color_cmp_0 = (uint4_t)(device_ram_value >> 4);
		uint4_t color_cmp_1 = (uint4_t)device_ram_value;
		
		if (addr_low == 0x8) {
			tmp12 = color_cmp_0;
			tmp12 <<= 8;
			color[0] &= 0x0FF;
			color[0] |= tmp12;
			
			tmp12 = color_cmp_1;
			tmp12 <<= 8;
			color[1] &= 0x0FF;
			color[1] |= tmp12;
		}
		else if (addr_low == 0x9) {
			tmp12 = color_cmp_0;
			tmp12 <<= 8;
			color[2] &= 0x0FF;
			color[2] |= tmp12;
			
			tmp12 = color_cmp_1;
			tmp12 <<= 8;
			color[3] &= 0x0FF;
			color[3] |= tmp12;
		}
		else if (addr_low == 0xA) {
			tmp12 = color_cmp_0;
			tmp12 <<= 4;
			color[0] &= 0xF0F;
			color[0] |= tmp12;
			
			tmp12 = color_cmp_1;
			tmp12 <<= 4;
			color[1] &= 0xF0F;
			color[1] |= tmp12;
		}
		else if (addr_low == 0xB) {
			tmp12 = color_cmp_0;
			tmp12 <<= 4;
			color[2] &= 0xF0F;
			color[2] |= tmp12;
			
			tmp12 = color_cmp_1;
			tmp12 <<= 4;
			color[3] &= 0xF0F;
			color[3] |= tmp12;
		}
		else if (addr_low == 0xC) {
			tmp12 = (uint12_t)color_cmp_0;
			color[0] &= 0xFF0;
			color[0] |= tmp12;
			
			tmp12 = (uint12_t)color_cmp_1;
			color[1] &= 0xFF0;
			color[1] |= tmp12;
		}
		else if (addr_low == 0xD) {
			tmp12 = (uint12_t)color_cmp_0;
			color[2] &= 0xFF0;
			color[2] |= tmp12;
			
			tmp12 = (uint12_t)color_cmp_1;
			color[3] &= 0xFF0;
			color[3] |= tmp12;
		}
	}
	
	uint16_t result = color[gpu_step_color];
	return result;
}

// #pragma PART "5CGXFC9E7F35C8" // TODO: try quartus step here for Cyclone V
#pragma MAIN uxn_top
uint16_t uxn_top(
	uint8_t controller0_buttons,
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
	static uint1_t is_ram_write = 0;
	static uint16_t u16_addr = 0x00FF; // ram address, or occasionally vram write addr
	static vector_snoop_result_t vectors = {0, 0};
	static uint8_t ram_write_value = 0;
	static uint8_t ram_read_value = 0;
	static uint8_t device_ram_address = 0;
	static uint8_t device_ram_read_value = 0;
	static uint1_t is_device_ram_write = 0;
	static uint1_t is_vram_write = 0;
	static uint1_t vram_write_layer = 0;
	static uint8_t vram_value = 0;
	
	static uint32_t cycle_count = 0;
	
	if (~is_booted) {
		#if DEBUG
		// (C-Array-Style)
		boot_step_result_t boot_step_result = step_boot();
		is_ram_write = boot_step_result.is_valid_byte;
		u16_addr = boot_step_result.ram_address;
		ram_write_value = boot_step_result.rom_byte;
		is_booted = boot_step_result.is_finished;
		#else
		boot_check = rom_load_valid_byte ? 0 : boot_check + 1;
		is_ram_write = rom_load_valid_byte;
		u16_addr = rom_load_address + 0x0100;
		ram_write_value = rom_load_value;
		is_booted = boot_check == 0xFFFFFF ? 1 : 0;
		#endif
	} else {
		cpu_step_result = step_cpu(ram_read_value, device_ram_read_value, controller0_buttons, gpu_step_result.is_new_frame, vectors.screen, vectors.controller);
		is_ram_write = cpu_step_result.is_ram_write;
		u16_addr = cpu_step_result.u16_addr;
		device_ram_address = cpu_step_result.device_ram_address;
		is_device_ram_write = cpu_step_result.is_device_ram_write;
		ram_write_value = cpu_step_result.u8_value;
		is_vram_write = cpu_step_result.is_vram_write;
		vram_write_layer = cpu_step_result.vram_write_layer;
		vram_value = cpu_step_result.u8_value;
	}
	
	ram_read_value = main_ram_update(
		u16_addr,
		ram_write_value, // shared register, write only to either ram or device ram in one cycle
		is_ram_write
	);
	
	device_ram_read_value = device_ram_update(
		device_ram_address,
		ram_write_value, // shared register, write only to either ram or device ram in one cycle
		is_device_ram_write
	);
	
	gpu_step_result = step_gpu(is_visible_pixel, is_vram_write, vram_write_layer, u16_addr, vram_value, cycle_count, vectors.screen == 0 ? 0 : 1, cpu_step_result.swap_buffers);
	uxn_eval_result = palette_snoop(device_ram_address, ram_write_value, is_device_ram_write, gpu_step_result.color);
	vectors = vector_snoop(device_ram_address, ram_write_value, is_device_ram_write);
	
	cycle_count += 1;
	
	return uxn_eval_result;
}