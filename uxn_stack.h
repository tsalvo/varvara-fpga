#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "intN_t.h"   // intN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

// Declare Working Stack RAM (256 bytes)
// dual port, first port is read+write,
// second port is read only
// 1 cycle(s) of latency
DECL_RAM_DP_RW_R_1(
  uint8_t,    // Element type
  stack_w_ram, // RAM function name
  256,        // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

// Declare Return Stack RAM (256 bytes)
// dual port, first port is read+write,
// second port is read only
// 1 cycle(s) of latency
DECL_RAM_DP_RW_R_1(
  uint8_t,    // Element type
  stack_r_ram, // RAM function name
  256,        // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

uint16_t stack_w_ram_update(
	uint8_t address0, 
	uint8_t write0_value,
	uint1_t write0_enable,
	uint1_t read0_enable,
	uint8_t address1,
	uint1_t read1_enable
) {
	static uint32_t rdaddr; // Read address
	static uint32_t rwaddr; // Write+read address
	static uint8_t wdata;   // Write data, start writing zeros
	static uint8_t rdvalue0;
	static uint8_t rdvalue1;
	static uint16_t result;
	rwaddr = (uint32_t)(address0);
	rdaddr = (uint32_t)(address1);
	wdata = write0_value;
	
	// The RAM instance
	uint1_t rw_valid = 1; // Always have valid RAM inputs
	uint1_t rd_valid = 1; // Always have valid RAM inputs
	stack_w_ram_outputs_t stack_w_ram_out = stack_w_ram(
		rwaddr,            // read-write_addr
		write0_value,      // write data
		write0_enable,     // write enable (RW port always writing (not reading))
		rw_valid,          // valid read-write input
		read0_enable,      // read-write (read) enable 0
		rdaddr,            // read address
		rd_valid,          // valid read input
		read1_enable	   // read enable 1
	);
		
	rdvalue0 = stack_w_ram_out.valid0 & read0_enable ? stack_w_ram_out.rd_data0 : 0;
	rdvalue1 = stack_w_ram_out.valid1 & read1_enable ? stack_w_ram_out.rd_data1 : 0;
	
	result = (uint16_t)(rdvalue0);
	result <<= 8;
	result |= ((uint16_t)(rdvalue1));
	
	printf("     Stack W rwaddr = 0x%X, write0_enable = 0x%X, wdata = 0x%X, rdaddr = 0x%X, read0_enable = 0x%X, read1_enable = 0x%X, result = 0x%X\n", rwaddr, write0_enable, wdata, rdaddr, read0_enable, read1_enable, result);
	
	
	return result;
}

uint16_t stack_r_ram_update(
	uint8_t address0, 
	uint8_t write0_value,
	uint1_t write0_enable,
	uint1_t read0_enable,
	uint8_t address1,
	uint1_t read1_enable
) {
	static uint32_t rdaddr; // Read address
	static uint32_t rwaddr; // Write+read address
	static uint8_t wdata;   // Write data, start writing zeros
	static uint8_t rdvalue0;
	static uint8_t rdvalue1;
	static uint16_t result;
	rwaddr = (uint32_t)(address0);
	rdaddr = (uint32_t)(address1);
	wdata = write0_value;
	
	// The RAM instance
	uint1_t rw_valid = 1; // Always have valid RAM inputs
	uint1_t rd_valid = 1; // Always have valid RAM inputs
	stack_r_ram_outputs_t stack_r_ram_out = stack_r_ram(
		rwaddr,            // read-write_addr
		write0_value,      // write data
		write0_enable,     // write enable (RW port always writing (not reading))
		rw_valid,          // valid read-write input
		read0_enable,      // read-write (read) enable 0
		rdaddr,            // read address
		rd_valid,          // valid read input
		read1_enable	   // read enable 1
	);
		
	rdvalue0 = stack_r_ram_out.valid0 & read0_enable ? stack_r_ram_out.rd_data0 : 0;
	rdvalue1 = stack_r_ram_out.valid1 & read1_enable ? stack_r_ram_out.rd_data1 : 0;
	
	result = (uint16_t)(rdvalue0);
	result <<= 8;
	result |= ((uint16_t)(rdvalue1));
	
	printf("     Stack R rwaddr = 0x%X, write0_enable = 0x%X, wdata = 0x%X, rdaddr = 0x%X, read0_enable = 0x%X, read1_enable = 0x%X, result = 0x%X\n", rwaddr, write0_enable, wdata, rdaddr, read0_enable, read1_enable, result);
	
	return result;
}
