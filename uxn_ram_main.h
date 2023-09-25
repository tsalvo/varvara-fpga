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
	static uint16_t result;
	rwaddr = (uint32_t)(address0);
	wdata = write0_value;
	rdaddr = (uint32_t)(address1);

	// The RAM instance
	uint1_t rw_valid = read0_enable | write0_enable;
	uint1_t rd_valid = read1_enable; 
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
	
	result = (uint16_t)(rdvalue0);
	result <<= 8;
	result |= ((uint16_t)(rdvalue1));
	
	printf("  Main RAM rwaddr = 0x%X, write0_enable = 0x%X, wdata = 0x%X, rdaddr = 0x%X, read0_enable = 0x%X, read1_enable = 0x%X, result = 0x%X\n", rwaddr, write0_enable, wdata, rdaddr, read0_enable, read1_enable, result);
	
	return result;
}
