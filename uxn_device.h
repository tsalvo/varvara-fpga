#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#include <stdint.h>

#pragma once
#include "uxn_ram_device.h"
#pragma once
#include "uxn_ram_screen.h"
#pragma once
#include "uxn_stack.h" 

typedef struct device_in_result_t {
	uint8_t device_ram_address;
	
	uint8_t dei_value;
	uint1_t is_dei_done;
} device_in_result_t;

typedef struct device_out_result_t {
	uint1_t is_device_ram_write;
	uint8_t device_ram_address;
	
	uint1_t is_vram_write;
	uint1_t vram_write_layer;
	uint16_t u16_addr; // vram_address or ram_address
	uint8_t u8_value; // device ram write value, RAM write value
	
	uint1_t is_deo_done;
} device_out_result_t;

typedef struct screen_blit_result_t {
	
	uint1_t is_vram_write;
	uint1_t vram_write_layer;
	uint16_t u16_addr; // vram_address write, or ram_address read
	
	uint8_t u8_value;
	
	uint1_t is_blit_done;
} screen_blit_result_t;

screen_blit_result_t screen_2bpp(uint12_t phase, uint16_t x1, uint16_t y1, uint4_t color, uint1_t fx, uint1_t fy, uint16_t ram_addr, uint8_t previous_ram_read) {
	static uint8_t blending[80] = {
		0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 0, 2, 3, 3, 3, 0,
		0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
		1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1,
		2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2,
		0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0
	};
	static uint16_t x, y;
	static uint1_t opaque = 0;
	static uint16_t c = 0;
	static uint8_t ch = 0;
	static uint8_t color8;
	static screen_blit_result_t result;
	static uint12_t phase_minus_two = 0;
	static uint4_t phase7_downto_4 = 0;
	static uint5_t phase7_downto_3 = 0;
	static uint3_t phase2_downto_0 = 0;
	static uint8_t phase7_downto_3_u8 = 0;
	static uint8_t sprite_rows[16];
	static uint1_t is_x_in_bounds = 0, is_y_in_bounds = 0, is_new_row = 0;
	
	color8 = color;
	phase7_downto_4 = phase(7, 4);
	phase7_downto_3 = phase(7, 3);
	phase2_downto_0 = phase(2, 0);
	phase7_downto_3_u8 = phase7_downto_3;
	phase_minus_two = phase - 2;
	
	if (phase_minus_two(7, 4) == 0) { // phase 2 through 17
		sprite_rows[phase_minus_two] = previous_ram_read;
	}
	
	if (phase == 0) {
		opaque = blending[0x40 + color8];
		x = x1 + (fx ? 0x0000 : 0x0007);
		y = y1 + (fy ? 0x0007 : 0x0000);
	}
	
	if (phase7_downto_4 == 0) { // if phase < 16
		result.is_vram_write = 0;
		result.u8_value = 0;
		result.is_blit_done = 0;
		result.u16_addr = ram_addr + phase; // RAM read
	} else {
		is_new_row = phase2_downto_0 == 0b00 ? 1 : 0;
		if (is_new_row) {
			c = uint16_uint8_8(0, sprite_rows[phase7_downto_3_u8 + 0x06]);
			c = uint16_uint8_0(c, sprite_rows[phase7_downto_3_u8 - 0x02]);
		}
		x = is_new_row ? (x1 + (fx ? 0x0000 : 0x0007)) : x;
		is_x_in_bounds = (x(15, 8) == 0x00) ? 1 : 0;
		is_y_in_bounds = (y(15, 8) == 0x00) ? 1 : 0;
		ch = uint8_uint1_5(0, c(8));
		ch = uint8_uint1_4(ch, c(0));
		result.u16_addr = uint16_uint8_8(0, y(7, 0));
		result.u16_addr = uint16_uint8_0(result.u16_addr, x(7, 0));
		result.is_vram_write = is_x_in_bounds & is_y_in_bounds & (opaque | (ch == 0x00 ? 0 : 1));
		result.u8_value = blending[color8 + ch];
		y = phase2_downto_0 == 0b111 ? (fy ? (y - 1) : (y + 1)) : y;
		result.is_blit_done = phase == 0x04F ? 1 : 0;
		x = (fx ? (x + 1) : (x - 1));
		c >>= 1;
	}

	return result;
}

