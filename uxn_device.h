#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

#pragma once
#include "uxn_ram_device.h"
#pragma once
#include "uxn_ram_screen.h"
#pragma once
#include "uxn_stack.h" 

uint1_t screen_deo(uint4_t device_port, uint4_t phase) {
	static uint16_t ctrl_none, x, y;
	static uint8_t ctrl;
	static uint2_t color;
	static uint1_t is_fill_mode, result;
	
	if (phase == 0) {
		if (device_port == 0xE) { // PIXEL
			ctrl_none = peek2_dev(0x2E); // START ctrl_none
		}
		
		result = 0;
	}
	else if (phase == 1) {
		if (device_port == 0xE) { // PIXEL
			ctrl_none = peek2_dev(0x28); // DONE ctrl_none / START x
			ctrl = (uint8_t)(ctrl_none >> 8);
			color = (uint2_t)(ctrl);
			is_fill_mode = (ctrl & 0x80) == 0 ? 0 : 1;
		}
	}
	else if (phase == 2) {
		if (device_port == 0xE) { // PIXEL
			x = peek2_dev(0x2A); // DONE x / START y
		}
	}
	else if (phase == 3) {
		if (device_port == 0xE) { // PIXEL
			y = peek2_dev(0x2A); // DONE y
			
		}
	}
	else if (phase == 4) {
		if (device_port == 0xE) { // PIXEL
			background_vram_update(
				((uint32_t)(y) * (uint32_t)(320)) + ((uint32_t)(x)), 	// port 0 address
				(uint2_t)(color),		// port 0 write value
				1,						// port 0 write enable
				0,						// port 0 read enable
				0						// port 1 read address
			);
		}
	}
	else if (phase == 5) { // END
		if (device_port == 0xE) { // PIXEL
			// TODO: implement auto-advance
			result = 1;
		}
		result = 1;
	}
	
	return result;
}

uint1_t emu_deo(uint4_t device_index, uint4_t device_port, uint4_t phase) {
	uint1_t result;
	
	if (phase == 0) {
		if (device_index == 0x2) { // SCREEN
			result = screen_deo(device_port, 0);
		} else {
			result = 1;
		}
	}
	else if (phase == 1) {
		if (device_index == 0x2) { // SCREEN
			result = screen_deo(device_port, 1);
		} else {
			result = 1;
		}
	}
	else if (phase == 2) {
		if (device_index == 0x2) { // SCREEN
			result = screen_deo(device_port, 2);
		} else {
			result = 1;
		}
	}
	else if (phase == 3) {
		if (device_index == 0x2) { // SCREEN
			result = screen_deo(device_port, 3);
		} else {
			result = 1;
		}
	}
	else if (phase == 4) {
		if (device_index == 0x2) { // SCREEN
			result = screen_deo(device_port, 4);
		} else {
			result = 1;
		}
	}
	else if (phase == 5) { // END
		if (device_index == 0x2) { // SCREEN
			result = screen_deo(device_port, 5);
		} else {
			result = 1;
		}
	}
	else {
		result = 1;
	}
	
	return result;
}

uint1_t device_out(uint8_t device_address, uint8_t value, uint4_t phase) {
	static uint1_t result;
	static uint4_t device_index, device_port;
	static uint16_t deo_mask[16] = {
		0xff28, 0x0300, 0xc028, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0xa260, 0xa260, 0x0000, 0x0000, 0x0000, 0x0000
	};
	
	if (phase == 0) {
		poke_dev(device_address, value); // START
		device_port = (uint4_t)(device_address & 0x0F);
		device_index = (uint4_t)(device_address >> 4);
		result = deo_mask[device_index] == 0 ? 1 : 0;
	}
	else if (phase == 1) {
		device_port = (uint4_t)(device_address & 0x0F);
		device_index = (uint4_t)(device_address >> 4);
		result = deo_mask[device_index] == 0 ? 1 : 0;
		if (result == 0) {
			result = emu_deo(device_index, device_port, 0);
		}
	}
	else if (phase == 2) {
		result = emu_deo(device_index, device_port, 1);
	}
	else if (phase == 3) {
		result = emu_deo(device_index, device_port, 2);
	}
	else if (phase == 4) {
		result = emu_deo(device_index, device_port, 3);
	}
	else if (phase == 5) {
		result = emu_deo(device_index, device_port, 4);
	}
	else if (phase == 6) { // END
		result = emu_deo(device_index, device_port, 5);
	}
	else {
		result = 1;
	}
	
	return result;
}