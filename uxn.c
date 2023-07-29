#include "uintN_t.h"  // uintN_t types for any N
#include "intN_t.h"  // intN_t types for any N
#include "ram.h"
#include <stdint.h>

// RULES:
// - cannot write to a global variable from more than one function (unless you use clock domain crossing)
// - no switch statements (C AST node cannot be parsed to logic)
// - only one return per function
// - no ++ or -- operators

// NOTES
// Build with Docker pipelinec image: docker run -v $(pwd):/workdir pipelinec pipelinec uxn.c
// https://github.com/JulianKemmerer/PipelineC/wiki/Global-Variables (Global Variables)

// Declare Main RAM (64KB)
// dual port, first port is read+write,
// second port is read only
// 0 cycle(s) of latency
DECL_RAM_DP_RW_R_0(
  uint8_t,  // Element type
  main_ram, // RAM function name
  65536,    // Number of elements
  RAM_INIT_INT_ZEROS // Initial value VHDL string, from ram.h
)

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

// SHARED
// Global Access 
uint8_t main_ram_read(uint16_t address) {
	static uint32_t main_r_rdaddr = 0; // Read address
	static uint32_t main_r_rwaddr = 0; // Write+read address
	static uint8_t  main_r_wdata = 0;  // Write data, start writing zeros
	main_r_rdaddr = (uint32_t)(address);
	
	  // The RAM instance
	  uint1_t main_r_wr_en = 1; // RW port always writing (not reading)
	  uint1_t main_r_rw_valid = 1; // Always have valid RAM inputs
	  uint1_t main_r_rd_valid = 1; // Always have valid RAM inputs
	  main_ram_outputs_t main_r_ram_out = main_ram(
		main_r_rwaddr, 
		main_r_wdata, 
		main_r_wr_en, 
		main_r_rw_valid, 
		main_r_rdaddr, 
		main_r_rd_valid
	);
		
	return main_r_ram_out.rd_data1;
}

void main_ram_write(uint16_t address, uint8_t value) {
	static uint32_t main_w_rdaddr = 0; // Read address
	static uint32_t main_w_rwaddr = 0; // Write+read address
	static uint8_t  main_w_wdata = 0;  // Write data
	main_w_rwaddr = (uint32_t)(address);
	main_w_wdata = value;
	
	  // The RAM instance
	  uint1_t main_w_wr_en = 1; // RW port always writing (not reading)
	  uint1_t main_w_rw_valid = 1; // Always have valid RAM inputs
	  uint1_t main_w_rd_valid = 1; // Always have valid RAM inputs
	  main_ram_outputs_t main_w_ram_out = main_ram(
		main_w_rwaddr,
		main_w_wdata,
		main_w_wr_en,
		main_w_rw_valid,
		main_w_rdaddr,
		main_w_rd_valid
	);
}

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

uint16_t peek2_dev(uint8_t address) {
	static int32_t mem0; 
	static uint16_t mem1; 
	static uint16_t result;
	mem0 = (int32_t)(device_ram_read(address));
	mem1 = (uint16_t)(device_ram_read(address + 1));
	result = (uint16_t)(mem0 << 8) | mem1;
	return result;
}

uint8_t uxn_halt(uint8_t instr, uint8_t err, uint16_t addr)
{
	// TODO: implement
	static uint16_t uxn_halt_handler;
	static uint8_t uxn_halt_ret_value;
	uxn_halt_handler = peek2_dev(0);
	uxn_halt_ret_value = 0;
	if (uxn_halt_handler) {
		stack_pointer_set(0, 4);
		stack_data_set(0, 0, (uint8_t)(addr >> 8));
		stack_data_set(0, 1, (uint8_t)(addr & 0x00FF));
		stack_data_set(0, 2, instr);
		stack_data_set(0, 3, err);
		// uxn_halt_ret_value = uxn_eval(uxn_halt_handler); // TODO: recursion?
	} else {
		// system_inspect(u);
		// fprintf(stderr, "%s %s, by %02x at 0x%04x.\n", (instr & 0x40) ? "Return-stack" : "Working-stack", errors[err - 1], instr, addr);
		uxn_halt_ret_value = 0;
	}
	
	return uxn_halt_ret_value;
}

