#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"

// Dual port, two read+write ports, 1 clock latency
// Does not have read enable separate from clock enable
DECL_RAM_DP_RW_RW_1(
  uint8_t,    // Element type
  device_ram, // RAM function name
  256,        // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

uint16_t device_ram_update(
	uint8_t address0, 
	uint8_t write0_value,
	uint1_t write0_enable,
	uint8_t address1,
	uint8_t write1_value,
	uint1_t write1_enable
) {
	static uint32_t rwaddr0;
	static uint32_t rwaddr1;
	static uint8_t wdata0;
	static uint8_t wdata1;
	static uint8_t rdvalue0;
	static uint8_t rdvalue1;
	static uint16_t result;
	rwaddr0 = (uint32_t)(address0);
	rwaddr1 = (uint32_t)(address1);
	wdata0 = write0_value;
	wdata1 = write1_value;
	
	// The RAM instance
	uint1_t rw_valid0 = 1; // Always have valid RAM inputs
	uint1_t rw_valid1 = 1; // Always have valid RAM inputs
	
	device_ram_outputs_t device_ram_out = device_ram(
		rwaddr0,
		wdata0,
		write0_enable,
		rw_valid0,
		rwaddr1,
		wdata1,
		write1_enable,
		rw_valid1
	);
	
	rdvalue0 = device_ram_out.rd_data0;
	rdvalue1 = device_ram_out.rd_data1;
	
	result = (uint16_t)(rdvalue0);
	result <<= 8;
	result |= ((uint16_t)(rdvalue1));
	
	return result;
}