screen_blit_result_t screen_1bpp(uint12_t phase, uint16_t x1, uint16_t y1, uint4_t color, uint1_t fx, uint1_t fy, uint16_t ram_addr, uint8_t previous_ram_read)
{
	static uint2_t blending[48] = {
		0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 0, 2, 3, 3, 3, 0,
		0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
		0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0
	};
	static uint16_t x, y;
	static uint1_t opaque = 0;
	static uint8_t c = 0;
	static uint8_t color8 = 0;
	static screen_blit_result_t result;
	static uint12_t phase_minus_two = 0;
	static uint5_t phase7_downto_3 = 0;
	static uint3_t phase2_downto_0 = 0;
	static uint8_t sprite_rows[8];
	static uint1_t is_x_in_bounds = 0;
	static uint1_t is_y_in_bounds = 0;
	
	color8 = color;
	phase7_downto_3 = phase(7, 3);
	phase2_downto_0 = phase(2, 0);
	phase_minus_two = phase - 2;
	
	if (phase_minus_two(7, 3) == 0) { // phase 2 through 9
		sprite_rows[phase_minus_two] = previous_ram_read;
	}
	
	if (phase == 0) {
		opaque = blending[0x20 + color8];
		x = x1 + (fx ? 0x0000 : 0x0007);
		y = y1 + (fy ? 0x0007 : 0x0000);
	}
	
	if (phase7_downto_3 == 0) { // if phase < 8
		result.is_vram_write = 0;
		result.u8_value = 0;
		result.is_blit_done = 0;
		result.u16_addr = ram_addr + phase; // RAM read
	} else {
		c = phase2_downto_0 == 0b000 ? sprite_rows[phase7_downto_3 - 1] : c;
		x = phase2_downto_0 == 0b000 ? (x1 + (fx ? 0x0000 : 0x0007)) : x;
		is_x_in_bounds = x(15, 8) == 0x00 ? 1 : 0;
		is_y_in_bounds = y(15, 8) == 0x00 ? 1 : 0;
		result.u16_addr = uint16_uint8_8(0, y(7, 0));
		result.u16_addr = uint16_uint8_0(result.u16_addr, x(7, 0));
		result.is_vram_write = is_x_in_bounds & is_y_in_bounds & (opaque | c(0));
		result.u8_value = blending[color8 + (c(0) ? 0x10 : 0x00)];
		y = phase2_downto_0 == 0b111 ? (fy ? (y - 1) : (y + 1)) : y;
		result.is_blit_done = phase == 0x047 ? 1 : 0;
		c >>= 1;
		x = (fx ? (x + 1) : (x - 1));
	}
	
	return result;
}

