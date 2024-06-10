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
	
	uint1_t is_waiting;
	
	uint8_t u8_value;

} cpu_step_result_t;

cpu_step_result_t step_cpu(uint8_t previous_ram_read_value, uint8_t previous_device_ram_read, uint32_t time, uint8_t controller0_buttons, uint1_t is_new_frame, uint1_t has_screen_vector, uint1_t has_controller_vector, uint16_t screen_vector, uint16_t controller_vector) {
	static uint16_t pc = 0x0100;
	static uint8_t ins = 0;
	static uint12_t step_cpu_phase = 0;
	static uint1_t is_ins_done = 0, is_waiting = 0, pending_frame = 0, pending_controller = 0;
	static uint8_t last_controller0 = 0;
	static cpu_step_result_t cpu_step_result = {0, 0, 0, 0, 0, 0, 0, 0};
	
	if (has_controller_vector & (controller0_buttons != last_controller0 ? 1 : 0)) {
		pending_controller = 1;
	}
	
	pending_frame = is_new_frame & has_screen_vector;
	pc = is_waiting ? (pending_frame ? screen_vector : (pending_controller ? controller_vector : pc)) : pc;
	is_waiting = pending_frame | pending_controller ? 0 : is_waiting;
	pending_controller = pc == controller_vector ? 0 : pending_controller;
	
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
		eval_opcode_result_t eval_opcode_result = eval_opcode_phased(step_cpu_phase - 2, ins, pc, controller0_buttons, time, previous_ram_read_value, previous_device_ram_read);
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
	
	cpu_step_result.is_waiting = is_waiting;
	
	return cpu_step_result;
}

typedef struct draw_command_t {
	uint16_t vram_address;
	uint2_t color;
	uint1_t layer;
	uint1_t is_fill;
	uint1_t fill_left;
	uint1_t fill_top;
	uint1_t is_valid;
} draw_command_t;

