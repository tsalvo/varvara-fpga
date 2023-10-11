#pragma once
#include "uintN_t.h"  // uintN_t types for any N

// 64KB Main RAM
uint8_t main_ram_update(uint16_t ram_address, uint8_t value, uint1_t write_enable) {
	static uint8_t main_ram[65536];
	static uint32_t rwaddr;
	static uint8_t wdata;
	rwaddr = (uint32_t)(ram_address);
	wdata = value;
	
	uint8_t rdata = main_ram_RAM_SP_RF_1(
		rwaddr,			// rw address
		wdata, 			// write value
		write_enable	// write enable
	);
	
	return rdata;
}