device_out_result_t pixel_deo(uint4_t device_port, uint12_t phase, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	static uint8_t x, y, ctrl, auto_advance, tmp8, tmp8b;
	static uint4_t phase4;
	static uint2_t color;
	static uint1_t ctrl_mode, flip_x, flip_y, layer, is_auto_x, is_auto_y, is_x_in_bounds, is_y_in_bounds;
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0};
	phase4 = phase(3, 0);
	
	if (phase4 == 0x0) {
		result.is_vram_write = 0;
		result.is_device_ram_write = 0;
		result.device_ram_address = 0x28; // x (hi)
		result.is_deo_done = 0;
	}
	else if (phase4 == 0x1) {
		result.device_ram_address = 0x29; // x (lo)
	}
	else if (phase4 == 0x2) {
		result.device_ram_address = 0x2A; // y (hi) 
		x = (uint16_t)(previous_device_ram_read);
		x <<= 8;
		is_x_in_bounds = previous_device_ram_read == 0 ? 1 : 0;
	}
	else if (phase4 == 0x3) {
		result.device_ram_address = 0x2B; // y (lo) 
		x |= (uint16_t)(previous_device_ram_read);
	}
	else if (phase4 == 0x4) {
		result.device_ram_address = 0x2E; // ctrl
		y = (uint16_t)(previous_device_ram_read);
		y <<= 8;
		is_y_in_bounds = previous_device_ram_read == 0 ? 1 : 0;
	}
	else if (phase4 == 0x5) {
		y |= (uint16_t)(previous_device_ram_read);
		result.device_ram_address = 0x26; // auto 
	}
	else if (phase4 == 0x6) {
		ctrl = previous_device_ram_read;
		ctrl_mode = ctrl(7);
		layer = ctrl(6);
		flip_y = ctrl(5);
		flip_x = ctrl(4);
		color = ctrl(1, 0);
		// extra bits for fill mode 0b000FXYCC (F = Is Fill, X = Flip X, Y = Flip Y)
		tmp8 = uint8_uint2_0(0, color);
		tmp8 = uint8_uint1_2(tmp8, flip_y);
		tmp8 = uint8_uint1_3(tmp8, flip_x);
		tmp8 = uint8_uint1_4(tmp8, ctrl_mode);
		tmp8b = ~(ctrl_mode & ~is_x_in_bounds) ? x : 0xFF;
		is_x_in_bounds = ctrl_mode ? (flip_x | is_x_in_bounds) : is_x_in_bounds;
		is_y_in_bounds = ctrl_mode ? (flip_y | is_y_in_bounds) : is_y_in_bounds;
		result.u16_addr = uint16_uint8_0(0, x);
		result.u16_addr = uint16_uint8_8(result.u16_addr, y);
		result.vram_write_layer = layer;
		result.device_ram_address = 0;
		result.is_vram_write = is_x_in_bounds & is_y_in_bounds;
		result.is_deo_done = ctrl_mode;
		result.u8_value = tmp8;
	}
	else if (phase4 == 0x7) {
		auto_advance = previous_device_ram_read;
		is_auto_x = auto_advance(0);
		is_auto_y = auto_advance(1);
		result.is_vram_write = 0;
		result.u16_addr = 0;
		result.is_device_ram_write = is_auto_x | is_auto_y;
		is_x_in_bounds = is_auto_x & is_x_in_bounds & (x == 0xFF ? 1 : 0) ? 0 : is_x_in_bounds;
		is_y_in_bounds = is_auto_y & is_y_in_bounds & (y == 0xFF ? 1 : 0) ? 0 : is_y_in_bounds;
		result.u8_value = (is_auto_x ? ~is_x_in_bounds : ~is_y_in_bounds);
		result.device_ram_address = is_auto_x ? 0x28 : 0x2A;
		result.is_deo_done = ~(is_auto_x | is_auto_y);
	}
	else if (phase4 == 0x8) {
		result.is_device_ram_write = is_auto_x | is_auto_y;
		result.u8_value = is_auto_x ? (x + 1) : (y + 1);
		result.device_ram_address = is_auto_x ? 0x29 : 0x2B;
		result.is_deo_done = ~(is_auto_x | is_auto_y);
	}
	else if (phase4 == 0x9) {
		// auto y if we did auto x last cycle
		result.is_device_ram_write = is_auto_y & is_auto_x;
		result.device_ram_address = 0x2A;
		result.u8_value = ~is_y_in_bounds; // y (hi)
		result.is_deo_done = 0;
	}
	else if (phase4 == 0xA) {
		// auto y if we did auto x last cycle
		result.is_device_ram_write = is_auto_y & is_auto_x;
		result.device_ram_address = 0x2B;
		result.u8_value = y + 1; // y (lo)
		result.is_deo_done = 1;
	}
	
	return result;
}