uint2_t step_gpu(
	uint1_t is_active_drawing_area, 
	uint1_t is_vram_write, 
	uint1_t vram_write_layer, 
	uint16_t vram_address, 
	uint8_t vram_value, 
	uint1_t has_screen_vector, 
	uint1_t is_cpu_waiting,
	uint1_t vsync, // cycle 0 of every frame (new frame)
	uint1_t hsync // cycle 3 of every horizontal line (new line)
) {
	static uint2_t gpu_color = 0;
	static uint15_t queue_read_ptr = 0;
	static uint15_t queue_write_ptr = 0;
	static draw_command_t current_queue_item = {0, 0, 0, 0, 0, 0, 0};
	static uint24_t queue_write_value = 0;
	static uint24_t queue_read_value = 0;
	static uint1_t queue_write_enable = 0;
	static uint2_t queue_phase = 0, bg_pixel_color = 0, fg_pixel_color = 0, adjusted_write_value_bg = 0, adjusted_write_value_fg = 0;
	static uint1_t adjusted_write_enable_bg = 0, adjusted_write_enable_fg = 0;
	static uint17_t adjusted_read_address = 0, adjusted_write_address = 0;
	
	// current fill
	static uint8_t fill_x0, fill_y0, fill_x1, fill_y1, x, y;
	static uint2_t fill_color;
	static uint1_t is_new_fill_row, is_last_fill_col, is_fill_active, fill_layer, is_fill_top, is_fill_left, is_fill_pixel0, is_fill_pixel1;
	static uint16_t pixel_counter = 0; // 256*256, max = 65535, visible pixels only
	static uint20_t cycle_counter = 0; // max 1024x1024, includes all pixels
	static uint20_t buffer_swap_begin_cycle = 0; // cycle at which we begin swapping buffers
	static uint16_t buffer_swap_cycle = 0;
	static uint16_t tmp16 = 0;
	static uint1_t is_caught_up = 0, is_read_ready = 0, is_copy_phase = 0, is_copy_start_cycle = 0, can_swap_buffers = 0;
	
	is_caught_up = queue_read_ptr == queue_write_ptr ? 1 : 0;
	is_read_ready = queue_phase == 2 ? 1 : 0;

	if (~current_queue_item.is_valid & ~is_caught_up & is_read_ready & ~is_copy_phase) { // ready for next item
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
		queue_write_value = uint24_uint16_0(0, vram_address);
		queue_write_value = uint24_uint5_16(queue_write_value, vram_value(4, 0)); // 0b000FLTCC (F = Fill, L = Left, T = Top, C = Color)
		queue_write_value = uint24_uint1_21(queue_write_value, vram_write_layer);
		queue_phase = is_caught_up ? 0 : queue_phase;
		queue_write_ptr += 1;
	}
	
	queue_write_enable = is_vram_write;
	queue_phase = queue_phase == 2 ? 2 : queue_phase + 1;
	tmp16 = current_queue_item.vram_address;
	
	if (current_queue_item.is_valid & current_queue_item.is_fill & ~is_fill_active & ~is_copy_phase) {
		is_fill_top = current_queue_item.fill_top;
		is_fill_left = current_queue_item.fill_left;
		fill_y1 = is_fill_top ? tmp16(15, 8) : 255;
		fill_x1 = is_fill_left ? tmp16(7, 0) : 255;
		fill_y0 = is_fill_top ? 0 : tmp16(15, 8);
		fill_x0 = is_fill_left ? 0 : tmp16(7, 0);
		fill_layer = current_queue_item.layer;
		fill_color = current_queue_item.color;
		is_new_fill_row = 0;
		is_last_fill_col = 0;
		y = fill_y0;
		x = fill_x0;
		is_fill_active = 1;
	} else if (current_queue_item.is_valid & ~current_queue_item.is_fill & ~is_copy_phase) {
		y = tmp16(15, 8);
		x = tmp16(7, 0);
	}
	
	// READ: from upper buffer if copy phase, lower buffer otherwise
	adjusted_read_address = uint17_uint16_0(0, is_copy_phase ? buffer_swap_cycle : pixel_counter);
	adjusted_read_address = uint17_uint1_16(adjusted_read_address, is_copy_phase);
	
	// WRITE: to lower buffer if copy phase, upper buffer otherwise
	adjusted_write_address = is_copy_phase ? uint17_uint16_0(0, buffer_swap_cycle - 2) : uint17_uint8_8(0, y);
	adjusted_write_address = is_copy_phase ? adjusted_write_address : uint17_uint8_0(adjusted_write_address, x);
	adjusted_write_address = uint17_uint1_16(adjusted_write_address, ~is_copy_phase);
	
	is_new_fill_row = (x == fill_x1) ? 1 : 0;
	is_last_fill_col = (y == fill_y1) ? 1 : 0;
	y = (is_new_fill_row & ~is_copy_phase) ? (y + 1) : y;
	x = is_copy_phase ? x : (is_new_fill_row ? fill_x0 : x + 1);
	is_fill_pixel0 = is_fill_active & (~fill_layer);
	is_fill_pixel1 = is_fill_active & fill_layer;
	
	adjusted_write_value_bg = is_copy_phase ? bg_pixel_color : (is_fill_pixel0 ? fill_color : current_queue_item.color);
	adjusted_write_value_fg = is_copy_phase ? fg_pixel_color : (is_fill_pixel1 ? fill_color : current_queue_item.color);
	
	adjusted_write_enable_bg = is_copy_phase | is_fill_pixel0 | (~is_fill_active & current_queue_item.is_valid & (~current_queue_item.layer));
	adjusted_write_enable_fg = is_copy_phase | is_fill_pixel1 | (~is_fill_active & current_queue_item.is_valid & current_queue_item.layer);
	
	bg_pixel_color = bg_vram_update(
		adjusted_read_address,		// read address
		adjusted_write_address,     // write address
		adjusted_write_value_bg,	// write value
		adjusted_write_enable_bg	// write enable
	);
	
	fg_pixel_color = fg_vram_update(
		adjusted_read_address,		// read address
		adjusted_write_address,     // write address
		adjusted_write_value_fg,	// write value
		adjusted_write_enable_fg	// write enable
	);
	
	queue_read_value = draw_queue_update(
		queue_read_ptr,			// read address
		queue_write_ptr,		// write address
		queue_write_value,		// write value
		queue_write_enable		// write enable
	);
	
	pixel_counter = vsync ? 0 : (is_active_drawing_area ? (pixel_counter + 1) : pixel_counter);
	buffer_swap_begin_cycle = vsync ? cycle_counter - 65538 : buffer_swap_begin_cycle;
	cycle_counter = vsync ? 0 : cycle_counter + 1;
	is_copy_start_cycle = cycle_counter == buffer_swap_begin_cycle ? 1 : 0;
	
	can_swap_buffers = is_copy_start_cycle ? ((is_cpu_waiting | ~has_screen_vector) ? 1 : ~can_swap_buffers) : can_swap_buffers;
	is_copy_phase = ~vsync & (is_copy_phase | (is_copy_start_cycle & can_swap_buffers));  	
	
	buffer_swap_cycle = (vsync | is_copy_start_cycle) ? 0 : (buffer_swap_cycle + is_copy_phase);
	
	is_fill_active = is_fill_active ? ~(is_new_fill_row & is_last_fill_col) : 0;
	current_queue_item.is_valid = is_fill_active;
	gpu_color = fg_pixel_color == 0 ? bg_pixel_color : fg_pixel_color;
	
	return gpu_color;
}

