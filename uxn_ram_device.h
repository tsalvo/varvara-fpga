#pragma once
#include "uintN_t.h"  // uintN_t types for any N

// 256 bytes Device RAM
uint8_t device_ram_update(uint8_t device_address, uint8_t value, uint1_t write_enable) {
	static uint8_t device_ram[256];
	static uint32_t rwaddr;
	static uint8_t wdata;
	rwaddr = (uint32_t)(device_address);
	wdata = value;
	
	uint8_t rdata = device_ram_RAM_SP_RF_1(
		rwaddr,			// rw address
		wdata, 			// write value
		write_enable	// write enable
	);
	
	return rdata;
}