device_out_result_t sprite_deo(uint4_t device_port, uint12_t phase, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	static uint16_t x, y, ram_addr, ram_addr_incr, tmp16, tmp16b;
	static uint12_t tmp12;
	static uint8_t ctrl, auto_advance, x_sprite_incr, y_sprite_incr;
	static uint4_t color, auto_length, tmp4;
	static uint1_t ctrl_mode, flip_x, flip_y, layer, is_blit_done, is_last_blit;
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0};
	static screen_blit_result_t screen_blit_result;
	
	if (phase == 0x000) {
		is_blit_done = 0;
		result.is_vram_write = 0;
		result.is_device_ram_write = 0;
		result.device_ram_address = 0x28; // x (hi)
		result.is_deo_done = 0;
	}
	else if (phase == 0x001) {
		result.device_ram_address = 0x29; // x (lo) 
	}
	else if (phase == 0x002) {
		x = (uint16_t)(previous_device_ram_read);
		x <<= 8;
		result.device_ram_address = 0x2A; // y (hi) 
	}
	else if (phase == 0x003) {
		x |= (uint16_t)(previous_device_ram_read);
		result.device_ram_address = 0x2B; // y (lo) 
	}
	else if (phase == 0x004) {
		y = (uint16_t)(previous_device_ram_read);
		y <<= 8;
		result.device_ram_address = 0x2F; // ctrl
	}
	else if (phase == 0x005) {
		y |= (uint16_t)(previous_device_ram_read);
		result.device_ram_address = 0x2C; // ram_addr (hi)
	}
	else if (phase == 0x006) {
		ctrl = previous_device_ram_read;
		ctrl_mode = ctrl(7);
		layer = ctrl(6);
		flip_y = ctrl(5);
		flip_x = ctrl(4);
		color = ctrl(3, 0);
		result.device_ram_address = 0x2D; // ram_addr (lo)
	}
	else if (phase == 0x007) {
		ram_addr = (uint16_t)(previous_device_ram_read);
		ram_addr <<= 8;
		result.device_ram_address = 0x26; // auto 
	}
	else if (phase == 0x008) {
		ram_addr |= (uint16_t)(previous_device_ram_read);
		tmp12 = 0x009;
		tmp4 = 0;
		tmp16 = x;
		tmp16b = y;
		is_blit_done = 0;
		is_last_blit = 0;
	}
	else {
		auto_advance = phase == 0x009 ? previous_device_ram_read : auto_advance;
		auto_length = auto_advance(7, 4); 	  // rML
		x_sprite_incr = auto_advance(0) ? 0x8 : 0;  // rDX
		y_sprite_incr = auto_advance(1) ? 0x8 : 0;  // rDY
		ram_addr_incr = (auto_advance(2) ? (ctrl_mode ? 0x0010 : 0x0008) : 0);
		if (is_blit_done) {
			if (tmp12 == phase) {
				tmp16 = flip_x ? (tmp16 - y_sprite_incr) : (tmp16 + y_sprite_incr);
				tmp16b = flip_y ? (tmp16b - x_sprite_incr) : (tmp16b + x_sprite_incr);
				x = (is_last_blit ? (flip_x ? (x - x_sprite_incr) : (x + x_sprite_incr)) : x);
				y = (is_last_blit ? (flip_y ? (y - y_sprite_incr) : (y + y_sprite_incr)) : y);
				result.is_vram_write = 0;
				result.u16_addr = 0;
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x28;
				result.u8_value = (uint8_t)(x >> 8); // x (hi) WRITE
			}
			else if (tmp12 == phase - 1) {
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x29;
				result.u8_value = (uint8_t)(x);      // x (lo) WRITE
			}
			else if (tmp12 == phase - 2) {
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x2A;    
				result.u8_value = (uint8_t)(y >> 8); // y (hi) WRITE
			}
			else if (tmp12 == phase - 3) {
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x2B;
				result.u8_value = (uint8_t)(y); 	// y (lo) WRITE
			}
			else if (tmp12 == phase - 4) {
				ram_addr += ram_addr_incr;
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x2C; // ram_addr (hi) WRITE
				result.u8_value = (uint8_t)(ram_addr >> 8);
			}
			else if (tmp12 == phase - 5) {
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x2D; // ram_addr (lo) WRITE
				result.u8_value = (uint8_t)(ram_addr);
			}
			else if (tmp12 == phase - 6) {
				tmp4 += 1;
				screen_blit_result.is_blit_done = 0;
				result.is_device_ram_write = 0; 
				result.device_ram_address = 0x00;
				result.is_deo_done = is_last_blit;
			}
		} else {
			if (ctrl_mode) {
				screen_blit_result = screen_2bpp(phase - tmp12, tmp16, tmp16b, color, flip_x, flip_y, ram_addr, previous_ram_read); // 80 cycles
			} else {
				screen_blit_result = screen_1bpp(phase - tmp12, tmp16, tmp16b, color, flip_x, flip_y, ram_addr, previous_ram_read); // 72 cycles
			}
			
			result.device_ram_address = 0;
			result.is_device_ram_write = 0;
			result.is_vram_write = screen_blit_result.is_vram_write;
			result.u16_addr = screen_blit_result.u16_addr;
			result.vram_write_layer = layer;
			result.u8_value = screen_blit_result.u8_value;
			is_last_blit = auto_length == tmp4 ? 1 : 0;
		}
		
		tmp12 = is_blit_done ^ screen_blit_result.is_blit_done ? phase + 1 : tmp12;
		is_blit_done = screen_blit_result.is_blit_done;
	}
	
	return result;
}

