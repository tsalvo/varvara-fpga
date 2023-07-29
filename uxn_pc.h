#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

// Declare Program Counter RAM (2 bytes)
// dual port, first port is read+write,
// second port is read only
// 0 cycle(s) of latency
DECL_RAM_DP_RW_R_0(
  uint16_t,    // Element type
  prog_ctr_ram, // RAM function name
  1,           // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

uint16_t prog_ctr_ram_read() {
	static uint32_t prog_ctr_r_rdaddr = 0; // Read address
	static uint32_t prog_ctr_r_rwaddr = 0; // Write+read address
	static uint8_t  prog_ctr_r_wdata = 0;  // Write data, start writing zeros
	
	  // The RAM instance
	  uint1_t prog_ctr_r_wr_en = 1; // RW port always writing (not reading)
	  uint1_t prog_ctr_r_rw_valid = 1; // Always have valid RAM inputs
	  uint1_t prog_ctr_r_rd_valid = 1; // Always have valid RAM inputs
	  prog_ctr_ram_outputs_t prog_ctr_r_ram_out = prog_ctr_ram(
		prog_ctr_r_rwaddr,
		prog_ctr_r_wdata,
		prog_ctr_r_wr_en,
		prog_ctr_r_rw_valid,
		prog_ctr_r_rdaddr,
		prog_ctr_r_rd_valid
	);
		
	return prog_ctr_r_ram_out.rd_data1;
}

void prog_ctr_ram_write(uint16_t value) {
	static uint32_t prog_ctr_w_rdaddr = 0; // Read address
	static uint32_t prog_ctr_w_rwaddr = 0; // Write+read address
	static uint16_t prog_ctr_w_wdata = 0;  // Write data
	prog_ctr_w_wdata = value;
	
	  // The RAM instance
	  uint1_t prog_ctr_w_wr_en = 1; // RW port always writing (not reading)
	  uint1_t prog_ctr_w_rw_valid = 1; // Always have valid RAM inputs
	  uint1_t prog_ctr_w_rd_valid = 1; // Always have valid RAM inputs
	  prog_ctr_ram_outputs_t prog_ctr_w_ram_out = prog_ctr_ram(
		prog_ctr_w_rwaddr,
		prog_ctr_w_wdata,
		prog_ctr_w_wr_en,
		prog_ctr_w_rw_valid,
		prog_ctr_w_rdaddr,
		prog_ctr_w_rd_valid
	);
}

uint16_t pc_get() {
	return prog_ctr_ram_read();
}

void pc_add(uint16_t adjustment) {
	static uint16_t current_prog_ctr;
	current_prog_ctr = prog_ctr_ram_read();
	prog_ctr_ram_write(current_prog_ctr + adjustment);
}

void pc_add_s8(int8_t adjustment) {
	static uint16_t current_prog_ctr;
	static uint16_t new_prog_ctr;
	current_prog_ctr = prog_ctr_ram_read();
	new_prog_ctr = current_prog_ctr + adjustment;
	prog_ctr_ram_write(new_prog_ctr);
}

void pc_set(uint16_t value) {
	prog_ctr_ram_write(value);
}