typedef struct vector_snoop_result_t {
	uint16_t screen;
	uint16_t controller;
	uint1_t has_screen_vector;
	uint1_t has_controller_vector;
} vector_snoop_result_t;

vector_snoop_result_t vector_snoop(uint8_t device_ram_address, uint8_t device_ram_value, uint1_t is_device_ram_write) {
	static vector_snoop_result_t vectors = {0, 0, 0, 0};
	
	if (is_device_ram_write) {
		if (device_ram_address == 0x20) {
			vectors.screen = uint16_uint8_8(vectors.screen, device_ram_value);
			vectors.has_screen_vector = device_ram_value == 0 ? 0 : 1;
		} else if (device_ram_address == 0x21) {
			vectors.screen = uint16_uint8_0(vectors.screen, device_ram_value);
		} else if (device_ram_address == 0x80) {
			vectors.controller = uint16_uint8_8(vectors.controller, device_ram_value);
			vectors.has_controller_vector = device_ram_value == 0 ? 0 : 1;
		} else if (device_ram_address == 0x81) {
			vectors.controller = uint16_uint8_0(vectors.controller, device_ram_value);
		}
	}
	
	return vectors;
}

uint16_t palette_snoop(uint8_t device_ram_address, uint8_t device_ram_value, uint1_t is_device_ram_write, uint2_t gpu_step_color) {
	static uint12_t color[4] = {0xFFF, 0x000, 0x7DB, 0xF62};
	static uint5_t device_ram_addr_7dt3 = 0;
	static uint4_t color_cmp_0 = 0, color_cmp_1 = 0;
	static uint2_t device_ram_addr_2dt1 = 0, index0 = 0, index1 = 0;
	static uint1_t is_palette_range = 0, device_ram_addr_0 = 0;
	
	device_ram_addr_7dt3 = device_ram_address(7, 3);
	is_palette_range = device_ram_addr_7dt3 == 1 ? 1 : 0;
	
	if (is_device_ram_write & is_palette_range) {
		device_ram_addr_2dt1 = device_ram_address(2, 1);
		device_ram_addr_0 = device_ram_address(0);
		color_cmp_0 = device_ram_value(7, 4);
		color_cmp_1 = device_ram_value(3, 0);
		index0 = uint2_uint1_1(0, device_ram_addr_0);
		index1 = index0 | 0b01;
		if (device_ram_addr_2dt1 == 0) {
			color[index0] = uint12_uint4_8(color[index0], color_cmp_0);
			color[index1] = uint12_uint4_8(color[index1], color_cmp_1);
		}
		else if (device_ram_addr_2dt1 == 1) {
			color[index0] = uint12_uint4_4(color[index0], color_cmp_0);
			color[index1] = uint12_uint4_4(color[index1], color_cmp_1);
		}
		else if (device_ram_addr_2dt1 == 2) {
			color[index0] = uint12_uint4_0(color[index0], color_cmp_0);
			color[index1] = uint12_uint4_0(color[index1], color_cmp_1);
		}
	}
	
	return color[gpu_step_color];
}

uint8_t bcd_to_decimal(uint8_t bcd_value) {
	uint8_t tens_digit = 0x0A * uint8_uint4_0(0, bcd_value(7, 4));
	uint8_t ones_digit = uint8_uint4_0(0, bcd_value(3, 0));
	return tens_digit + ones_digit;
}

