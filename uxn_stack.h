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
		
	rdvalue0 = stack_w_ram_out.valid0 & read0_enable ? stack_w_ram_out.rd_data0 : 0;
	rdvalue1 = stack_w_ram_out.valid1 & read1_enable ? stack_w_ram_out.rd_data1 : 0;
	
	return (uint16_t)((rdvalue0 << 8) | rdvalue1);
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
		
	rdvalue0 = stack_r_ram_out.valid0 & read0_enable ? stack_r_ram_out.rd_data0 : 0;
	rdvalue1 = stack_r_ram_out.valid1 & read1_enable ? stack_r_ram_out.rd_data1 : 0;
	
	return (uint16_t)((rdvalue0 << 8) | rdvalue1);
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
		
	rdvalue0 = stack_p_ram_out.valid0 & read0_enable ? stack_p_ram_out.rd_data0 : 0;
	rdvalue1 = stack_p_ram_out.valid1 & read1_enable ? stack_p_ram_out.rd_data1 : 0;
	
	return (uint16_t)((rdvalue0 << 8) | rdvalue1);
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

// TODO: deprecated
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

uint8_t uxn_halt(uint8_t instr, uint8_t err, uint16_t addr)
{
	// TODO: implement
	static uint16_t halt_handler;
	static uint8_t result;
	halt_handler = peek2_dev(0);
	result = 0;
	if (halt_handler) {
		stack_pointer_set(0, 4);
		stack_data_set(0, 0, (uint8_t)(addr >> 8));
		stack_data_set(0, 1, (uint8_t)(addr & 0x00FF));
		stack_data_set(0, 2, instr);
		stack_data_set(0, 3, err);
		// result = uxn_eval(halt_handler); // TODO: recursion?
	} else {
		// system_inspect(u);
		// fprintf(stderr, "%s %s, by %02x at 0x%04x.\n", (instr & 0x40) ? "Return-stack" : "Working-stack", errors[err - 1], instr, addr);
		result = 0;
	}
	
	return result;
}

uint8_t halt(uint8_t ins, uint8_t err) {
	// HALT(c) { return uxn_halt(u, ins, (c), pc - 1); }
	// Ex: HALT(3)
	return uxn_halt(ins, err, pc_get() - 1);
}

uint8_t push2_stack(uint1_t stack_index, uint8_t ins, uint16_t value) {
	static uint16_t tmp = 0;
	static uint8_t result = 0;
	static uint1_t halt_return = 0;
	
	if (stack_pointer_get(stack_index) > 253) {
		result = halt(ins, 2);
		halt_return = 1;
	} else {
		result = 0;
		halt_return = 0;
	}
	
	if (halt_return == 0) {
		tmp = value;
		stack_data_set(stack_index, stack_pointer_get(stack_index), (uint8_t)(tmp >> 8));
		stack_data_set(stack_index, stack_pointer_get(stack_index) + 1, (uint8_t)(tmp));
		stack_pointer_move_old(stack_index, 2, 0);
	}
	
	return result;
}

uint8_t push_stack(uint1_t stack_index, uint8_t ins, uint8_t value) {
	
	static uint8_t result;
	static uint1_t halt_return;
	
	if (stack_pointer_get(stack_index) > 254) {
		result = halt(ins, 2);
		halt_return = 1;
	} else {
		result = 0;
		halt_return = 0;
	}
	
	if (halt_return == 0) {
		stack_data_set(stack_index, stack_pointer_get(stack_index), value);
		stack_pointer_move_old(stack_index, 1, 0);
	}
	
	return result;
}

// TODO: remove when no longer used
uint8_t set_old(uint1_t stack_index, uint8_t ins, uint8_t k, uint8_t mul, int8_t add) {
	static uint8_t result, set_tmp;
	static uint1_t halt_return;
	if (mul > stack_pointer_get(stack_index)) {
		result = halt(ins, 1);
		halt_return = 1;
	} else {
		result = 0;
		halt_return = 0;
	}
	
	set_tmp = (mul & k) + add + stack_pointer_get(stack_index);
	if (set_tmp > 254) {
		result = halt(ins, 2);
		halt_return = 1;
	}
	
	if (halt_return == 0) {
		stack_pointer_set(stack_index, set_tmp);
	}
	
	return result;
}

uint1_t set_will_fail(uint8_t sp, uint8_t k, uint8_t mul, int8_t add) {
	static uint1_t condition0, condition1;
	condition0 = mul > sp ? 1 : 0;
	condition1 = (mul & k) + add + sp > 254 ? 1 : 0;
	return condition0 | condition1;
}

uint1_t set_will_succeed(uint8_t sp, uint8_t k, uint8_t mul, int8_t add) {
	static uint1_t condition0, condition1;
	condition0 = mul > sp ? 0 : 1;
	condition1 = (mul & k) + add + sp > 254 ? 0 : 1;
	
	return condition0 & condition1;
}

// TODO: replace set_old()
// precondition: set_will_fail returned 0
void set(uint8_t sp, uint1_t stack_index, uint8_t ins, uint8_t k, uint8_t mul, int8_t add) {
	static uint8_t result, set_tmp;
	set_tmp = (mul & k) + add + sp;
	stack_pointer_set(stack_index, set_tmp);
}

void put_stack(uint8_t sp, uint1_t stack_index, uint8_t offset, uint8_t value) {
	static uint8_t put_tmp;
	put_tmp = sp - 1 - offset;
	stack_data_set(stack_index, put_tmp, value);
}

void put_stack_old(uint1_t stack_index, uint8_t offset, uint8_t value) {
	static uint8_t put_tmp;
	put_tmp = stack_pointer_get(stack_index) - 1 - offset;
	stack_data_set(stack_index, put_tmp, value);
}

void put2_stack(uint1_t stack_index, uint8_t offset, uint16_t value) {
	static uint16_t put2_tmp;
	static uint8_t put2_tmp8;
	put2_tmp8 = stack_pointer_get(stack_index) - offset - 2;
	put2_tmp = value;
	stack_data_set(stack_index, put2_tmp8, (uint8_t)(put2_tmp >> 8));
	stack_data_set(stack_index, put2_tmp8 + 1, (uint8_t)(put2_tmp));
} 
