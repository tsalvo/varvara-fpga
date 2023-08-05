#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

// Declare Screen RAM (64KB)
// dual port, first port is read+write,
// second port is read only
// 1 cycle(s) of latency
DECL_RAM_DP_RW_R_1(
  uint8_t,      // Element type - bbffBBFF (element is 2 pixels, BG and FG layers)
  screen_ram,   // RAM function name
  65536,        // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

uint8_t screen_ram_read(uint16_t address) {
	static uint32_t screen_r_rdaddr;       // Read address
	screen_r_rdaddr = (uint32_t)(address); // read 
	
	// The RAM instance
	screen_ram_outputs_t screen_r_ram_out = screen_ram(
		0,                // read-write_addr
		0,                // write data
		0,                // write enable (RW port always writing (not reading))
		0,                // valid read-write input
		0,                // read-write (read) enable 0
		screen_r_rdaddr,  // read address
		1,                // valid read input
		1				  // read enable 1
	);
		
	return screen_r_ram_out.rd_data1;
}

void screen_ram_write(uint17_t address, uint2_t value) {
	static uint32_t screen_w_rwaddr; // Write+read address
	static uint8_t  screen_w_wdata;  // Write data
	screen_w_rwaddr = (uint32_t)(address);
	screen_w_wdata = value;
	
	// The RAM instance
	screen_ram_outputs_t screen_w_ram_out = screen_ram(
		screen_w_rwaddr,   // read-write_addr
		screen_w_wdata,    // write data
		1,                 // write enable (RW port always writing (not reading))
		1,                 // valid read-write input
		0,                 // read-write (read) enable 0
		0,                 // read address
		1,                 // valid read input
		0				   // read enable 1
	);
}

uint2_t pixel_palette_color(uint17_t pixel_index) {
	static uint16_t adjusted_pixel_index;
	static uint1_t use_second_word;
	static uint2_t layer_bg;
	static uint2_t layer_fg;
	static uint8_t pixels;
	static uint2_t result;
	adjusted_pixel_index = (uint16_t)(pixel_index >> 1);
	use_second_word = (pixel_index & 1 == 1) ? 0 : 1;
	pixels = screen_ram_read(adjusted_pixel_index);
	layer_bg = (uint2_t)(pixels >> (use_second_word ? 6 : 2));
	layer_fg = (uint2_t)(pixels >> (use_second_word ? 6 : 2));
	result = (layer_fg == 0) ? layer_bg : layer_fg;	
	return result;
}
