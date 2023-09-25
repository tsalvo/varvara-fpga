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

typedef struct device_out_result_t {
	uint1_t is_device_ram_write;
	uint1_t is_device_ram_read;
	uint8_t device_ram_address;
	uint8_t device_ram_value;
	
	uint1_t is_vram_write;
	uint32_t vram_address;
	uint2_t vram_value;
	
	uint1_t is_deo_done;
} device_out_result_t;

device_out_result_t screen_deo(uint4_t device_port, uint8_t phase, uint8_t previous_device_ram_read) {
	static uint16_t ctrl_none, x, y;
	static uint8_t pixel, sprite, auto_advance;
	static uint2_t color;
	static uint1_t is_fill_mode;
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0, 0};
	
	printf("            SCREEN DEO: Port: 0x%X, Phase 0x%X\n", device_port, phase);
	
	if (phase == 0x00) {
		if (device_port == 0xE) { // PIXEL
			result.is_device_ram_write = 0;
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x2E; // pixel
			result.is_deo_done = 0;
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x01) {
		if (device_port == 0xE) { // PIXEL
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x2E; 
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x02) {
		if (device_port == 0xE) { // PIXEL
			pixel = previous_device_ram_read;
			color = (uint2_t)(pixel);
			is_fill_mode = (pixel & 0x80) == 0 ? 0 : 1;
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x2F; // sprite
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x03) {
		if (device_port == 0xE) { // PIXEL
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x2F;
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x04) {
		if (device_port == 0xE) { // PIXEL
			sprite = previous_device_ram_read;
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x28; // x (hi)
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x05) {
		if (device_port == 0xE) { // PIXEL
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x28;
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x06) {
		if (device_port == 0xE) { // PIXEL
			x = (uint16_t)(previous_device_ram_read);
			x <<= 8;
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x29; // x (lo)
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x07) {
		if (device_port == 0xE) { // PIXEL
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x29;
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x08) {
		if (device_port == 0xE) { // PIXEL
			x |= (uint16_t)(previous_device_ram_read);
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x2A; // y (hi)
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x09) {
		if (device_port == 0xE) { // PIXEL			
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x2A;
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x0A) {
		if (device_port == 0xE) { // PIXEL
			y = (uint16_t)(previous_device_ram_read);
			y <<= 8;
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x2B; // y (lo)
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x0B) {
		if (device_port == 0xE) { // PIXEL
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x2B;
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x0C) {
		if (device_port == 0xE) { // PIXEL
			y |= (uint16_t)(previous_device_ram_read);
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x26; // (auto)
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x0D) { // END
		if (device_port == 0xE) { // PIXEL
			result.is_device_ram_read = 1;
			result.device_ram_address = 0x26;
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x0E) { // END
		if (device_port == 0xE) { // PIXEL
			printf("            SCREEN DEO: VRAM Write: X = 0x%X, Y = 0x%X, Color = 0x%X\n", x, y, (uint4_t)(color));
			auto_advance = previous_device_ram_read;
			result.is_device_ram_read = 0;
			result.device_ram_address = 0;
			result.is_vram_write = 1;
			result.vram_address = ((uint32_t)(y) * (uint32_t)(320)) + ((uint32_t)(x));
			result.vram_value = (uint2_t)(color);
		} else {
			result.is_deo_done = 1;
		}
	}
	else if (phase == 0x0F) { // END
		if (device_port == 0xE) { // PIXEL
			// TODO: implement auto-advance
			result.is_vram_write = 0;
			result.vram_address = 0;
			result.vram_value = 0;
			result.is_deo_done = 1;
		}
		result.is_deo_done = 1;
	}
	else {
		result.is_deo_done = 1;
	}
	
	return result;
}

device_out_result_t emu_deo(uint4_t device_index, uint4_t device_port, uint8_t phase, uint8_t previous_device_ram_read) {
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0, 0};
	
	if (device_index == 0x2) { // SCREEN
		result = screen_deo(device_port, phase, previous_device_ram_read);
	} else {
		result.is_deo_done = 1;
	}
	
	return result;
}

device_out_result_t device_out(uint8_t device_address, uint8_t value, uint8_t phase, uint8_t previous_device_ram_read) {
	static device_out_result_t result = {0, 0, 0, 0, 0, 0, 0, 0};
	static uint4_t device_index, device_port;
	static uint16_t deo_mask[16] = {
		0xff28, 0x0300, 0xc028, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0xa260, 0xa260, 0x0000, 0x0000, 0x0000, 0x0000
	};
	
	printf("       DEVICE OUT: Address: 0x%X, Value: 0x%X, Phase 0x%X\n", device_address, value, phase);
	
	if (phase == 0) {
		result.is_device_ram_read = 0;
		result.is_device_ram_write = 1;
		result.device_ram_address = device_address;
		result.device_ram_value = value;
		// poke_dev(device_address, value); // START
		device_port = (uint4_t)(device_address & 0x0F);
		device_index = (uint4_t)(device_address >> 4);
		result.is_deo_done = deo_mask[device_index] == 0 ? 1 : 0;
	}
	else if (phase == 1) {
		device_port = (uint4_t)(device_address & 0x0F);
		device_index = (uint4_t)(device_address >> 4);
		result.is_deo_done = deo_mask[device_index] == 0 ? 1 : 0;
	}
	else {
		result = emu_deo(device_index, device_port, phase - 2, previous_device_ram_read);
	}
	
	return result;
}