uint8_t halt(uint8_t ins, uint8_t err) {
	// HALT(c) { return uxn_halt(u, ins, (c), pc - 1); }
	// Ex: HALT(3)
	return uxn_halt(ins, err, pc_get() - 1);
}

uint8_t push2_stack(uint1_t stack_index, uint8_t ins, uint16_t value) {
	static uint16_t push2_tmp = 0;
	static uint8_t push2_ret_value = 0;
	static uint1_t push2_halt_return = 0;
	
	if (stack_pointer_get(stack_index) > 253) {
		push2_ret_value = halt(ins, 2);
		push2_halt_return = 1;
	} else {
		push2_ret_value = 0;
		push2_halt_return = 0;
	}
	
	if (push2_halt_return == 0) {
		push2_tmp = value;
		stack_data_set(stack_index, stack_pointer_get(stack_index), (uint8_t)(push2_tmp >> 8));
		stack_data_set(stack_index, stack_pointer_get(stack_index) + 1, (uint8_t)(push2_tmp));
		stack_pointer_move(stack_index, 2, 0);
	}
	
	return push2_ret_value;
}

uint8_t push_stack(uint1_t stack_index, uint8_t ins, uint8_t value) {
	
	static uint8_t push_ret_value;
	static uint1_t push_halt_return;
	
	if (stack_pointer_get(stack_index) > 254) {
		push_ret_value = halt(ins, 2);
		push_halt_return = 1;
	} else {
		push_ret_value = 0;
		push_halt_return = 0;
	}
	
	if (push_halt_return == 0) {
		stack_data_set(stack_index, stack_pointer_get(stack_index), value);
		stack_pointer_move(stack_index, 1, 0);
	}
	
	return push_ret_value;
}

