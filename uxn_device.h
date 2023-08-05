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

uint8_t screen_dei(uint8_t addr) {
	static uint8_t result;
	// TODO: implement
	return result;
}

uint8_t datetime_dei(uint8_t addr) {
	static uint8_t result;
	// TODO: implement
	return result;
}

uint8_t uxn_dei(uint8_t addr) {
	/*
	Varvara
	00  system	    80	controller
	10  console     90	mouse
	20  screen      a0	file
	30  audio	    b0
	40              c0	datetime
	50              d0	Reserved
	60              e0
	70  	        f0
	*/
	static uint8_t d;
	static uint8_t result;
	d = addr & 0xF0;
	result = 0;
	
	if (d == 0x20) {
		result = screen_dei(addr);
	}
	else if (d == 0xC0) {
		result = datetime_dei(addr);
	} 
	else {
		result = device_ram_read(addr);
	}
	
	return result;
}
	
void dei(uint1_t stack_index, uint8_t stack_offset, uint8_t addr) {
	static uint16_t dei_mask[16] = {0x0000, 0x0000, 0x003c, 0x0014, 0x0014, 0x0014, 0x0014, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x07ff, 0x0000, 0x0000, 0x0000};
	static uint16_t is_event; 
	static uint8_t value;
	is_event = (dei_mask[value >> 4] >> (addr & 0x0F)) & 0x0001;
	value = is_event ? uxn_dei(addr) : device_ram_read(addr);
	put_stack(stack_index, stack_offset, value);
}

void system_deo(uint8_t d, uint8_t device_port) {
	if (device_port == 0x03) {
		// TODO: implement
		// system_cmd(u->ram, PEEK2(d + 2));
	} 
	else if (device_port == 0x05) {
		// TODO: implement
		// system_friend(u->ram, PEEK2(d + 4));
	} 
	else if (device_port == 0x0E) {
		// TODO: implement
		// system_friend(u->ram, PEEK2(d + 4));
	}
}

void console_deo(uint8_t d, uint8_t device_port) {
	// TODO: implement
}

void screen_deo(uint8_t d, uint8_t device_port) {
	// TODO: implement
}

void screen_palette(uint8_t device_port) {
	// TODO: implement
}

void file_deo(uint1_t file_index, uint8_t d, uint8_t p) {
	// TODO: implement
}

void uxn_deo(uint8_t addr)
{
	/*
	Varvara
	00  system	    80	controller
	10  console     90	mouse
	20  screen      a0	file
	30  audio	    b0
	40              c0	datetime
	50              d0	Reserved
	60              e0
	70  	        f0
	*/
	static uint8_t device_port;
	static uint8_t device_index;
	static uint1_t port_range_palette_lo;
	static uint1_t port_range_palette_hi;
	device_port = addr & 0x0F;
	device_index = addr & 0xF0;
	if (device_index == 0x00) { // system
		system_deo(device_ram_read(device_index), device_port);
		port_range_palette_lo = device_port > 0x07 ? 1 : 0;
		port_range_palette_hi = device_port < 0x0E ? 1 : 0;
		if (port_range_palette_lo & port_range_palette_hi) {
			// set system palette colors
			//----------------------------------
			// color0  color1  color2  color3
			// #fff	   #000    #7ec    #f00
			//----------------------------------
			// #f07f .System/r DEO2
			// #f0e0 .System/g DEO2
			// #f0c0 .System/b DEO2
			screen_palette(device_ram_read(0x08));
		}
	}
	else if (device_index == 0x10) { // console
		console_deo(device_ram_read(device_index), device_port);
	}
	else if (device_index == 0x20) { // screen
		screen_deo(device_ram_read(device_index), device_port);
	}
	else if (device_index == 0xA0) { // file 1
		file_deo(0, device_ram_read(device_index), device_port);
	}
	else if (device_index == 0xB0) { // file 2
		file_deo(1, device_ram_read(device_index), device_port);
	}
}

void deo(uint8_t device_address, uint8_t value) {
	// #define DEO(a, b) { u->dev[(a)] = (b); if((deo_mask[(a) >> 4] >> ((a) & 0xf)) & 0x1) uxn_deo(u, (a)); }
	static uint16_t deo_mask[16] = {0xff28, 0x0300, 0xc028, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0xa260, 0xa260, 0x0000, 0x0000, 0x0000, 0x0000};
	device_ram_write(device_address, value);
	if ((deo_mask[(device_address) >> 4] >> ((device_address) & 0x0F)) & 0x0001) {
		uxn_deo(device_address); 
	}
}
