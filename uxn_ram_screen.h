#pragma once
#include "uintN_t.h"  // uintN_t types for any N

// 260 x 234 (double-buffering)
#define SCREEN_RAM_SIZE 65536 * 2
#define DRAW_QUEUE_SIZE 8192

uint24_t draw_queue_update(
	uint13_t read_address, 
	uint13_t write_address, 
	uint24_t write_value, 
	uint1_t write_enable
) {
	static uint24_t draw_queue_ram[DRAW_QUEUE_SIZE];
	static uint32_t waddr = 0;
	static uint32_t wdata = 0;
	static uint32_t raddr = 0;
	raddr = (uint32_t)read_address;
	waddr = (uint32_t)write_address;
	wdata = write_value;
	
	uint24_t rdata = draw_queue_ram_RAM_DP_RF_1(
		raddr,			// read address
		waddr, 			// write address
		wdata,			// write value
		write_enable	// write enable
	);
	
	return rdata;
}

uint2_t bg_vram_update(
	uint17_t read_address, 
	uint17_t write_address, 
	uint2_t write_value, 
	uint1_t write_enable
) {
	static uint2_t bg_vram[SCREEN_RAM_SIZE];
	static uint32_t waddr = 0;
	static uint32_t wdata = 0;
	static uint32_t raddr = 0;
	raddr = (uint32_t)read_address;
	waddr = (uint32_t)write_address;
	wdata = (uint32_t)write_value;
	
	uint2_t rdata = bg_vram_RAM_DP_RF_1(
		raddr,			// read address
		waddr, 			// write address
		wdata,			// write value
		write_enable	// write enable
	);
	
	return rdata;
}

uint2_t fg_vram_update(
	uint17_t read_address, 
	uint17_t write_address, 
	uint2_t write_value, 
	uint1_t write_enable
) {
	static uint2_t fg_vram[SCREEN_RAM_SIZE];
	static uint32_t waddr = 0;
	static uint32_t wdata = 0;
	static uint32_t raddr = 0;
	raddr = (uint32_t)read_address;
	waddr = (uint32_t)write_address;
	wdata = (uint32_t)write_value;
	
	uint2_t rdata = fg_vram_RAM_DP_RF_1(
		raddr,			// read address
		waddr, 			// write address
		wdata,			// write value
		write_enable	// write enable
	);
	
	return rdata;
}