device_out_result_t screen_deo(uint4_t device_port, uint12_t phase, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0};
	if (device_port == 0xE) {
		result = pixel_deo(device_port, phase, previous_device_ram_read, previous_ram_read);
	} else if (device_port == 0xF) {
		result = sprite_deo(device_port, phase, previous_device_ram_read, previous_ram_read);
	} else {
		result.is_vram_write = 0;
		result.is_device_ram_write = 0;
		result.is_deo_done = 1;
	}
	
	return result;
}

device_out_result_t emu_deo(uint4_t device_index, uint4_t device_port, uint12_t phase, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0};
	
	if (device_index == 0x2) { // SCREEN
		result = screen_deo(device_port, phase, previous_device_ram_read, previous_ram_read);
	} else {
		result.is_vram_write = 0;
		result.is_device_ram_write = 0;
		result.is_deo_done = 1;
	}
	
	return result;
}

device_out_result_t device_out(uint8_t device_address, uint8_t value, uint12_t phase, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0};
	static uint4_t device_index, device_port;
	static uint1_t deo_mask[16] = {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1};
		
	if (phase == 0x000) {
		result.is_vram_write = 0;
		result.is_device_ram_write = 1;
		result.device_ram_address = device_address;
		result.u8_value = value;
		device_index = (uint4_t)(device_address >> 4);
		result.is_deo_done = deo_mask[device_index];
	}
	else {
		device_port = (uint4_t)(device_address);
		result = emu_deo(device_index, device_port, phase - 1, previous_device_ram_read, previous_ram_read);
	}

	return result;
}

device_in_result_t generic_dei(uint8_t device_address, uint8_t phase, uint8_t previous_device_ram_read) {
	
	device_in_result_t result;
	result.device_ram_address = phase == 0 ? device_address : 0;
	result.dei_value = phase == 2 ? previous_device_ram_read : 0;
	result.is_dei_done = phase == 2 ? 1 : 0;
	
	return result;
}

device_in_result_t system_dei(uint8_t device_address, uint8_t phase, uint8_t stack_ptr0, uint8_t stack_ptr1, uint8_t previous_device_ram_read) {
	static device_in_result_t result = {0, 0, 0};
	if (device_address == 0x04) {
		result.device_ram_address = 0;
		result.dei_value = stack_ptr0;
		result.is_dei_done = 1;
	}
	else if (device_address == 0x05) {
		result.device_ram_address = 0;
		result.dei_value = stack_ptr1;
		result.is_dei_done = 1;
	}
	else {
		result = generic_dei(device_address, phase, previous_device_ram_read);
	}
	
	return result;
}

