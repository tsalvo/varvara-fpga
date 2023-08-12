#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations


// Dual port, two read+write ports, 1 clock latency
// Does not have read enable separate from clock enable
DECL_RAM_DP_RW_RW_1(
  uint8_t,  // Element type
  main_ram, // RAM function name
  65536,    // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

uint16_t main_ram_update(
	uint16_t address0, 
	uint8_t write_value0,
	uint1_t write_enable0,
	uint16_t address1,
	uint8_t write_value1,
	uint1_t write_enable1
) {
	static uint32_t rwaddr0; // Write+read address 1
	static uint32_t rwaddr1; // Write+read address 2
	rwaddr0 = (uint32_t)(address0);
	rwaddr1 = (uint32_t)(address1);
	
	// The RAM instance
	uint1_t valid0 = 1;
	uint1_t valid1 = 1;
	main_ram_outputs_t ram_out = main_ram(
		rwaddr0, 
		write_value0, 
		write_enable0, 
		valid0, 
		rwaddr1, 
		write_value1, 
		write_enable1, 
		valid1
	);
	
	return (uint16_t)((ram_out.rd_data0 << 8) | ram_out.rd_data1);
}

void poke_ram(uint16_t address, uint8_t value) {
	main_ram_update(address, value, 1, 0, 0, 0);
}

void poke2_ram(uint16_t address, uint16_t value) {
	static uint8_t value0;
	static uint8_t value1;
	value0 = (uint8_t)(value >> 8);
	value1 = (uint8_t)(value);
	main_ram_update(address, value, 1, address + 1, value1, 1);
}

uint8_t peek_ram(uint16_t address) {
	static uint16_t ram_read;
	ram_read = main_ram_update(0, 0, 0, address, 0, 0);
	return (uint8_t)(ram_read);
}

uint16_t peek2_ram(uint16_t address) {
	return main_ram_update(address, 0, 0, address + 1, 0, 0);
}
