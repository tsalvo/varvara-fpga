#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

// Main RAM (64KB)
// Dual port (1 read+write port, 1 read-only port).  
// 1 clock cycle latency
DECL_RAM_DP_RW_R_1(
  uint8_t,  // Element type
  main_ram, // RAM function name
  65536,    // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

uint16_t main_ram_update(
	uint16_t address0, 
	uint8_t write0_value,
	uint1_t write0_enable,
	uint1_t read0_enable,
	uint16_t address1,
	uint1_t read1_enable
) {
	static uint32_t rdaddr; // Read address
	static uint32_t rwaddr; // Write+read address
	static uint8_t wdata;   // Write data, start writing zeros
	static uint8_t rdvalue0;
	static uint8_t rdvalue1;
	rwaddr = (uint32_t)(address0);
	wdata = write0_value;
	rdaddr = (uint32_t)(address1);

	// The RAM instance
	uint1_t rw_valid = 1; // Always have valid RAM inputs
	uint1_t rd_valid = 1; // Always have valid RAM inputs
	main_ram_outputs_t ram_out = main_ram(
		rwaddr, 
		wdata, 
		write0_enable, 
		rw_valid, 
		read0_enable, 
		rdaddr, 
		rd_valid, 
		read1_enable
	);

	rdvalue0 = ram_out.valid0 & read0_enable ? ram_out.rd_data0 : 0;
	rdvalue1 = ram_out.valid1 & read1_enable ? ram_out.rd_data1 : 0;
	
	return (uint16_t)((rdvalue0 << 8) | rdvalue1);
}

void poke_ram(uint16_t address, uint8_t value) {
	main_ram_update(address, value, 1, 0, 0, 0);
}

void poke2_ram(uint16_t address, uint16_t value) {
	static uint8_t value0;
	static uint8_t value1;
	value0 = (uint8_t)(value >> 8);
	value1 = (uint8_t)(value);
	main_ram_update(address, value0, 1, 0, 0, 0);
	main_ram_update(address + 1, value1, 1, 0, 0, 0);
}

uint8_t peek_ram(uint16_t address) {
	static uint16_t ram_read;
	ram_read = main_ram_update(0, 0, 0, 0, address, 1);
	return (uint8_t)(ram_read);
}

uint16_t peek2_ram(uint16_t address) {
	return main_ram_update(address, 0, 0, 1, address + 1, 1);
}
