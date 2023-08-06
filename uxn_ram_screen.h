#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

#define SCREEN_RAM_SIZE 204800 // 512*400
// Declare Screen RAM
// dual port, first port is read+write,
// second port is read only
// 1 cycle(s) of latency
DECL_RAM_DP_RW_R_1(
  uint2_t,      		// Element type
  screen_ram,   		// RAM function name
  SCREEN_RAM_SIZE,      // Number of elements
  RAM_INIT_INT_ZEROS 	// Initial value VHDL string, from ram.h
)

uint2_t screen_ram_update(
	uint32_t write_address, 
	uint2_t write_value,
	uint32_t read_address
) {
	static uint32_t rdaddr; // Read address
	static uint32_t rwaddr; // Write+read address
    static uint2_t wdata;   // Write data, start writing zeros
	rwaddr = write_address;
	wdata = write_value;
	rdaddr = read_address;

	// The RAM instance
	uint1_t wr_en = 1; // RW port always writing (not reading)
	uint1_t rw_valid = 1; // Always have valid RAM inputs
	uint1_t rw_out_en = 1; // Always ready for RAM outputs
	uint1_t rd_valid = 1; // Always have valid RAM inputs
	uint1_t rd_out_en = 1; // Always ready for RAM outputs
	screen_ram_outputs_t ram_out = screen_ram(
		rwaddr, 
		wdata, 
		wr_en, 
		rw_valid, 
		rw_out_en, 
		rdaddr, 
		rd_valid, 
		rd_out_en
	);

	// second port always reading
	// connected to output
	return ram_out.rd_data1;
}

