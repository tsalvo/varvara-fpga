#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

// 400 x 360
#define SCREEN_RAM_SIZE 144000
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
	uint32_t write0_address, 
	uint2_t write0_value,
	uint1_t write0_enable,
	uint32_t read1_address,
	uint1_t read1_enable
) {
	static uint32_t rdaddr = 0; // Read address
	static uint32_t rwaddr = 0; // Write+read address
    static uint2_t wdata = 0;   // Write data, start writing zeros
	static uint2_t bg_vram_result = 0;
	rwaddr = write0_address;
	wdata = write0_value;
	rdaddr = read1_address;

	// The RAM instance
	uint1_t rw_valid = write0_enable;
	uint1_t rd_valid = read1_enable; 
	background_vram_outputs_t bg_vram_out = background_vram(
		rwaddr, 
		wdata, 
		write0_enable, 
		rw_valid, 
		0, // read0 enable
		rdaddr, 
		rd_valid, 
		read1_enable // read1 enable
	);
	
	bg_vram_result = bg_vram_out.rd_data1;
		
	printf("     VRAM rwaddr = 0x%X, write0_enable = 0x%X, wdata = 0x%X, rdaddr = 0x%X, read1_enable = 0x%X, result = 0x%X\n", rwaddr, write0_enable, (uint4_t)(wdata), rdaddr, read1_enable, (uint4_t)(bg_vram_result));

	// second port always reading
	// connected to output
	return bg_vram_result;
}