uint32_t step_time(uint32_t bcd_time, uint1_t bcd_is_valid, uint1_t vsync) {
	static uint8_t ticks = 0;
	static uint8_t seconds = 0;
	static uint8_t minutes = 0;
	static uint8_t hours = 0;
	static uint8_t day_of_week = 0;
	static uint32_t result = 0;
	static uint1_t has_set_from_bcd = 0;
	
	if (bcd_is_valid & ~has_set_from_bcd) {
		ticks = 0;
		day_of_week = bcd_to_decimal(bcd_time(31, 24)); // TODO: maybe this could be simplified
		hours = bcd_to_decimal(bcd_time(23, 16));
		minutes = bcd_to_decimal(bcd_time(15, 8));
		seconds = bcd_to_decimal(bcd_time(7, 0));
		
		result = uint32_uint8_0(0, seconds);
		result = uint32_uint8_8(result, minutes);
		result = uint32_uint8_16(result, hours);
		result = uint32_uint8_24(result, day_of_week);
		
		has_set_from_bcd = 1;
	} else if (vsync) {
		ticks += 1;
		seconds = ticks == 60 ? seconds + 1 : seconds;
		minutes = seconds == 60 ? minutes + 1 : minutes;
		hours = minutes == 60 ? hours + 1 : hours;
		day_of_week = hours == 24 ? day_of_week + 1 : day_of_week;
		ticks = ticks == 60 ? 0 : ticks;
		seconds = seconds == 60 ? 0 : seconds;
		minutes = minutes == 60 ? 0 : minutes;
		hours = hours == 24 ? 0 : hours;
		day_of_week = day_of_week == 7 ? 0 : day_of_week;
		
		result = uint32_uint8_0(0, seconds);
		result = uint32_uint8_8(result, minutes);
		result = uint32_uint8_16(result, hours);
		result = uint32_uint8_24(result, day_of_week);
	}
	
	return result;
}

// #pragma PART "5CGXFC9E7F35C8" // TODO: try quartus step here for Cyclone V
#pragma MAIN uxn_top
uint16_t uxn_top(
	uint33_t rtc_time_bcd,
	uint1_t controller0_up,
	uint1_t controller0_down,
	uint1_t controller0_left,
	uint1_t controller0_right,
	uint1_t controller0_a,
	uint1_t controller0_b,
	uint1_t controller0_x,
	uint1_t controller0_y,
	uint1_t controller0_l,
	uint1_t controller0_r,
	uint1_t controller0_select,
	uint1_t controller0_start,
	uint1_t vsync, // cycle 0 of every frame (new frame)
	uint1_t hsync, // cycle 3 of every horizontal line (new line)
	uint1_t is_visible_pixel,
	uint1_t rom_load_valid_byte,
	uint16_t rom_load_address,
	uint8_t rom_load_value
) {
	static uint24_t boot_check = 0;
	static uint16_t uxn_eval_result = 0;
	static uint1_t is_booted = 0;
	static uint2_t gpu_color;
	static cpu_step_result_t cpu_step_result;
	static uint1_t is_ram_write = 0;
	static uint16_t u16_addr = 0x00FF; // ram address, or occasionally vram write addr
	static vector_snoop_result_t vectors = {0, 0, 0, 0};
	static uint8_t ram_write_value = 0;
	static uint8_t ram_read_value = 0;
	static uint8_t device_ram_address = 0;
	static uint8_t device_ram_read_value = 0;
	static uint1_t is_device_ram_write = 0;
	static uint1_t is_vram_write = 0;
	static uint1_t vram_write_layer = 0;
	static uint8_t vram_value = 0;
	static uint8_t controller0_buttons = 0;
	static uint1_t rtc_valid = 0;
	static uint32_t time_reg = 0;
	
	time_reg = step_time(rtc_time_bcd(31, 0), rtc_time_bcd(32), vsync);
	
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
		controller0_buttons = uint8_uint1_0(0, controller0_a);
		controller0_buttons = uint8_uint1_1(controller0_buttons, controller0_b);
		controller0_buttons = uint8_uint1_2(controller0_buttons, controller0_start);
		controller0_buttons = uint8_uint1_3(controller0_buttons, controller0_select);
		controller0_buttons = uint8_uint1_4(controller0_buttons, controller0_up);
		controller0_buttons = uint8_uint1_5(controller0_buttons, controller0_down);
		controller0_buttons = uint8_uint1_6(controller0_buttons, controller0_left);
		controller0_buttons = uint8_uint1_7(controller0_buttons, controller0_right);
		cpu_step_result = step_cpu(ram_read_value, device_ram_read_value, time_reg, controller0_buttons, vsync, vectors.has_screen_vector, vectors.has_controller_vector, vectors.screen, vectors.controller);
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
	
	gpu_color = step_gpu(is_visible_pixel, is_vram_write, vram_write_layer, u16_addr, vram_value, vectors.has_screen_vector, cpu_step_result.is_waiting, vsync, hsync);
	uxn_eval_result = palette_snoop(device_ram_address, ram_write_value, is_device_ram_write, gpu_color);
	vectors = vector_snoop(device_ram_address, ram_write_value, is_device_ram_write);
	
	return uxn_eval_result;
}