device_in_result_t screen_dei(uint8_t device_address, uint8_t phase, uint8_t previous_device_ram_read) {
	static device_in_result_t result = {0, 0, 0};
	static uint4_t device_port = 0;
	device_port = (uint4_t)device_address;
	if (device_port == 0x2) {      // screen width (256, or 0x0100) (high byte)
		result.device_ram_address = 0;
		result.dei_value = 0x01;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x3) { // screen width (256, or 0x0100) (low byte)
		result.device_ram_address = 0;
		result.dei_value = 0x00;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x4) { // screen height (256, or 0x0100) (high byte)
		result.device_ram_address = 0;
		result.dei_value = 0x01;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x5) { // screen height (256, or 0x0100) (low byte)
		result.device_ram_address = 0;
		result.dei_value = 0x00;
		result.is_dei_done = 1;
	}
	else {
		result = generic_dei(device_address, phase, previous_device_ram_read);
	}
	
	return result;
}

device_in_result_t controller_dei(uint8_t device_address, uint8_t phase, uint8_t controller0_buttons, uint8_t previous_device_ram_read) {
	static device_in_result_t result = {0, 0, 0};
	static uint4_t device_port = 0;
	device_port = (uint4_t)device_address;
	
	if (device_port == 0x2) {  // button RLDUTSBA (right, left, down, up, start, select, B, A)
		result.device_ram_address = 0;
		result.dei_value = controller0_buttons;
		result.is_dei_done = 1;
	}
	else {
		result = generic_dei(device_address, phase, previous_device_ram_read);
	}
	
	return result;
}

device_in_result_t datetime_dei(uint8_t device_address, uint8_t phase, uint8_t previous_device_ram_read) {
	static device_in_result_t result = {0, 0, 0};
	static uint4_t device_port = 0;
	device_port = (uint4_t)device_address;
	
	// TODO: result should be based on system real time clock
	if (device_port == 0x0) { 		// year, high byte
		result.device_ram_address = 0;
		result.dei_value = 0x07;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x1) { // year, low byte
		result.device_ram_address = 0;
		result.dei_value = 0xBE;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x2) { // month
		result.device_ram_address = 0;
		result.dei_value = 0x0B;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x3) { // day
		result.device_ram_address = 0;
		result.dei_value = 0x1E;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x4) { // hour
		result.device_ram_address = 0;
		result.dei_value = 0x06;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x5) { // minute
		result.device_ram_address = 0;
		result.dei_value = 0x33;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x6) { // second
		result.device_ram_address = 0;
		result.dei_value = 0x04;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x7) { // day of week, beginning Sunday
		result.device_ram_address = 0;
		result.dei_value = 0x05;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x8) { // day of year, high byte
		result.device_ram_address = 0;
		result.dei_value = 0x01;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x9) { // day of year, low byte
		result.device_ram_address = 0;
		result.dei_value = 0x6C;
		result.is_dei_done = 1;
	}
	else if (device_port == 0xA) { // is daylight savings time
		result.device_ram_address = 0;
		result.dei_value = 0x00;
		result.is_dei_done = 1;
	}
	else {
		result = generic_dei(device_address, phase, previous_device_ram_read);
	}
	
	return result;
}

device_in_result_t device_in(uint8_t device_address, uint8_t phase, uint8_t controller0_buttons, uint8_t stack_ptr0, uint8_t stack_ptr1, uint8_t previous_device_ram_read) {
	static uint8_t device;
	static device_in_result_t result = {0, 0, 0};
	
	device = device_address & 0xF0;
	
	if (device == 0x00) {
		result = system_dei(device_address, phase, stack_ptr0, stack_ptr1, previous_device_ram_read);
	}
	else if (device == 0x20) {
		result = screen_dei(device_address, phase, previous_device_ram_read);
	}
	else if (device == 0x80) {
		result = controller_dei(device_address, phase, controller0_buttons, previous_device_ram_read);
	}
	else if (device == 0xC0) {
		result = datetime_dei(device_address, phase, previous_device_ram_read);
	}
	else {
		result = generic_dei(device_address, phase, previous_device_ram_read);
	}
	
	return result;
}