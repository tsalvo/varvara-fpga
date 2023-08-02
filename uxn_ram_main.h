#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

// Declare Main RAM (64KB)
// dual port, first port is read+write,
// second port is read only
// 1 cycle(s) of latency
DECL_RAM_DP_RW_R_1(
  uint8_t,  // Element type
  main_ram, // RAM function name
  65536,    // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

uint8_t main_ram_read(uint16_t address) {
	static uint32_t main_r_rdaddr;       // Read address
	main_r_rdaddr = (uint32_t)(address); // read 
	
	// The RAM instance
	main_ram_outputs_t main_r_ram_out = main_ram(
		0,              // read-write_addr
		0,              // write data
		0,              // write enable (RW port always writing (not reading))
		0,              // valid read-write input
		0,              // read-write (read) enable 0
		main_r_rdaddr,  // read address
		1,              // valid read input
		1				// read enable 1
	);
		
	return main_r_ram_out.rd_data1;
}

void main_ram_write(uint16_t address, uint8_t value) {
	static uint32_t main_w_rwaddr = 0; // Write+read address
	static uint8_t  main_w_wdata = 0;  // Write data
	main_w_rwaddr = (uint32_t)(address);
	main_w_wdata = value;
	
	// The RAM instance
	main_ram_outputs_t main_w_ram_out = main_ram(
		main_w_rwaddr,   // read-write_addr
		main_w_wdata,    // write data
		1,               // write enable (RW port always writing (not reading))
		1,               // valid read-write input
		0,               // read-write (read) enable 0
		0,               // read address
		1,               // valid read input
		0				 // read enable 1
	);
}

uint16_t peek2_ram(uint16_t address) {
	static int32_t mem0; 
	static uint16_t mem1; 
	static uint16_t result; 
	// cast to int32 to silence warning about integer promotion during bit shift
	mem0 = (int32_t)(main_ram_read(address));
	mem1 = (uint16_t)(main_ram_read(address + 1));
	result = (uint16_t)(mem0 << 8) | mem1;
	return result;
}

void poke2_ram(uint16_t address, uint16_t value) {
	main_ram_write(address, (uint8_t)(value >> 8));
	main_ram_write(address + 1, (uint8_t)(value));
}