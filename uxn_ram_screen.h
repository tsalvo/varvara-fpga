#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

// 320 x 240
#define SCREEN_RAM_SIZE 76800
// Declare Screen RAM
// dual port, first port is read+write,
// second port is read only
// 1 cycle(s) of latency
DECL_RAM_DP_RW_R_1(
  uint2_t,      		// Element type
  background_vram,   	// RAM function name
  SCREEN_RAM_SIZE,      // Number of elements
  RAM_INIT_INT_ZEROS 	// Initial value VHDL string, from ram.h
)

uint2_t background_vram_update(
	uint32_t write_address0, 
	uint2_t write_value0,
	uint1_t write_enable0,
	uint1_t read_enable0,
	uint32_t read_address1
) {
	static uint32_t rdaddr; // Read address
	static uint32_t rwaddr; // Write+read address
    static uint2_t wdata;   // Write data, start writing zeros
	rwaddr = write_address0;
	wdata = write_value0;
	rdaddr = read_address1;

	// The RAM instance
	uint1_t wr_en = write_enable0;
	uint1_t rw_valid = 1;
	uint1_t rw_out_en = read_enable0; 
	uint1_t rd_valid = 1; // Always have valid RAM inputs
	uint1_t rd_out_en = 1; // Always ready for RAM outputs
	background_vram_outputs_t ram_out = background_vram(
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
