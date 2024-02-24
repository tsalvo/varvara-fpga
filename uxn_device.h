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

screen_blit_result_t screen_2bpp(uint8_t phase, uint8_t x1, uint8_t y1, uint4_t color, uint1_t fx, uint1_t fy, uint16_t ram_addr, uint8_t previous_ram_read) {
	static uint8_t blending[80] = {
		0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 0, 2, 3, 3, 3, 0,
		0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
		1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1,
		2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2,
		0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0
	};
	static uint16_t x, y;
	static uint8_t xmod = 0;
	static uint1_t opaque = 0;
	static uint16_t c = 0;
	static uint8_t ch = 0;
	static uint8_t color8;
	static screen_blit_result_t result;
	static uint8_t phase_minus_two = 0;
	static uint4_t phase7_downto_4 = 0;
	static uint5_t phase7_downto_3 = 0;
	static uint3_t phase2_downto_0 = 0;
	static uint8_t sprite_rows[16];
	
	color8 = color;
	phase7_downto_4 = phase(7, 4);
	phase7_downto_3 = phase(7, 3);
	phase2_downto_0 = phase(2, 0);
	phase_minus_two = phase - 2;
	
	if (phase_minus_two(7, 4) == 0) { // phase 2 through 17
		sprite_rows[phase_minus_two] = previous_ram_read;
	}
	
	if (phase == 0) {
		opaque = blending[0x40 + color8];
		xmod = fx ? 0 : 7;
		x = x1 + xmod;
		y = y1 + (fy ? 7 : 0);
	}
	
	if (phase7_downto_4 == 0) { // if phase < 16
		result.is_vram_write = 0;
		result.u8_value = 0;
		result.is_blit_done = 0;
		result.u16_addr = ram_addr + phase; // RAM read
	} else {
		c = phase2_downto_0 == 0b000 ? sprite_rows[(uint8_t)(phase7_downto_3) - 0x02] | (sprite_rows[(uint8_t)(phase7_downto_3) + 0x06] << 8) : c;
		x = phase2_downto_0 == 0b000 ? (x1 + xmod) : x;
		ch = c(8);
		ch <<= 1;
		ch |= c(0);
		result.u16_addr = (y << 8) + x;
		result.is_vram_write = opaque | (ch == 0x00 ? 0 : 1);
		result.u8_value = blending[color8 + (ch << 4)];
		y = phase2_downto_0 == 0b111 ? (fy ? (y - 1) : (y + 1)) : y;
		result.is_blit_done = phase == 0x4F ? 1 : 0;
		x = (fx ? (x + 1) : (x - 1));
		c >>= 1;
	}

	return result;
}

screen_blit_result_t screen_1bpp(uint12_t phase, uint8_t x1, uint8_t y1, uint4_t color, uint1_t fx, uint1_t fy, uint16_t ram_addr, uint8_t previous_ram_read)
{
	static uint2_t blending[48] = {
		0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 0, 2, 3, 3, 3, 0,
		0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
		0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0
	};
	static uint16_t x, y;
	static uint8_t xmod = 0;
	static uint1_t opaque = 0;
	static uint8_t c = 0;
	static uint8_t color8 = 0;
	static screen_blit_result_t result;
	static uint8_t phase_minus_two = 0;
	static uint5_t phase7_downto_3 = 0;
	static uint3_t phase2_downto_0 = 0;
	static uint8_t sprite_rows[8];
	
	color8 = color;
	phase7_downto_3 = phase(7, 3);
	phase2_downto_0 = phase(2, 0);
	phase_minus_two = phase - 2;
	
	if (phase_minus_two(7, 3) == 0) { // phase 2 through 9
		sprite_rows[phase_minus_two] = previous_ram_read;
	}
	
	if (phase == 0) {
		opaque = blending[0x20 + color8];
		xmod = fx ? 0 : 7;
		x = x1 + xmod;
		y = y1 + (fy ? 7 : 0);
	}
	
	if (phase7_downto_3 == 0) { // if phase < 8
		result.is_vram_write = 0;
		result.u8_value = 0;
		result.is_blit_done = 0;
		result.u16_addr = ram_addr + phase; // RAM read
	} else {
		c = phase2_downto_0 == 0b000 ? sprite_rows[phase7_downto_3 - 1] : c;
		x = phase2_downto_0 == 0b000 ? (x1 + xmod) : x;
		result.u16_addr = (y << 8) + x;
		result.is_vram_write = opaque | c(0);
		result.u8_value = blending[color8 + (c(0) ? 0x10 : 0x00)];
		y = phase2_downto_0 == 0b111 ? (fy ? (y - 1) : (y + 1)) : y;
		result.is_blit_done = phase == 0x47 ? 1 : 0; // phase2_downto_0 == 0b111 && phase7_downto_3 == 0b01000;
		c >>= 1;
		x = (fx ? (x + 1) : (x - 1));
	}
	
	return result;
}

