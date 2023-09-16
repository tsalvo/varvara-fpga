#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "intN_t.h"   // intN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

#pragma once
#include "uxn_ram_device.h"

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

// Declare Stack Pointer RAM (2 bytes)
// dual port, first port is read+write,
// second port is read only
// 0 cycle(s) of latency
DECL_RAM_DP_RW_R_0(
  uint8_t,    // Element type
  stack_p_ram, // RAM function name
  2,        // Number of elements
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
		
	rdvalue0 = /*stack_w_ram_out.valid0 & */read0_enable ? stack_w_ram_out.rd_data0 : 0;
	rdvalue1 = /*stack_w_ram_out.valid1 & */read1_enable ? stack_w_ram_out.rd_data1 : 0;
	
	result = (uint16_t)(rdvalue0);
	result <<= 8;
	result |= ((uint16_t)(rdvalue1));
	
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
		
	rdvalue0 = /*stack_r_ram_out.valid0 & */read0_enable ? stack_r_ram_out.rd_data0 : 0;
	rdvalue1 = /*stack_r_ram_out.valid1 & */read1_enable ? stack_r_ram_out.rd_data1 : 0;
	
	result = (uint16_t)(rdvalue0);
	result <<= 8;
	result |= ((uint16_t)(rdvalue1));
	
	return result;
}

uint16_t stack_p_ram_update(
	uint1_t address0, 
	uint8_t write0_value,
	uint1_t write0_enable,
	uint1_t read0_enable,
	uint1_t address1,
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
	stack_p_ram_outputs_t stack_p_ram_out = stack_p_ram(
		rwaddr,        // read-write_addr
		write0_value,  // write data
		write0_enable, // write enable (RW port always writing (not reading)),
		rw_valid,      // valid read-write input
		rdaddr,        // read address
		rd_valid       // valid read input
	);
		
	rdvalue0 = /*stack_p_ram_out.valid0 & */read0_enable ? stack_p_ram_out.rd_data0 : 0;
	rdvalue1 = /*stack_p_ram_out.valid1 & */read1_enable ? stack_p_ram_out.rd_data1 : 0;
	
	result = (uint16_t)(rdvalue0);
	result <<= 8;
	result |= ((uint16_t)(rdvalue1));
	
	return result;
}

uint8_t peek_stack_w(uint8_t address) {
	static uint16_t ram_read;
	ram_read = stack_w_ram_update(0, 0, 0, 0, address, 1);
	return (uint8_t)(ram_read);
}

uint8_t peek_stack_r(uint8_t address) {
	static uint16_t ram_read;
	ram_read = stack_r_ram_update(0, 0, 0, 0, address, 1);
	return (uint8_t)(ram_read);
}

void poke_stack_w(uint8_t address, uint8_t value) {
	stack_w_ram_update(address, value, 1, 0, 0, 0);
}

void poke_stack_r(uint8_t address, uint8_t value) {
	stack_r_ram_update(address, value, 1, 0, 0, 0);
}

uint16_t peek2_stack_w(uint8_t address) {
	return stack_w_ram_update(address, 0, 0, 1, address + 1, 1);
}

uint16_t peek2_stack_r(uint8_t address) {
	return stack_r_ram_update(address, 0, 0, 1, address + 1, 1);
}

void stack_data_set(uint1_t stack_index, uint8_t address, uint8_t value) {
	if (stack_index == 0) {
		poke_stack_w(address, value);
	} else {
		poke_stack_r(address, value);
	}
}

uint8_t stack_data_get(uint1_t stack_index, uint8_t address) {
	static uint8_t stack_data_ret_value;
	if (stack_index == 0) {
		stack_data_ret_value = peek_stack_w(address);
	} else {
		stack_data_ret_value = peek_stack_r(address);
	}
	
	return stack_data_ret_value;
}

uint16_t stack_data_get2(uint1_t stack_index, uint8_t address) {
	static uint16_t stack_data_ret_value;
	if (stack_index == 0) {
		stack_data_ret_value = peek2_stack_w(address);
	} else {
		stack_data_ret_value = peek2_stack_r(address);
	}
	
	return stack_data_ret_value;
}

void stack_pointer_set(uint1_t stack_index, uint8_t value) {
	stack_p_ram_update(stack_index, value, 1, 0, 0, 0);
}

uint8_t stack_pointer_get(uint1_t stack_index) {
	static uint16_t ram_read;
	ram_read = stack_p_ram_update(0, 0, 0, 0, stack_index, 1);
	return (uint8_t)(ram_read);
}

// TODO: remove when no longer used
void stack_pointer_move_old(uint1_t stack_index, uint8_t adjustment, uint1_t is_negative) {
	static uint8_t stack_ptr_existing;
	static uint8_t stack_ptr_new;
	stack_ptr_existing = stack_pointer_get(stack_index);
	if (is_negative) {
		stack_ptr_new = stack_ptr_existing - adjustment;
	} else {
		stack_ptr_new = stack_ptr_existing + adjustment;
	}
	
	stack_pointer_set(stack_index, stack_ptr_new);
}

uint16_t peek2_stack(uint1_t stack_index, uint8_t address) {
	return stack_data_get2(stack_index, address);
}

