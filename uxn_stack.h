#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"

// Dual port, two read+write ports, 2 clock latency
// Does not have read enable separate from clock enable
DECL_RAM_DP_RW_RW_1(
  uint8_t,    // Element type
  stack_ram, // RAM function name
  512,        // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

uint16_t stack_ram_update(
	uint9_t address0, 
	uint8_t write0_value,
	uint1_t write0_enable,
	uint9_t address1,
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
	
	stack_ram_outputs_t stack_ram_out = stack_ram(
		rwaddr0,
		wdata0,
		write0_enable,
		rw_valid0,
		rwaddr1,
		wdata1,
		write1_enable,
		rw_valid1
	);
	
	rdvalue0 = stack_ram_out.rd_data0;
	rdvalue1 = stack_ram_out.rd_data1;
	
	result = uint16_uint8_0(0, rdvalue1);
	result = uint16_uint8_8(result, rdvalue0);
	
	return result;
}