device_out_result_t pixel_deo(uint4_t device_port, uint12_t phase, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	static uint16_t x, y;
	static uint8_t ctrl, auto_advance, tmp8;
	static uint4_t color;
	static uint1_t ctrl_mode, flip_x, flip_y, layer, is_auto_x, is_auto_y;
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0};
	
	if (phase == 0x000) {
		result.is_vram_write = 0;
		result.is_device_ram_write = 0;
		result.device_ram_address = 0x29; // x (lo)
		result.is_deo_done = 0;
	} else if (phase == 0x001) {
		result.device_ram_address = 0x2B; // y (lo) 
	} else if (phase == 0x002) {
		x = (uint16_t)(previous_device_ram_read);
		result.device_ram_address = 0x2E; // ctrl
	} else if (phase == 0x003) {
		y = (uint16_t)(previous_device_ram_read);
		result.device_ram_address = 0x26; // auto 
	} else if (phase == 0x004) {
		ctrl = previous_device_ram_read;
		color = (uint4_t)(ctrl);
		ctrl_mode = (uint1_t)(ctrl >> 7);
		layer = (uint1_t)(ctrl >> 6);
		flip_y = (uint1_t)(ctrl >> 5);
		flip_x = (uint1_t)(ctrl >> 4);
		tmp8 = color & 0x3;
		result.device_ram_address = 0;
		result.u16_addr = (y << 8) + x;
		result.vram_write_layer = layer;
		result.is_vram_write = 1;
		if (ctrl_mode) { // fill mode
			tmp8 |= flip_x ? 0x18 : 0x10;
			tmp8 |= flip_y ? 0x04 : 0x00;
			result.is_deo_done = 1;
		}
		result.u8_value = tmp8;
	} else if (phase == 0x005) {
		auto_advance = previous_device_ram_read;
		is_auto_x = auto_advance(0);
		is_auto_y = auto_advance(1);
		result.is_vram_write = 0;
		result.u16_addr = 0;
		result.is_device_ram_write = is_auto_x | is_auto_y;
		result.u8_value = is_auto_x ? (x + 1) : (y + 1);
		result.device_ram_address = is_auto_x ? 0x29 : 0x2B;
		result.is_deo_done = ~(is_auto_x | is_auto_y);
	} else if (phase == 0x006) {
		// auto y if we did auto x last cycle
		result.is_device_ram_write = is_auto_y & is_auto_x;
		result.device_ram_address = 0x2B;
		result.u8_value = (uint8_t)(y + 1); // y (lo)
		result.is_deo_done = 1;
	}
	
	return result;
}

