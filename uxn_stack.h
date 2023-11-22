#pragma once
#include "uintN_t.h"  // uintN_t types for any N

// 512 bytes Stack RAM (256 for work stack, 256 for return stack)
uint8_t stack_ram_update(uint9_t stack_address, uint8_t value, uint1_t write_enable) {
	static uint8_t stack_ram[512];
	static uint32_t rwaddr;
	static uint8_t wdata;
	rwaddr = (uint32_t)(stack_address);
	wdata = value;
	
	uint8_t rdata = stack_ram_RAM_SP_RF_1(
		rwaddr,			// rw address
		wdata, 			// write value
		write_enable	// write enable
	);
	
	return rdata;
}
