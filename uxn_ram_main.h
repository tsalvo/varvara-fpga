#pragma once
#include "uintN_t.h"  // uintN_t types for any N
// #pragma once
// #include "ram.h"      // PipelineC RAM declarations

// 64KB Main RAM
uint8_t main_ram_update(uint16_t read_address, uint8_t value, uint1_t write_enable) {
	static uint8_t main_ram[65536];
	static uint32_t rwaddr;
	static uint8_t wdata;
	rwaddr = (uint32_t)(read_address);
	wdata = value;
  
  uint8_t rdata = main_ram_RAM_SP_RF_1(
	  rwaddr,			// rw address
	  wdata, 			// write value
	  write_enable		// write enable
  );
  
  printf("  Main RAM rwaddr = 0x%X, write0_enable = 0x%X, wdata = 0x%X, result = 0x%X\n", rwaddr, write_enable, wdata, rdata);

  return rdata;
}
