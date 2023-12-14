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

screen_blit_result_t screen_blit(uint8_t phase, uint8_t ctrl, uint8_t auto_advance, uint16_t x, uint16_t y, uint16_t ram_addr, uint8_t previous_ram_read) {
	static uint2_t blending[64] = {
		0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 0, 2, 3, 3, 3, 0,
		0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
		1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1,
		2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2
	};
	static uint1_t ctrl_mode, layer, flip_x, flip_y;
	static uint16_t dx, dy, dxy, dyx, dyy, dxx, ram_addr_incr;
	static uint12_t i_phase;
	static uint4_t length, i_length;
	static screen_blit_result_t result;
	
	if (phase == 0) {
		ctrl_mode = (uint1_t)(ctrl >> 7);
		layer = (uint1_t)(ctrl >> 6);
		flip_y = (uint1_t)(ctrl >> 5);
		flip_x = (uint1_t)(ctrl >> 4);
		dx = ((uint16_t)(auto_advance & 0x01) << 3);
		dy = ((uint16_t)(auto_advance & 0x02) << 2);
		dxy = flip_y ? (dx * (-1)) : dx;
		dyx = flip_x ? (dy * (-1)) : dy;
		dxx = flip_x ? (dx * (-1)) : dx;
		dyy = flip_y ? (dy * (-1)) : dy;
		ram_addr_incr = (auto_advance & 0x04) << (1 + ((ctrl & 0x80) > 0 ? 1 : 0));
		length = (uint4_t)(auto_advance >> 4);
		i_phase = 0;
		i_length = length;
	}
	else {
		// TODO: implement
		result.is_vram_write = 0;
		result.u16_addr = 0;
		result.is_blit_done = 1;
	}
	
	return result;
}

device_out_result_t screen_deo(uint4_t device_port, uint8_t phase, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	static uint16_t x, y, ram_addr;
	static uint8_t ctrl, auto_advance, tmp8;
	static uint4_t color;
	static uint1_t is_pixel_port, is_sprite_port, is_drawing_port, ctrl_mode, flip_x, flip_y, layer;
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0};
	
	if (phase == 0x00) {
		is_pixel_port = device_port == 0xE ? 1 : 0;
		is_sprite_port = device_port == 0xF ? 1 : 0;
		is_drawing_port = is_pixel_port | is_sprite_port;
		result.is_vram_write = 0;
		result.is_device_ram_write = 0;
		if (is_drawing_port) { // PIXEL or SPRITE
			result.device_ram_address = is_pixel_port ? 0x2E : 0x2F; // ctrl
			result.is_deo_done = 0;
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x01) { // assume is_drawing_port
		result.device_ram_address = 0x28; // x (hi)
	}
	else if (phase == 0x02) {
		ctrl = previous_device_ram_read;
		color = (uint4_t)(ctrl);
		ctrl_mode = (uint1_t)(ctrl >> 7);
		layer = (uint1_t)(ctrl >> 6);
		flip_y = (uint1_t)(ctrl >> 5);
		flip_x = (uint1_t)(ctrl >> 4);
		result.device_ram_address = 0x29; // x (lo)
	}
	else if (phase == 0x03) {
		x = (uint16_t)(previous_device_ram_read);
		x <<= 8;
		result.device_ram_address = 0x2A; // y (hi) 
	}
	else if (phase == 0x04) {
		x |= (uint16_t)(previous_device_ram_read);
		result.device_ram_address = 0x2B; // y (lo) 
	}
	else if (phase == 0x05) {
		y = (uint16_t)(previous_device_ram_read);
		y <<= 8;
		result.device_ram_address = 0x26; // auto 
	}
	else if (phase == 0x06) {
		if (is_pixel_port) { // PIXEL
			result.device_ram_address = 0;
			y |= (uint16_t)(previous_device_ram_read);
			tmp8 = color & 0x3;
			result.u16_addr = (y * 0x0104) + x;
			result.vram_write_layer = layer;
			result.is_vram_write = 1;
			if (ctrl_mode) { // fill mode
				tmp8 |= flip_x ? 0x18 : 0x10;
				tmp8 |= flip_y ? 0x04 : 0x00;
				result.is_deo_done = 1;
			}
			
			result.u8_value = tmp8;
		}
		else { // SPRITE
			y |= (uint16_t)(previous_device_ram_read);
			result.device_ram_address = 0x2C; // ram_addr (hi)
		}
	}
	else if (phase == 0x07) {
		if (is_pixel_port) { // PIXEL, assume single-pixel mode
			auto_advance = previous_device_ram_read;
			result.is_vram_write = 0;
			result.u16_addr = 0;
			result.u8_value = 0;
			if (auto_advance & 0x01) { // auto X
				x += 1;
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x28;
				result.u8_value = (uint8_t)(x >> 8); // x (hi)
			}
		}
		else { // SPRITE
			auto_advance = previous_device_ram_read;
			result.device_ram_address = 0x2D; // ram_addr (lo)
		}
	}
	else if (phase == 0x08) {
		if (is_pixel_port) { // PIXEL
			if (auto_advance & 0x01) { // auto X
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x29;
				result.u8_value = (uint8_t)(x); // x (lo)
			}
		}
		else { // SPRITE
			ram_addr = (uint16_t)(previous_device_ram_read);
			ram_addr <<= 8;
		}
	}
	else if (phase == 0x09) {
		if (is_pixel_port) {  // PIXEL, assume single pixel mode
			if ((auto_advance >> 1) & 0x01) { // auto Y
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x2A;
				result.u8_value = (uint8_t)(y >> 8); // y (hi)
			}
		}
		else { // SPRITE
			ram_addr |= (uint16_t)(previous_device_ram_read);
		}
	}
	else if (phase == 0x0A) {
		if (is_pixel_port) {  // PIXEL, assume single pixel mode
			if ((auto_advance >> 1) & 0x01) { // auto Y
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x2B;
				result.u8_value = (uint8_t)(y); // y (lo)
			}
		}
	}
	else {
		if (is_sprite_port) { // SPRITE
			screen_blit_result_t screen_blit_result = screen_blit(phase - 0x0B, ctrl, auto_advance, x, y, ram_addr, previous_ram_read);
			result.is_device_ram_write = 0;
			result.device_ram_address = 0;
			result.is_vram_write = screen_blit_result.is_vram_write;
			result.u16_addr = screen_blit_result.u16_addr;
			result.vram_write_layer = screen_blit_result.vram_write_layer;
			result.u8_value = screen_blit_result.u8_value;
			result.is_deo_done = screen_blit_result.is_blit_done;
		} else {
			result.is_device_ram_write = 0;
			result.device_ram_address = 0;
			result.is_deo_done = 1;
		}
	}
	
	return result;
}

