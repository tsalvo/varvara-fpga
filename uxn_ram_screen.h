#pragma once
#include "uintN_t.h"  // uintN_t types for any N

// 260 x 234
#define SCREEN_RAM_SIZE 65536

uint2_t bg_vram_update(
	uint16_t read_address, 
	uint16_t write_address, 
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
	uint16_t read_address, 
	uint16_t write_address, 
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