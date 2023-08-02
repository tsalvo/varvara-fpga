#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

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

uint8_t device_ram_read(uint8_t address) {
	static uint32_t device_r_rdaddr; // Read address
	device_r_rdaddr = (uint32_t)(address);
	
	device_ram_outputs_t device_r_ram_out = device_ram(
		0,                 // read-write_addr
		0,                 // write data
		0,                 // write enable (RW port always writing (not reading))
		0,                 // valid read-write input
		0,                 // read-write (read) enable 0
		device_r_rdaddr,   // read address
		1,                 // valid read input
		1				   // read enable 1
	);
		
	return device_r_ram_out.rd_data1;
}

void device_ram_write(uint8_t address, uint8_t value) {
	static uint32_t device_w_rwaddr; // Write+read address
	static uint8_t device_w_wdata; // Write data
	device_w_rwaddr = (uint32_t)(address);
	device_w_wdata = value;
	
	// The RAM instance
	device_ram_outputs_t device_w_ram_out = device_ram(
		device_w_rwaddr,   // read-write_addr
		device_w_wdata,    // write data
		1,                  // write enable (RW port always writing (not reading))
		1,                  // valid read-write input
		0,                  // read-write (read) enable 0
		0,                  // read address
		1,                  // valid read input
		0				    // read enable 1
	);
}

uint16_t peek2_dev(uint8_t address) {
	static int32_t mem0; 
	static uint16_t mem1; 
	static uint16_t result;
	mem0 = (int32_t)(device_ram_read(address));
	mem1 = (uint16_t)(device_ram_read(address + 1));
	result = (uint16_t)(mem0 << 8) | mem1;
	return result;
}