uint8_t set(uint1_t stack_index, uint8_t ins, uint8_t k, uint8_t mul, int8_t add) {
	// SET(mul, add) { if(mul > s->ptr) HALT(1) tmp = (mul & k) + add + s->ptr; if(tmp > 254) HALT(2) s->ptr = tmp; }
	// Example: SET(2,1)
	static uint8_t set_ret_value, set_tmp;
	static uint1_t set_halt_return;
	if (mul > stack_pointer_get(stack_index)) {
		set_ret_value = halt(ins, 1);
		set_halt_return = 1;
	} else {
		set_ret_value = 0;
		set_halt_return = 0;
	}
	
	set_tmp = (mul & k) + add + stack_pointer_get(stack_index);
	if (set_tmp > 254) {
		set_ret_value = halt(ins, 2);
		set_halt_return = 1;
	}
	
	if (set_halt_return == 0) {
		stack_pointer_set(stack_index, set_tmp);
	}
	
	return set_ret_value;
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

// REGISTERS

uint8_t t_register(uint1_t stack_index) {
	return stack_data_get(stack_index, stack_pointer_get(stack_index) - 1);
}

uint8_t n_register(uint1_t stack_index) {
	return stack_data_get(stack_index, stack_pointer_get(stack_index) - 2);
}

uint8_t l_register(uint1_t stack_index) {
	return stack_data_get(stack_index, stack_pointer_get(stack_index) - 3);
}

uint16_t h2_register(uint1_t stack_index) {
	return peek2_stack(stack_index, stack_pointer_get(stack_index) - 3);
}

uint16_t t2_register(uint1_t stack_index) {
	return peek2_stack(stack_index, stack_pointer_get(stack_index) - 2);
}

uint16_t n2_register(uint1_t stack_index) {
	return peek2_stack(stack_index, stack_pointer_get(stack_index) - 4);
}

uint16_t l2_register(uint1_t stack_index) {
	return peek2_stack(stack_index, stack_pointer_get(stack_index) - 6);
}

// DEVICE OPERATIONS

uint8_t screen_dei(uint8_t addr) {
	static uint8_t result;
	// TODO: implement
	return result;
}

uint8_t datetime_dei(uint8_t addr) {
	static uint8_t result;
	// TODO: implement
	return result;
}

uint8_t uxn_dei(uint8_t addr) {
	static uint8_t d;
	static uint8_t result;
	d = addr & 0xF0;
	result = 0;
	
	if (d == 0x20) {
		result = screen_dei(addr);
	}
	else if (d == 0xC0) {
		result = datetime_dei(addr);
	} 
	else {
		result = device_ram_read(addr);
	}
	
	return result;
}
	
void dei(uint1_t stack_index, uint8_t stack_offset, uint8_t addr) {
	static uint16_t dei_mask[16] = {0x0000, 0x0000, 0x003c, 0x0014, 0x0014, 0x0014, 0x0014, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x07ff, 0x0000, 0x0000, 0x0000};
	static uint16_t is_event; 
	static uint8_t value;
	is_event = (dei_mask[value >> 4] >> (addr & 0x0F)) & 0x0001;
	value = is_event ? uxn_dei(addr) : device_ram_read(addr);
	put_stack(stack_index, stack_offset, value);
}

void system_deo(uint8_t d, uint8_t port) {
	if (port == 0x03) {
		// TODO: implement
		// system_cmd(u->ram, PEEK2(d + 2));
	} 
	else if (port == 0x05) {
		// TODO: implement
		// system_friend(u->ram, PEEK2(d + 4));
	} 
	else if (port == 0x0E) {
		// TODO: implement
		// system_friend(u->ram, PEEK2(d + 4));
	}
}

void console_deo(uint8_t d, uint8_t port) {
	// TODO: implement
}

void screen_deo(uint8_t d, uint8_t port) {
	// TODO: implement
}

void screen_palette(uint8_t port) {
	// TODO: implement
}

void file_deo(uint1_t file_index, uint8_t d, uint8_t p) {
	// TODO: implement
}

void uxn_deo(uint8_t addr)
{
	static uint8_t port;
	static uint8_t device_index;
	static uint1_t port_range_palette_lo;
	static uint1_t port_range_palette_hi;
	port = addr & 0x0F;
	device_index = addr & 0xF0;
	if (device_index == 0x00) { // system
		system_deo(device_ram_read(device_index), port);
		port_range_palette_lo = port > 0x07 ? 1 : 0;
		port_range_palette_hi = port < 0x0E ? 1 : 0;
		if (port_range_palette_lo & port_range_palette_hi) {
			// set system palette colors
			//----------------------------------
			// color0  color1  color2  color3
			// #fff	   #000    #7ec    #f00
			//----------------------------------
			// #f07f .System/r DEO2
			// #f0e0 .System/g DEO2
			// #f0c0 .System/b DEO2
			screen_palette(device_ram_read(0x08));
		}
	}
	else if (device_index == 0x10) { // console
		console_deo(device_ram_read(device_index), port);
	}
	else if (device_index == 0x20) { // screen
		screen_deo(device_ram_read(device_index), port);
	}
	else if (device_index == 0xA0) { // file 1
		file_deo(0, device_ram_read(device_index), port);
	}
	else if (device_index == 0xB0) { // file 2
		file_deo(1, device_ram_read(device_index), port);
	}
}

void deo(uint8_t device_address, uint8_t value) {
	// #define DEO(a, b) { u->dev[(a)] = (b); if((deo_mask[(a) >> 4] >> ((a) & 0xf)) & 0x1) uxn_deo(u, (a)); }
	static uint16_t deo_mask[16] = {0xff28, 0x0300, 0xc028, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0xa260, 0xa260, 0x0000, 0x0000, 0x0000, 0x0000};
	device_ram_write(device_address, value);
	if ((deo_mask[(device_address) >> 4] >> ((device_address) & 0x0F)) & 0x0001) {
		uxn_deo(device_address); 
	}
}

// INSTRUCTION HANDLING

uint1_t eval_opcode(
	uint1_t stack_index,
	uint8_t opcode,
	uint8_t ins,
	uint8_t k
) {
	static uint8_t 	t8, n8, l8, eval_opcode_tmp;
	static uint16_t t16, n16, l16, eval_opcode_tmp16;
	static uint1_t eval_opcode_ret_value;
	
	eval_opcode_ret_value = 0;
	
	if (opcode == 0x00      /* BRK */) {
		eval_opcode_ret_value = 1;
	}
	else if (opcode == 0xFF /* JCI */) {
		stack_pointer_move(stack_index, 1, 1);
		eval_opcode_tmp = stack_data_get(stack_index, stack_pointer_get(stack_index));
		eval_opcode_tmp16 = (eval_opcode_tmp == 0) ? 0 : peek2_ram(pc_get() + 2);
		pc_add(eval_opcode_tmp16);
	}
	else if (opcode == 0xFE /* JMI */) {    
		pc_add(peek2_ram(pc_get()) + 2);
	}
	else if (opcode == 0xFD /* JSI */) {
		eval_opcode_tmp = push2_stack(1, ins, pc_get() + 2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else { pc_add(peek2_ram(pc_get()) + 2); }
	}
	else if (opcode == 0xFC /* LIT */) {
		eval_opcode_tmp = push_stack(stack_index, ins, main_ram_read(pc_get()));
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else { pc_add(1); }
	}
	else if (opcode == 0xFB /* LIT2 */) {
		eval_opcode_tmp16 = peek2_ram(pc_get());
		eval_opcode_tmp = push2_stack(stack_index, ins, eval_opcode_tmp16);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else { pc_add(2); }
	}
	else if (opcode == 0xFA /* LITr */) {
		eval_opcode_tmp = push_stack(stack_index, ins, main_ram_read(pc_get()));
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else { pc_add(1); }
	}
	else if (opcode == 0xF9 /* LIT2r */) {
		eval_opcode_tmp16 = peek2_ram(pc_get());
		eval_opcode_tmp = push2_stack(stack_index, ins, eval_opcode_tmp16);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else { pc_add(2); }
	}
	else if (opcode == 0x01 /* INC */) {
		t8 = t_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 1, 0);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else { put_stack(stack_index, 0, t8 + 1); }
	}
	else if (opcode == 0x21 /*  */) {
		t16 = t2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, 0);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else { put2_stack(stack_index, 0, t16 + 1); }
	}
	else if (opcode == 0x02 /* POP */) {
		eval_opcode_tmp = set(stack_index, ins, k, 1, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
	}
	else if (opcode == 0x22 /*  */) {
		eval_opcode_tmp = set(stack_index, ins, k, 2, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
	}
	else if (opcode == 0x03 /* NIP */) {
		t8 = t_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else { put_stack(stack_index, 0, t8); }
	}
	else if (opcode == 0x23 /*  */) {
		t16 = t2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else { put2_stack(stack_index, 0, t16); }
	}
	else if (opcode == 0x04 /* SWP */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, 0);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, n8);
			put_stack(stack_index, 1, t8);
		} // stack overflow
	}
	else if (opcode == 0x24 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index); 
		eval_opcode_tmp = set(stack_index, ins, k, 4, 0);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, n16);
			put2_stack(stack_index, 2, t16);  
		}
	}
	else if (opcode == 0x05 /* ROT */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		l8 = l_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 3, 0);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, l8);
			put_stack(stack_index, 1, t8);
			put_stack(stack_index, 2, n8);
		}
	}
	else if (opcode == 0x25 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		l16 = l2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 6, 0);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, l16);
			put2_stack(stack_index, 2, t16);
			put2_stack(stack_index, 4, n16);
		}
	}
	else if (opcode == 0x06 /* DUP */) {
		t8 = t_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 1, 1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, t8);
			put_stack(stack_index, 1, t8);
		}
	}
	else if (opcode == 0x26 /*  */) {
		t16 = t2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, 2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, t16);
			put2_stack(stack_index, 2, t16);    
		}
	}
	else if (opcode == 0x07 /* OVR */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, 1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, n8);
			put_stack(stack_index, 1, t8);
			put_stack(stack_index, 2, n8);
		}
	}
	else if (opcode == 0x27 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, 2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, n16);
			put2_stack(stack_index, 2, t16);
			put2_stack(stack_index, 4, n16);  
		}
	}
	else if (opcode == 0x08 /* EQU */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, n8 == t8 ? 1 : 0);
		}
	}
	else if (opcode == 0x28 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, -3);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, n16 == t16 ? 1 : 0);
		}
	}
	else if (opcode == 0x09 /* NEQ */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, n8 == t8 ? 0 : 1);
		}
	}
	else if (opcode == 0x29 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, -3);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, n16 == t16 ? 0 : 1);
		}
	}
	else if (opcode == 0x0A /* GTH */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, n8 > t8 ? 1 : 0);
		}
	}
	else if (opcode == 0x2A /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, -3);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, n16 > t16 ? 1 : 0);
		}
	}
	else if (opcode == 0x0B /* LTH */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, n8 < t8 ? 1 : 0);
		}
	}
	else if (opcode == 0x2B /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, -3);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, n16 < t16 ? 1 : 0);
		}
	}
	else if (opcode == 0x0C /* JMP */) {
		t8 = t_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 1, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			pc_add_s8((int8_t)(t8));
		}
	}
	else if (opcode == 0x2C /*  */) {
		t16 = t2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			pc_set(t16);
		}
	}
	else if (opcode == 0x0D /* JCN */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else if (n8 > 0) {
			pc_add_s8((int8_t)(t8));
		}
	}
	else if (opcode == 0x2D /*  */) {
		t16 = t2_register(stack_index);
		n8 = l_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 3, -3);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else if (n8 > 0) {
			pc_set(t16);
		}
	}
	else if (opcode == 0x0E /* JSR */) {
		t8 = t_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 1, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			eval_opcode_tmp = push2_stack(1, ins, pc_get());
			if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
			else {
				pc_add_s8((int8_t)(t8));
			}
		}
	}
	else if (opcode == 0x2E /*  */) {
		t16 = t2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			eval_opcode_tmp = push2_stack(1, ins, pc_get());
			if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
			else {
				pc_set(t16);
			}
		}
	}
	else if (opcode == 0x0F /* STH */) {
		t8 = t_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 1, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			eval_opcode_tmp = push_stack(ins & 0x40 ? 0 : 1, ins, t8);
			if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		}
	}
	else if (opcode == 0x2F /*  */) {
		t16 = t2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			eval_opcode_tmp = push2_stack(ins & 0x40 ? 0 : 1, ins, t16);
			if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		}
	}
	else if (opcode == 0x10 /* LDZ */) {
		t8 = t_register(stack_index);
		t16 = (uint16_t)(t8);
		eval_opcode_tmp = set(stack_index, ins, k, 1, 0);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, main_ram_read(t16));
		}
	}
	else if (opcode == 0x30 /*  */) {
		t8 = t_register(stack_index);
		t16 = (uint16_t)(t8);
		eval_opcode_tmp = set(stack_index, ins, k, 1, 1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, peek2_ram(t16));
		}
	}
	else if (opcode == 0x11 /* STZ */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		t16 = (uint16_t)(t8);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			main_ram_write(t16, n8);
		}
	}
	else if (opcode == 0x31 /*  */) {
		t8 = t_register(stack_index);
		t16 = (uint16_t)(t8);
		n16 = h2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 3, -3);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			poke2_ram(t16, n16);
		}
	}
	else if (opcode == 0x12 /* LDR */) {
		t8 = t_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 1, 0);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			t16 = pc_get() + ((int8_t)(t8));
			put_stack(stack_index, 0, main_ram_read(t16));
		}
	}
	else if (opcode == 0x32 /*  */) {
		t8 = t_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 1, 1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			t16 = pc_get() + ((int8_t)(t8));
			put2_stack(stack_index, 0, peek2_ram(t16));
		}
	}
	else if (opcode == 0x13 /* STR */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			t16 = pc_get() + ((int8_t)(t8));
			main_ram_write(t16, n8);
		}
	}
	else if (opcode == 0x33 /*  */) {
		t8 = t_register(stack_index);
		n16 = h2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 3, -3);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			t16 = pc_get() + ((int8_t)(t8));
			poke2_ram(t16, n16);
		}
	}
	else if (opcode == 0x14 /* LDA */) {
		t16 = t2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, main_ram_read(t16));
		}
	}
	else if (opcode == 0x34 /*  */) {
		t16 = t2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, 0);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 9, peek2_ram(t16));
		}
	}
	else if (opcode == 0x15 /* STA */) {
		t16 = t2_register(stack_index);
		n8 = l_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 3, -3);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			main_ram_write(t16, n8);
		}
	}
	else if (opcode == 0x35 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, -4);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			poke2_ram(t16, n16);
		}
	}
	else if (opcode == 0x16 /* DEI */) {
		t8 = t_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 1, 0);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			dei(stack_index, 0, t8);
		}
	}
	else if (opcode == 0x36 /*  */) {
		t8 = t_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 1, 1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			dei(stack_index, 1, t8);
			dei(stack_index, 0, t8 + 1);
		}
	}
	else if (opcode == 0x17 /* DEO */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			deo(t8, n8);
		}
		/* t=T;n=N;        SET(2,-2) DEO(t, n) break; */
	}
	else if (opcode == 0x37 /*  */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		l8 = l_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 3, -3);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			deo(t8, l8);
			deo(t8 + 1, n8);
		}
		/* t=T;n=N;l=L;    SET(3,-3) DEO(t, l) DEO(t + 1, n) break; */
	}
	else if (opcode == 0x18 /* ADD */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, n8 + t8);
		}
	}
	else if (opcode == 0x38 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, n16 + t16);
		}
	}
	else if (opcode == 0x19 /* SUB */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, n8 - t8);
		}
	}
	else if (opcode == 0x39 /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, n16 - t16);
		}
	}
	else if (opcode == 0x1A /* MUL */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, n8 * t8);
		}
	}
	else if (opcode == 0x3A /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, n16 * t16);
		}
	}
	else if (opcode == 0x1B /* DIV */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else if (t8 == 0) {
			eval_opcode_tmp = halt(ins, 3);
			if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		} else {
			put_stack(stack_index, 0, n8 / t8);
		}
		/* t=T;n=N;        SET(2,-1) if(!t) HALT(3) PUT(0, n / t) break; */
	}
	else if (opcode == 0x3B /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else if (t16 == 0) {
			eval_opcode_tmp = halt(ins, 3);
			if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		} else {
			put2_stack(stack_index, 0, n16 / t16);
		}
		/* t=T2;n=N2;      SET(4,-2) if(!t) HALT(3) PUT2(0, n / t) break; */
	}
	else if (opcode == 0x1C /* AND */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, n8 & t8);
		}
	}
	else if (opcode == 0x3C /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, n16 & t16);
		}
	}
	else if (opcode == 0x1D /* ORA */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, n8 | t8);
		}
	}
	else if (opcode == 0x3D /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, n16 | t16);
		}
	}
	else if (opcode == 0x1E /* EOR */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, n8 ^ t8);
		}
	}
	else if (opcode == 0x3E /*  */) {
		t16 = t2_register(stack_index);
		n16 = n2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 4, -2);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, n16 ^ t16);
		}
	}
	else if (opcode == 0x1F /* SFT */) {
		t8 = t_register(stack_index);
		n8 = n_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 2, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put_stack(stack_index, 0, (n8 >> (t8 & 0x0F)) << (t8 >> 4));
		}
		/* t=T;n=N;        SET(2,-1) PUT(0, n >> (t & 0xf) << (t >> 4)) break; */
	}
	else if (opcode == 0x3F /*  */) {
		t8 = t_register(stack_index);
		n16 = h2_register(stack_index);
		eval_opcode_tmp = set(stack_index, ins, k, 3, -1);
		if (eval_opcode_tmp > 0) { eval_opcode_ret_value = 1; }
		else {
			put2_stack(stack_index, 0, (n16 >> (t8 & 0x0F)) << (t8 >> 4));
		}
		/* t=T;n=H2;       SET(3,-1) PUT2(0, n >> (t & 0xf) << (t >> 4)) break; */
	}
	
	return eval_opcode_ret_value;
}

#pragma MAIN_MHZ uxn_eval 1.0
uint1_t uxn_eval() {

	static uint8_t k, opc, ins;
	static uint1_t s, uxn_eval_should_return, uxn_eval_ret_value;

	uxn_eval_ret_value = 0;
	uxn_eval_should_return = 0;

	if(pc_get() == 0) {
		uxn_eval_should_return = 1;
	}
	
	if (device_ram_read(15) != 0) {
		uxn_eval_should_return = 1;
	}
	
	if (!uxn_eval_should_return) {
		ins = main_ram_read(pc_get()) & 0xff;
		pc_add(1);
		k = ins & 0x80 ? 0xff : 0;
		s = ins & 0x40 ? 1 : 0;
		opc = !(ins & 0x1f) ? (0 - (ins >> 5)) & 0xff : ins & 0x3f;	
		uxn_eval_should_return = eval_opcode(s, opc, ins, k);
	}
	
	return uxn_eval_ret_value;
}