device_out_result_t sprite_deo(uint4_t device_port, uint12_t phase, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	static uint16_t x, y, ram_addr, ram_addr_incr;
	static uint12_t tmp12;
	static uint8_t ctrl, auto_advance, tmp8, tmp8b, x_sprite_incr, y_sprite_incr;
	static uint4_t color, auto_length, tmp4;
	static uint1_t ctrl_mode, flip_x, flip_y, layer, is_blit_done, is_last_blit;
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0};
	static screen_blit_result_t screen_blit_result;
	if (phase == 0x000) {
		is_blit_done = 0;
		result.is_vram_write = 0;
		result.is_device_ram_write = 0;
		result.device_ram_address = 0x29; // x (lo)
		result.is_deo_done = 0;
	} else if (phase == 0x001) {
		result.device_ram_address = 0x2B; // y (lo) 
	} else if (phase == 0x002) {
		x = (uint16_t)(previous_device_ram_read);
		result.device_ram_address = 0x2F; // ctrl
	} else if (phase == 0x003) {
		y = (uint16_t)(previous_device_ram_read);
		result.device_ram_address = 0x2C; // ram_addr (hi)
	} else if (phase == 0x004) {
		ctrl = previous_device_ram_read;
		color = (uint4_t)(ctrl);
		ctrl_mode = (uint1_t)(ctrl >> 7);
		layer = (uint1_t)(ctrl >> 6);
		flip_y = (uint1_t)(ctrl >> 5);
		flip_x = (uint1_t)(ctrl >> 4);
		result.device_ram_address = 0x2D; // ram_addr (lo)
	} else if (phase == 0x005) {
		ram_addr = (uint16_t)(previous_device_ram_read);
		ram_addr <<= 8;
		result.device_ram_address = 0x26; // auto 
	} else if (phase == 0x006) {
		ram_addr |= (uint16_t)(previous_device_ram_read);
		tmp12 = 0x007;
		tmp4 = 0;
		tmp8 = x;
		tmp8b = y;
		is_blit_done = 0;
		is_last_blit = 0;
	} else {
		auto_advance = phase == 0x007 ? previous_device_ram_read : auto_advance;
		auto_length = auto_advance(7, 4); 	  // rML
		x_sprite_incr = auto_advance(0) ? 0x8 : 0;  // rDX
		y_sprite_incr = auto_advance(1) ? 0x8 : 0;  // rDY
		ram_addr_incr = (auto_advance(2) ? (ctrl_mode ? 0x0010 : 0x0008) : 0);

		if (is_blit_done) {
			result.is_vram_write = 0;
			result.u16_addr = 0;
			if (phase == tmp12) {
				tmp8 = flip_x ? (tmp8 - y_sprite_incr) : (tmp8 + y_sprite_incr);
				tmp8b = flip_y ? (tmp8b - x_sprite_incr) : (tmp8b + x_sprite_incr);
				x = (is_last_blit ? (flip_x ? (x - x_sprite_incr) : (x + x_sprite_incr)) : x);
				y = (is_last_blit ? (flip_y ? (y - y_sprite_incr) : (y + y_sprite_incr)) : y);
				ram_addr += ram_addr_incr;
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x29;
				result.u8_value = (uint8_t)(x); // x (lo)
			} else if (phase == tmp12 + 1) {
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x2B;
				result.u8_value = (uint8_t)(y); // y (lo)
			} else if (phase == tmp12 + 2) {
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x2C; // ram_addr (hi)
				result.u8_value = (uint8_t)(ram_addr >> 8);
			} else if (phase == tmp12 + 3) {
				tmp4 += 1;
				screen_blit_result.is_blit_done = 0;
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x2D; // ram_addr (lo)
				result.u8_value = (uint8_t)(ram_addr);
				result.is_deo_done = is_last_blit;
			}
		} else {
			if (ctrl_mode) {
				screen_blit_result = screen_2bpp(phase - tmp12, tmp8, tmp8b, color, flip_x, flip_y, ram_addr, previous_ram_read); // 80 cycles
			} else {
				screen_blit_result = screen_1bpp(phase - tmp12, tmp8, tmp8b, color, flip_x, flip_y, ram_addr, previous_ram_read); // 72 cycles
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
		
	if (phase == 0) {
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
	static device_in_result_t result = {0, 0, 0};
	
	if (phase == 0) {
		result.device_ram_address = device_address;
		result.dei_value = 0;
		result.is_dei_done = 0;
	}
	else if (phase == 1) {
		result.device_ram_address = 0;
		result.dei_value = 0;
		result.is_dei_done = 0;
	}
	else if (phase == 2) {
		result.device_ram_address = 0;
		result.dei_value = previous_device_ram_read;
		result.is_dei_done = 1;
	}
	
	return result;
}

device_in_result_t system_dei(uint8_t device_address, uint8_t phase, uint8_t previous_device_ram_read) {
	static device_in_result_t result = {0, 0, 0};
	if (device_address == 0x04) {
		result.device_ram_address = 0;
		result.dei_value = 0; // TODO: STACK 0 (WST) Pointer
		result.is_dei_done = 1;
	}
	else if (device_address == 0x05) {
		result.device_ram_address = 0;
		result.dei_value = 0; // TODO: STACK 1 (RST) Pointer
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
	else if (device_port == 0x4) { // screen height (240, or 0x00F0) (high byte)
		result.device_ram_address = 0;
		result.dei_value = 0x00;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x5) { // screen height (240, or 0x00F0) (low byte)
		result.device_ram_address = 0;
		result.dei_value = 0xF0;
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
		uint8_t button_map = uint8_uint1_0(0, controller0_buttons(4));
		button_map = uint8_uint1_1(button_map, controller0_buttons(5));
		button_map = uint8_uint1_2(button_map, controller0_buttons(6));
		button_map = uint8_uint1_3(button_map, controller0_buttons(7));
		button_map = uint8_uint1_4(button_map, controller0_buttons(0));
		button_map = uint8_uint1_5(button_map, controller0_buttons(1));
		button_map = uint8_uint1_6(button_map, controller0_buttons(2));
		button_map = uint8_uint1_7(button_map, controller0_buttons(3));
		result.device_ram_address = 0;
		result.dei_value = button_map;
		result.is_dei_done = 1;
	}
	else {
		result = generic_dei(device_address, phase, previous_device_ram_read);
	}
	
	return result;
}

device_in_result_t device_in(uint8_t device_address, uint8_t phase, uint8_t controller0_buttons, uint8_t previous_device_ram_read) {
	static uint8_t device;
	static device_in_result_t result = {0, 0, 0};
	
	device = device_address & 0xF0;
	
	if (device == 0x00) {
		result = system_dei(device_address, phase, previous_device_ram_read);
	}
	else if (device == 0x20) {
		result = screen_dei(device_address, phase, previous_device_ram_read);
	}
	else if (device == 0x80) {
		result = controller_dei(device_address, phase, controller0_buttons, previous_device_ram_read);
	}
	else {
		result = generic_dei(device_address, phase, previous_device_ram_read);
	}
	
	return result;
}