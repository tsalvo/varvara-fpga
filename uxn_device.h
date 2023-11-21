#pragma once
#include "uintN_t.h"  // uintN_t types for any N

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
	uint1_t is_device_ram_16bit;
	uint8_t device_ram_address;
	
	uint1_t is_vram_write;
	uint1_t vram_write_layer;
	uint24_t vram_address;
	
	uint16_t u16_value; // Main RAM Address or 16-bit device RAM write
	
	uint8_t u8_value; // device ram write value, RAM write value
	
	uint1_t is_deo_done;
} device_out_result_t;

typedef struct screen_blit_result_t {
	
	uint16_t ram_address;
	
	uint1_t is_vram_write;
	uint1_t vram_write_layer;
	uint24_t vram_address;
	
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
		result.ram_address = 0;
		result.vram_address = 0;
		result.is_blit_done = 1;
	}
	
	return result;
}

device_out_result_t screen_deo(uint4_t device_port, uint8_t phase, uint16_t previous_device_ram_read, uint8_t previous_ram_read) {
	static uint24_t vram_addr;
	static uint16_t x, y, ram_addr;
	static uint8_t ctrl, auto_advance;
	static uint4_t color;
	static uint1_t is_pixel_port, is_sprite_port, is_drawing_port, ctrl_mode, flip_x, flip_y, layer, is_x_onscreen, is_y_onscreen;
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	
	if (phase == 0x00) {
		is_pixel_port = device_port == 0xE ? 1 : 0;
		is_sprite_port = device_port == 0xF ? 1 : 0;
		is_drawing_port = is_pixel_port | is_sprite_port;
		result.is_vram_write = 0;
		result.is_device_ram_write = 0;
		vram_addr = 0;
		if (is_drawing_port) { // PIXEL or SPRITE
			result.device_ram_address = 0x2E; // [pixel_ctrl 0x2E][sprite_ctrl 0x2F]
			result.is_device_ram_16bit = 1;
			result.is_deo_done = 0;
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x01) {
		// Assume is_drawing_port == 1 from here on out
		result.device_ram_address = 0x28; // x [0x28][0x29]
	}
	else if (phase == 0x02) {
		ctrl = is_pixel_port ? ((uint8_t)(previous_device_ram_read >> 8)) : ((uint8_t)(previous_device_ram_read));
		color = (uint4_t)(ctrl);
		ctrl_mode = (uint1_t)(ctrl >> 7);
		layer = (uint1_t)(ctrl >> 6);
		flip_y = (uint1_t)(ctrl >> 5);
		flip_x = (uint1_t)(ctrl >> 4);
		result.device_ram_address = 0x2A; // y [0x2A][0x2B]
	}
	else if (phase == 0x03) {
		x = previous_device_ram_read;
		is_x_onscreen = previous_device_ram_read < 0x0140 ? 1 : 0;
		result.is_device_ram_16bit = 0;
		result.device_ram_address = 0x26; // auto [0x26]
	}
	else if (phase == 0x04) {
		y = previous_device_ram_read;
		is_y_onscreen = previous_device_ram_read < 0x0120 ? 1 : 0;
		result.is_device_ram_16bit = 1;
		result.device_ram_address = 0x2C; // ram_addr [0x2C][0x2D]
		if (is_pixel_port) { // PIXEL
			result.device_ram_address = 0;
			result.u8_value = (uint8_t)(color & 0x3);
			result.vram_write_layer = layer;
			if (ctrl_mode) { // fill mode
				x = is_x_onscreen ? x : 0x013F;
				y = is_y_onscreen ? y : 0x011F;
				vram_addr = ((uint24_t)(y) * (uint24_t)(320)) + ((uint24_t)(x));
				vram_addr &= 0x03FFFF;
				vram_addr |= (flip_y ? 0x080000 : 0);
				vram_addr |= (flip_x ? 0x040000 : 0);
				vram_addr |= 0xF00000;
				result.is_vram_write = 0;
				result.vram_address = vram_addr;
			} else if (is_x_onscreen & is_y_onscreen) { // single pixel mode, where pixel is onscreen
				result.vram_address = ((uint24_t)(y) * (uint24_t)(320)) + ((uint24_t)(x));
				result.is_vram_write = 1;
			}
		}
	}
	else if (phase == 0x05) {
		auto_advance = (uint8_t)previous_device_ram_read;
		result.is_vram_write = 0;
		result.vram_address = 0;
		result.u8_value = 0;
		if (is_pixel_port) { // PIXEL
			result.is_vram_write = 0;
			result.vram_address = 0;
			if (ctrl_mode) { // Fill mode
				result.is_device_ram_write = 0;
				result.is_device_ram_16bit = 0;
				result.is_deo_done = 1; // no auto-advance for fill mode.  we're done
			} else if (auto_advance & 0x01) { // auto X in pixel mode
				x += 1;
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x28; // x [0x28][0x29]
				result.u16_value = x;
			}
		}
	}
	else if (phase == 0x06) {
		ram_addr = previous_device_ram_read;
		if (is_pixel_port) { // PIXEL
			// Assume single-pixel mode
			if ((auto_advance >> 1) & 0x01) { // auto Y
				result.is_device_ram_write = 1;
				result.device_ram_address = 0x2A; // y [0x2A][0x2B]
				result.u16_value = y; 
			} else {
				result.is_device_ram_write = 0;
				result.is_device_ram_16bit = 0;
				result.is_deo_done = 1;
			}
		}
		else if (is_sprite_port) { // SPRITE
			result.is_device_ram_write = 0;
			result.is_device_ram_16bit = 0;
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x07) {
		if (is_pixel_port) { // PIXEL
			result.is_device_ram_write = 0;
			result.is_device_ram_16bit = 0;
			result.is_deo_done = 1;
		}
	}
	else {
		if (is_sprite_port) { // SPRITE
			screen_blit_result_t screen_blit_result = screen_blit(phase - 0x08, ctrl, auto_advance, x, y, ram_addr, previous_ram_read);
			result.is_device_ram_write = 0;
			result.device_ram_address = 0;
			result.is_vram_write = screen_blit_result.is_vram_write;
			result.vram_address = screen_blit_result.vram_address;
			result.vram_write_layer = screen_blit_result.vram_write_layer;
			result.u8_value = screen_blit_result.u8_value;
			result.u16_value = screen_blit_result.ram_address;
			result.is_deo_done = screen_blit_result.is_blit_done;
		} else {
			result.is_device_ram_write = 0;
			result.device_ram_address = 0;
			result.is_deo_done = 1;
		}
	}
	
	return result;
}

device_out_result_t emu_deo(uint4_t device_index, uint4_t device_port, uint8_t phase, uint16_t previous_device_ram_read, uint8_t previous_ram_read) {
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	
	if (device_index == 0x2) { // SCREEN
		result = screen_deo(device_port, phase, previous_device_ram_read, previous_ram_read);
	} else {
		result.is_deo_done = 1;
	}
	
	return result;
}

device_out_result_t device_out(uint8_t device_address, uint8_t value, uint8_t phase, uint16_t previous_device_ram_read, uint8_t previous_ram_read) {
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	static uint4_t device_index, device_port;
	static uint16_t deo_mask[16] = {
		0xff28, 0x0300, 0xc028, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0xa260, 0xa260, 0x0000, 0x0000, 0x0000, 0x0000
	};
		
	if (phase == 0) {
		result.is_vram_write = 0;
		result.is_device_ram_write = 1;
		result.device_ram_address = device_address;
		result.u8_value = value;
		device_port = (uint4_t)(device_address & 0x0F);
		device_index = (uint4_t)(device_address >> 4);
		result.is_deo_done = deo_mask[device_index] == 0 ? 1 : 0;
	}
	else if (phase == 1) {
		result.is_device_ram_write = 0;
		device_port = (uint4_t)(device_address & 0x0F);
		device_index = (uint4_t)(device_address >> 4);
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
	if (device_address == 0x22) {      // screen width (320, or 0x0140) (high byte)
		result.device_ram_address = 0;
		result.dei_value = 0x01;
		result.is_dei_done = 1;
	}
	else if (device_address == 0x23) { // screen width (320, or 0x0140) (low byte)
		result.device_ram_address = 0;
		result.dei_value = 0x40;
		result.is_dei_done = 1;
	}
	else if (device_address == 0x24) { // screen height (288, or 0x0120) (high byte)
		result.device_ram_address = 0;
		result.dei_value = 0x01;
		result.is_dei_done = 1;
	}
	else if (device_address == 0x25) { // screen height (288, or 0x0120) (low byte)
		result.device_ram_address = 0;
		result.dei_value = 0x20;
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