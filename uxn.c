#include "uintN_t.h"  // uintN_t types for any N
#include "intN_t.h"   // intN_t types for any N

#include "uxn_rom.h"
#include "uxn_opcodes.h"
#include "uxn_ram_main.h"

// RULES:
// - cannot write to a global variable from more than one function (unless you use clock domain crossing)
// - no switch statements (C AST node cannot be parsed to logic)
// - only one return per function
// - no ++ or -- operators

// Build with Docker pipelinec image: 
// docker run -v $(pwd):/workdir pipelinec pipelinec uxn.c

// https://github.com/JulianKemmerer/PipelineC/wiki/Automatically-Generated-Functionality#rams

typedef struct boot_step_result_t {
	uint1_t is_valid_byte;  // is the most recent byte valid?
	uint1_t is_finished; 	// is the last byte read?
	uint8_t rom_byte;  		// the most recent byte read
	uint16_t ram_address; 	// RAM address to copy byte to (if valid)
} boot_step_result_t;

boot_step_result_t step_boot() {
	static uint16_t ram_address = 0xFE, rom_address = 0; // first ROM byte goes to RAM 0x0100, but keep the RAM address behind by 1 due to latency, and 1 more behind due to early ram_address incrementing
	static uint8_t rom_byte = 0;
	static uint1_t boot_phase = 0, is_finished = 0;
	
	if (boot_phase == 0) {
		rom_byte = read_rom_byte(rom_address); // START
		printf("    STEP BOOT: Phase 0, reading from ROM Address: 0x%X ...\n", rom_address);
		is_finished = 0;
	}
	else if (boot_phase == 1) {
		rom_byte = read_rom_byte(rom_address); // DONE		
		printf("    STEP BOOT: Phase 1, ROM Address:  0x%X, ROM Byte = 0x%X ...\n", rom_address, rom_byte);
		rom_address += 1;
		ram_address += 1;
		is_finished = rom_address > 511;
	}
	
	boot_step_result_t boot_result = {boot_phase, is_finished, rom_byte, ram_address}; // max init
	boot_phase += 1;
	
	return boot_result;
}

typedef struct cpu_step_result_t {
	uint1_t is_ram_write;
	uint16_t ram_address;
	uint8_t ram_value;
	
	uint1_t is_vram_write;
	uint32_t vram_address;
	uint2_t vram_value;
} cpu_step_result_t;

cpu_step_result_t step_cpu(uint8_t ram_read_value) {
	static uint16_t pc = 0x0100;
	static uint8_t ins = 0;
	static uint8_t step_cpu_phase = 0x00;
	static uint1_t is_ins_done = 0;
	static eval_opcode_result_t eval_opcode_result;
	static cpu_step_result_t cpu_step_result = {0, 0, 0, 0, 0, 0};
	if (step_cpu_phase == 0x00) {
		is_ins_done = 0;
		cpu_step_result.ram_address = pc; // START
		cpu_step_result.is_ram_write = 0;
		printf("    STEP CPU: Phase = 0x%X, PC = 0x%X \n", step_cpu_phase, pc);
	}
	else if (step_cpu_phase == 0x01) {
		cpu_step_result.ram_address = pc; // DONE
		printf("    STEP CPU: Phase = 0x%X, PC = 0x%X, ins = 0x%X\n", step_cpu_phase, pc, ins);
	}
	else if (step_cpu_phase == 0x02) {	
		ins = ram_read_value;
		pc += 1;
		printf("    STEP CPU: Phase = 0x%X, PC = 0x%X, ins = 0x%X\n", step_cpu_phase, pc, ins);
	}
	else {
		eval_opcode_result = eval_opcode_phased(step_cpu_phase - 3, ins, pc, ram_read_value);
		pc = eval_opcode_result.is_pc_updated ? eval_opcode_result.pc : pc;
		cpu_step_result.is_ram_write = eval_opcode_result.is_ram_write;
		cpu_step_result.ram_address = eval_opcode_result.ram_addr;
		cpu_step_result.ram_value = eval_opcode_result.ram_value;
		cpu_step_result.is_vram_write = eval_opcode_result.is_vram_write;
		cpu_step_result.vram_address = eval_opcode_result.vram_address;
		cpu_step_result.vram_value = eval_opcode_result.vram_value;
		is_ins_done = eval_opcode_result.is_opc_done;
		printf("    STEP CPU: Phase = 0x%X, PC = 0x%X, ins = 0x%X\n", step_cpu_phase, pc, ins);
	}
	
	if (is_ins_done == 1) {
		step_cpu_phase = 0;
	} else {
		step_cpu_phase += 1;
	}
	
	return cpu_step_result;
}

