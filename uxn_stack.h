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
// 0 cycle(s) of latency
DECL_RAM_DP_RW_R_0(
  uint8_t,    // Element type
  stack_w_ram, // RAM function name
  256,        // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

// Declare Return Stack RAM (256 bytes)
// dual port, first port is read+write,
// second port is read only
// 0 cycle(s) of latency
DECL_RAM_DP_RW_R_0(
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

uint8_t stack_w_ram_read(uint8_t address) {
	static uint32_t stack_w_r_rdaddr = 0; // Read address
	static uint32_t stack_w_r_rwaddr = 0; // Write+read address
	static uint8_t stack_w_r_wdata = 0; // Write data, start writing zeros
	stack_w_r_rdaddr = (uint32_t)(address);
	
	  // The RAM instance
	  uint1_t stack_w_r_wr_en = 1; // RW port always writing (not reading)
	  uint1_t stack_w_r_rw_valid = 1; // Always have valid RAM inputs
	  uint1_t stack_w_r_rd_valid = 1; // Always have valid RAM inputs
	  stack_w_ram_outputs_t stack_w_r_ram_out = stack_w_ram(
		stack_w_r_rwaddr,
		stack_w_r_wdata,
		stack_w_r_wr_en,
		stack_w_r_rw_valid,
		stack_w_r_rdaddr,
		stack_w_r_rd_valid
	);
		
	return stack_w_r_ram_out.rd_data1;
}

void stack_w_ram_write(uint8_t address, uint8_t value) {
	static uint32_t stack_w_w_rdaddr = 0; // Read address
	static uint32_t stack_w_w_rwaddr = 0; // Write+read address
	static uint8_t stack_w_w_wdata = 0;   // Write data
	stack_w_w_rwaddr = (uint32_t)(address);
	stack_w_w_wdata = value;
	
	  // The RAM instance
	  uint1_t stack_w_w_wr_en = 1; // RW port always writing (not reading)
	  uint1_t stack_w_w_rw_valid = 1; // Always have valid RAM inputs
	  uint1_t stack_w_w_rd_valid = 1; // Always have valid RAM inputs
	  stack_w_ram_outputs_t stack_w_w_ram_out = stack_w_ram(
		stack_w_w_rwaddr,
		stack_w_w_wdata,
		stack_w_w_wr_en,
		stack_w_w_rw_valid,
		stack_w_w_rdaddr,
		stack_w_w_rd_valid
	);
}

uint8_t stack_r_ram_read(uint8_t address) {
	static uint32_t stack_r_r_rdaddr = 0; // Read address
	static uint32_t stack_r_r_rwaddr = 0; // Write+read address
	static uint8_t  stack_r_r_wdata = 0;  // Write data, start writing zeros
	stack_r_r_rdaddr = (uint32_t)(address);
	
	  // The RAM instance
	  uint1_t stack_r_r_wr_en = 1; // RW port always writing (not reading)
	  uint1_t stack_r_r_rw_valid = 1; // Always have valid RAM inputs
	  uint1_t stack_r_r_rd_valid = 1; // Always have valid RAM inputs
	  stack_r_ram_outputs_t stack_r_r_ram_out = stack_r_ram(
		stack_r_r_rwaddr,
		stack_r_r_wdata,
		stack_r_r_wr_en,
		stack_r_r_rw_valid,
		stack_r_r_rdaddr,
		stack_r_r_rd_valid
	);
		
	return stack_r_r_ram_out.rd_data1;
}

void stack_r_ram_write(uint8_t address, uint8_t value) {
	static uint32_t stack_r_w_rdaddr = 0; // Read address
	static uint32_t stack_r_w_rwaddr = 0; // Write+read address
	static uint8_t  stack_r_w_wdata = 0;  // Write data
	stack_r_w_rwaddr = (uint32_t)(address);
	stack_r_w_wdata = value;
	
	  // The RAM instance
	  uint1_t stack_r_w_wr_en = 1; // RW port always writing (not reading)
	  uint1_t stack_r_w_rw_valid = 1; // Always have valid RAM inputs
	  uint1_t stack_r_w_rd_valid = 1; // Always have valid RAM inputs
	  stack_r_ram_outputs_t stack_r_w_ram_out = stack_r_ram(
		stack_r_w_rwaddr,
		stack_r_w_wdata,
		stack_r_w_wr_en,
		stack_r_w_rw_valid,
		stack_r_w_rdaddr,
		stack_r_w_rd_valid
	);
}

uint8_t stack_ptr_ram_read(uint1_t stack_index) {
	static uint32_t stack_p_r_rdaddr = 0; // Read address
	static uint32_t stack_p_r_rwaddr = 0; // Write+read address
	static uint8_t  stack_p_r_wdata = 0;  // Write data, start writing zeros
	stack_p_r_rdaddr = (uint32_t)(stack_index);
	
	  // The RAM instance
	  uint1_t stack_p_r_wr_en = 1; // RW port always writing (not reading)
	  uint1_t stack_p_r_rw_valid = 1; // Always have valid RAM inputs
	  uint1_t stack_p_r_rd_valid = 1; // Always have valid RAM inputs
	  stack_p_ram_outputs_t stack_p_r_ram_out = stack_p_ram(
		stack_p_r_rwaddr,
		stack_p_r_wdata,
		stack_p_r_wr_en,
		stack_p_r_rw_valid,
		stack_p_r_rdaddr,
		stack_p_r_rd_valid
	);
		
	return stack_p_r_ram_out.rd_data1;
}

void stack_ptr_ram_write(uint1_t stack_index, uint8_t value) {
	static uint32_t stack_p_w_rdaddr = 0; // Read address
	static uint32_t stack_p_w_rwaddr = 0; // Write+read address
	static uint8_t  stack_p_w_wdata = 0;  // Write data
	stack_p_w_rwaddr = (uint32_t)(stack_index);
	stack_p_w_wdata = value;
	
	  // The RAM instance
	  uint1_t stack_p_w_wr_en = 1; // RW port always writing (not reading)
	  uint1_t stack_p_w_rw_valid = 1; // Always have valid RAM inputs
	  uint1_t stack_p_w_rd_valid = 1; // Always have valid RAM inputs
	  stack_p_ram_outputs_t stack_p_w_ram_out = stack_p_ram(
		stack_p_w_rwaddr,
		stack_p_w_wdata,
		stack_p_w_wr_en,
		stack_p_w_rw_valid,
		stack_p_w_rdaddr,
		stack_p_w_rd_valid
	);
}

void stack_data_set(uint1_t stack_index, uint8_t index, uint8_t value) {
	if (stack_index == 0) {
		stack_w_ram_write(index, value);
	} else {
		stack_r_ram_write(index, value);
	}
}

uint8_t stack_data_get(uint1_t stack_index, uint8_t index) {
	static uint8_t stack_data_ret_value;
	if (stack_index == 0) {
		stack_data_ret_value = stack_w_ram_read(index);
	} else {
		stack_data_ret_value = stack_r_ram_read(index);
	}
	
	return stack_data_ret_value;
}

void stack_pointer_set(uint1_t stack_index, uint8_t value) {
	stack_ptr_ram_write(stack_index, value);
}

uint8_t stack_pointer_get(uint1_t stack_index) {
	return stack_ptr_ram_read(stack_index);
}

void stack_pointer_move(uint1_t stack_index, uint8_t adjustment, uint1_t is_negative) {
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
	// stack_index: 0 = working stack, 1 = return stack
	static int32_t mem0; 
	static uint16_t mem1; 
	static uint16_t result;
	// cast to int32 to silence warning about integer promotion during bit shift
	mem0 = (int32_t)(stack_data_get(stack_index, address));
	mem1 = (uint16_t)(stack_data_get(stack_index, address + 1));
	result = (uint16_t)(mem0 << 8) | mem1;
	return result;
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
		stack_pointer_move(stack_index, 2, 0);
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
		stack_pointer_move(stack_index, 1, 0);
	}
	
	return result;
}

uint8_t set(uint1_t stack_index, uint8_t ins, uint8_t k, uint8_t mul, int8_t add) {
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

void put_stack(uint1_t stack_index, uint8_t offset, uint8_t value) {
	// PUT(o, v) { s->dat[(Uint8)(s->ptr - 1 - (o))] = (v); }
	static uint8_t put_tmp;
	put_tmp = stack_pointer_get(stack_index) - 1 - offset;
	stack_data_set(stack_index, put_tmp, value);
}

void put2_stack(uint1_t stack_index, uint8_t offset, uint16_t value) {
	// PUT2(o, v) { tmp = (v); s->dat[(Uint8)(s->ptr - o - 2)] = tmp >> 8; s->dat[(Uint8)(s->ptr - o - 1)] = tmp; }
	static uint16_t put2_tmp;
	static uint8_t put2_tmp8;
	put2_tmp8 = stack_pointer_get(stack_index) - offset - 2;
	put2_tmp = value;
	stack_data_set(stack_index, put2_tmp8, (uint8_t)(put2_tmp >> 8));
	stack_data_set(stack_index, put2_tmp8 + 1, (uint8_t)(put2_tmp));
} 