device_out_result_t emu_deo(uint4_t device_index, uint4_t device_port, uint8_t phase, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0};
	
	if (device_index == 0x2) { // SCREEN
		result = screen_deo(device_port, phase, previous_device_ram_read, previous_ram_read);
	} else {
		result.is_deo_done = 1;
	}
	
	return result;
}

device_out_result_t device_out(uint8_t device_address, uint8_t value, uint8_t phase, uint8_t previous_device_ram_read, uint8_t previous_ram_read) {
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0};
	static uint4_t device_index, device_port;
	static uint16_t deo_mask[16] = {
		0xff28, 0x0300, 0xc028, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0xa260, 0xa260, 0x0000, 0x0000, 0x0000, 0x0000
	};
		
	if (phase == 0) {
		result.is_vram_write = 0;
		result.is_device_ram_write = 1;
		result.device_ram_address = device_address;
		result.u8_value = value;
		device_port = (uint4_t)(device_address);
		device_index = (uint4_t)(device_address >> 4);
		result.is_deo_done = deo_mask[device_index] == 0 ? 1 : 0;
	}
	else if (phase == 1) {
		result.is_device_ram_write = 0;
		result.is_deo_done = deo_mask[device_index] == 0 ? 1 : 0;
	}
	else {
		result = emu_deo(device_index, device_port, phase - 2, previous_device_ram_read, previous_ram_read);
	}
	
	return result;
}

device_in_result_t generic_dei(uint8_t device_address, uint8_t phase, uint8_t previous_device_ram_read) {
	static device_in_result_t result = {0, 0, 0};
	
	if (phase < 2) {
		result.device_ram_address = device_address;
		result.dei_value = 0;
		result.is_dei_done = 0;
	}
	else {
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
	if (device_port == 0x2) {      // screen width (260, or 0x0104) (high byte)
		result.device_ram_address = 0;
		result.dei_value = 0x01;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x3) { // screen width (260, or 0x0104) (low byte)
		result.device_ram_address = 0;
		result.dei_value = 0x04;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x4) { // screen height (234, or 0x00EA) (high byte)
		result.device_ram_address = 0;
		result.dei_value = 0x00;
		result.is_dei_done = 1;
	}
	else if (device_port == 0x5) { // screen height (234, or 0x00EA) (low byte)
		result.device_ram_address = 0;
		result.dei_value = 0xEA;
		result.is_dei_done = 1;
	}
	else {
		result = generic_dei(device_address, phase, previous_device_ram_read);
	}
	
	return result;
}

device_in_result_t device_in(uint8_t device_address, uint8_t phase, uint8_t previous_device_ram_read) {
	static uint8_t device;
	static device_in_result_t result = {0, 0, 0};
	
	device = device_address & 0xF0;
	
	if (device == 0x00) {
		result = system_dei(device_address, phase, previous_device_ram_read);
	}
	else if (device == 0x20) {
		result = screen_dei(device_address, phase, previous_device_ram_read);
	}
	else {
		result = generic_dei(device_address, phase, previous_device_ram_read);
	}
	
	return result;
}