#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

// Declare Program Counter RAM (2 bytes)
// dual port, first port is read+write,
// second port is read only
// 0 cycle(s) of latency
DECL_RAM_DP_RW_R_0(
  uint16_t,     // Element type
  prog_ctr_ram, // RAM function name
  1,            // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

uint16_t prog_ctr_ram_update(
	uint16_t write0_value,
	uint1_t write0_enable,
	uint1_t read1_enable
) {
	static uint16_t wdata;   // Write data, start writing zeros
	static uint16_t rdvalue1;
	wdata = write0_value;
	
	// The RAM instance
	uint1_t rw_valid = 1; // Always have valid RAM inputs
	uint1_t rd_valid = 1; // Always have valid RAM inputs
	prog_ctr_ram_outputs_t prog_ctr_ram_out = prog_ctr_ram(
		0,        	   // read-write_addr
		write0_value,  // write data
		write0_enable, // write enable (RW port always writing (not reading)),
		rw_valid,      // valid read-write input
		0,        	   // read address
		rd_valid       // valid read input
	);
		
	rdvalue1 = prog_ctr_ram_out.valid1 & read1_enable ? prog_ctr_ram_out.rd_data1 : 0;
	return rdvalue1;
}

uint16_t pc_get() {
	return prog_ctr_ram_update(0, 0, 1);
}

void pc_set(uint16_t value) {
	prog_ctr_ram_update(value, 1, 0);
}

void pc_add(uint16_t adjustment) {
	static uint16_t current_prog_ctr;
	current_prog_ctr = pc_get();
	pc_set(current_prog_ctr + adjustment);
}

void pc_add_s8(uint16_t pc, int8_t adjustment) {
	static uint16_t new_prog_ctr;
	new_prog_ctr = pc + adjustment;
	pc_set(new_prog_ctr);
}

// TODO: remove eventually
void pc_add_s8_old(int8_t adjustment) {
	static uint16_t current_prog_ctr;
	static uint16_t new_prog_ctr;
	current_prog_ctr = pc_get();
	new_prog_ctr = current_prog_ctr + adjustment;
	pc_set(new_prog_ctr);
}
