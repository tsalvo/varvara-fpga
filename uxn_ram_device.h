#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

// Declare Device RAM (256 bytes)
// dual port, first port is read+write,
// second port is read only
// 0 cycle(s) of latency
DECL_RAM_DP_RW_R_0(
  uint8_t,    // Element type
  device_ram, // RAM function name
  256,        // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

uint8_t device_ram_read(uint8_t address) {
	static uint32_t device_r_rdaddr = 0; // Read address
	static uint32_t device_r_rwaddr = 0; // Write+read address
	static uint8_t  device_r_wdata = 0;  // Write data, start writing zeros
	device_r_rdaddr = (uint32_t)(address);
	
	  // The RAM instance
	  uint1_t device_r_wr_en = 1; // RW port always writing (not reading)
	  uint1_t device_r_rw_valid = 1; // Always have valid RAM inputs
	  uint1_t device_r_rd_valid = 1; // Always have valid RAM inputs
	  device_ram_outputs_t device_r_ram_out = device_ram(
		device_r_rwaddr,
		device_r_wdata,
		device_r_wr_en,
		device_r_rw_valid,
		device_r_rdaddr,
		device_r_rd_valid
	);
		
	return device_r_ram_out.rd_data1;
}

void device_ram_write(uint8_t address, uint8_t value) {
	static uint32_t device_w_rdaddr = 0; // Read address
	static uint32_t device_w_rwaddr = 0; // Write+read address
	static uint8_t device_w_wdata = 0; // Write data
	device_w_rwaddr = (uint32_t)(address);
	device_w_wdata = value;
	
	  // The RAM instance
	  uint1_t device_w_wr_en = 1; // RW port always writing (not reading)
	  uint1_t device_w_rw_valid = 1; // Always have valid RAM inputs
	  uint1_t device_w_rd_valid = 1; // Always have valid RAM inputs
	  device_ram_outputs_t device_w_ram_out = device_ram(
		device_w_rwaddr,
		device_w_wdata,
		device_w_wr_en,
		device_w_rw_valid,
		device_w_rdaddr,
		device_w_rd_valid
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