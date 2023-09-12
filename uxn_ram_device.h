#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

// Declare XY Screen RAM(4 bytes)
// dual port, first port is read+write,
// second port is read only
// 0 cycle(s) of latency
DECL_RAM_DP_RW_R_0(
  uint16_t,     // Element type
  screen_xy_ram, // RAM function name
  2,            // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

uint16_t screen_xy_ram_update(
	uint1_t write0_address,
	uint16_t write0_value,
	uint1_t write0_enable,
	uint1_t read1_address,
	uint1_t read1_enable
) {
	static uint32_t rwaddr0, rdaddr1;
	static uint16_t wdata;   // Write data, start writing zeros
	static uint16_t rdvalue1;
	rwaddr0 = (uint32_t)(write0_address);
	rdaddr1 = (uint32_t)(read1_address);
	wdata = write0_value;
	
	// The RAM instance
	uint1_t rw_valid = 1; // Always have valid RAM inputs
	uint1_t rd_valid = 1; // Always have valid RAM inputs
	screen_xy_ram_outputs_t screen_xy_ram_out = screen_xy_ram(
		rwaddr0,       // read-write_addr
		write0_value,  // write data
		write0_enable, // write enable (RW port always writing (not reading)),
		rw_valid,      // valid read-write input
		rdaddr1,        	   // read address
		rd_valid       // valid read input
	);
		
	rdvalue1 = screen_xy_ram_out.valid1 & read1_enable ? screen_xy_ram_out.rd_data1 : 0;
	return rdvalue1;
}

uint16_t x_get() {
	return screen_xy_ram_update(0, 0, 0, 0, 1);
}

void x_set(uint16_t value) {
	screen_xy_ram_update(0, value, 1, 0, 0);
}

uint16_t y_get() {
	return screen_xy_ram_update(0, 0, 0, 1, 1);
}

void y_set(uint16_t value) {
	screen_xy_ram_update(1, value, 1, 0, 0);
}


// Declare Device RAM (256 bytes)
// dual port, first port is read+write,
// second port is read only
// 0 cycle(s) of latency
DECL_RAM_DP_RW_R_1(
  uint8_t,    // Element type
  device_ram, // RAM function name
  256,        // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

uint16_t device_ram_update(
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
	device_ram_outputs_t device_ram_out = device_ram(
		rwaddr,            // read-write_addr
		write0_value,      // write data
		write0_enable,     // write enable (RW port always writing (not reading))
		rw_valid,          // valid read-write input
		read0_enable,      // read-write (read) enable 0
		rdaddr,            // read address
		rd_valid,          // valid read input
		read1_enable	   // read enable 1
	);
		
	rdvalue0 = device_ram_out.valid0 & read0_enable ? device_ram_out.rd_data0 : 0;
	rdvalue1 = device_ram_out.valid1 & read1_enable ? device_ram_out.rd_data1 : 0;
	
	result = (uint16_t)(rdvalue0);
	result <<= 8;
	result |= ((uint16_t)(rdvalue1));
	
	return result;
}

uint8_t peek_dev(uint8_t address) {
	static uint16_t ram_read;
	ram_read = device_ram_update(0, 0, 0, 0, address, 1);
	return (uint8_t)(ram_read);
}

void poke_dev(uint8_t address, uint8_t value) {
	device_ram_update(address, value, 1, 0, 0, 0);
}

uint16_t peek2_dev(uint8_t address) {
	return device_ram_update(address, 0, 0, 1, address + 1, 1);
}