uint2_t step_gpu(uint1_t is_active_drawing_area, uint1_t is_vram_write, uint32_t vram_address, uint2_t vram_value) {
	static uint2_t step_gpu_result = 0;
	static uint32_t pixel_counter = 0; // 800x720, max = 575999

	step_gpu_result = background_vram_update(
		vram_address,   		// port 0 address
		vram_value,				// port 0 write value
		is_vram_write,			// port 0 write enable
		pixel_counter,			// port 1 read address
		is_active_drawing_area 	// port 1 read enable
	);
	// Pixel Counter
	if (pixel_counter == 576000 - 1) { // 800x720
		pixel_counter = 0;
	} else if (is_active_drawing_area) {
		pixel_counter += 1;
	}
	
	printf("    STEP GPU: is_active_drawing_area = 0x%X, is_vram_write = 0x%X, vram_address = 0x%X, vram_value = 0x%X, result = 0x%X\n", is_active_drawing_area, is_vram_write, vram_address, vram_value, step_gpu_result);

	return step_gpu_result;
}

// Top-level module
// 16-bit input message format:
// 0001 UDLR SSBA YXLR  Controls
// 0010 ---- ---- -PHV  (P)Visible Pixel, (H)HBLANK, (V)VBLANK
#pragma MAIN_MHZ uxn_eval 44.28
uint16_t uxn_eval(uint16_t input) {
	static uint32_t main_clock_cycle = 0;
	static uint4_t input_code;
	static uint12_t palette_color_values[4] = {0xFFF, 0x000, 0x7DB, 0xF62};
	static uint16_t uxn_eval_result = 0;
	static uint2_t current_pixel_palette_color = 0;
	static uint1_t is_active_drawing_area = 0, is_booted = 0;
	
	static uint1_t is_ram_write = 0;
	static uint16_t ram_address = 0;
	static uint8_t ram_write_value = 0;
	static uint8_t ram_read_value = 0;
	static uint1_t is_vram_write = 0;
	static uint32_t vram_address = 0;
	static uint2_t vram_value = 0;
	input_code = input >> 12;
	
	printf("CLOCK: 0x%X\n", main_clock_cycle);
	
	if (input_code == 0x2) {
		is_active_drawing_area = input >> 2 & 0x0001;
	} 
	
	if (is_booted) {
		cpu_step_result_t cpu_step_result = step_cpu(ram_read_value);
		is_ram_write = cpu_step_result.is_ram_write;
		ram_address = cpu_step_result.ram_address;
		ram_write_value = cpu_step_result.ram_value;
		is_vram_write = cpu_step_result.is_vram_write;
		vram_address = cpu_step_result.vram_address;
		vram_value = cpu_step_result.vram_value;
	} else {
		boot_step_result_t boot_step_result = step_boot();
		is_ram_write = boot_step_result.is_valid_byte;
		ram_address = boot_step_result.ram_address;
		ram_write_value = boot_step_result.rom_byte;
		is_booted = boot_step_result.is_finished;
	}
	
	ram_read_value = main_ram_update(
		ram_address,
		ram_write_value,
		is_ram_write
	);
	
	current_pixel_palette_color = step_gpu(is_active_drawing_area, is_vram_write, vram_address, vram_value);
	uxn_eval_result = (uint16_t)(palette_color_values[current_pixel_palette_color]);
	main_clock_cycle += 1;
	
	return uxn_eval_result;
}