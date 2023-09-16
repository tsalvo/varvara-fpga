#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#include <stdint.h>

/* ANNOTATED ASSEMBLY
|0100
  LIT2r(E0) 0000 main_(_JSI0002_ 60 0002) POP2r(62) BRK(00)
( bss )
( data )
( text )

@main_ ( -- result* )
	OVR2r(67) (_LIT2_A0_) #08df (_LIT_80) #08 DEO2(37) (_LIT2_A0_) #12bf (_LIT_80) #0a DEO2(37) (_LIT2_A0_) #549d (_LIT_80) #0c DEO2(37) (_LIT2_A0_) #000a (_LIT_80) #28 DEO2(37) (_LIT2_A0_) #000a (_LIT_80) #2a DEO2(37) (_LIT2_A0_) #022e DEO(17) (_LIT2_A0_) #0014 (_LIT_80) #28 DEO2(37) (_LIT2_A0_) #0014 (_LIT_80) #2a DEO2(37) (_LIT2_A0_) #012e DEO(17) (_LIT2_A0_) #000a (_LIT_80) #28 DEO2(37) (_LIT2_A0_) #0014 (_LIT_80) #2a DEO2(37) (_LIT2_A0_) #032e DEO(17) (_LIT2_A0_) #0014 (_LIT_80) #28 DEO2(37) (_LIT2_A0_) #000a (_LIT_80) #2a DEO2(37) (_LIT2_A0_) #022e DEO(17) (_LIT2_A0_) #0000

  &return
	POP2r(62) JMP2r(6C)  
*/
uint8_t read_rom_byte(uint8_t read_address)
{
	// Draw single pixel at (10,10)
	// E0000060 00026200 67A008DF 800837A0 12BF800A 37A0549D 800C37A0 000A8028 37A0000A 802A37A0 022E17A0 0000626C
	static uint8_t uxn_rom[128] = {
		0xE0, 0x00, 0x00, 0x60, 0x00, 0x02, 0x62, 0x00, 
		0x67, 0xA0, 0x08, 0xDF, 0x80, 0x08, 0x37, 0xA0, 
		0x12, 0xBF, 0x80, 0x0A, 0x37, 0xA0, 0x54, 0x9D, 
		0x80, 0x0C, 0x37, 0xA0, 0x00, 0x0A, 0x80, 0x28,
		0x37, 0xA0, 0x00, 0x0A, 0x80, 0x2A, 0x37, 0xA0, 
		0x02, 0x2E, 0x17, 0xA0, 0x00, 0x14, 0x80, 0x28,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  
  // // E0000060 00026200 67A0000A 802837A0 000A802A 37A0022E 17A00014 802837A0 0014802A 37A0012E 17A0000A 802837A0 0014802A 37A0032E 17A00014 802837A0 000A802A 37A0002E 17A00032 802837A0 0032802A 37A0012E 17A0001E 802837A0 0032802A 37A0022E 17A00032 802837A0 001E802A 37A0032E 17A00000 626C
  // 
  // static uint8_t uxn_rom[128] = {
	//   0xE0, 0x00, 0x00, 0x60, 0x00, 0x02, 0x62, 0x00,
	//   0x67, 0xA0, 0x00, 0x0A, 0x80, 0x28, 0x37, 0xA0,
	//   0x00, 0x0A, 0x80, 0x2A, 0x37, 0xA0, 0x02, 0x2E,
	//   0x17, 0xA0, 0x00, 0x14, 0x80, 0x28, 0x37, 0xA0,
	//   0x00, 0x14, 0x80, 0x2A, 0x37, 0xA0, 0x01, 0x2E,
	//   0x17, 0xA0, 0x00, 0x0A, 0x80, 0x28, 0x37, 0xA0,
	//   0x00, 0x14, 0x80, 0x2A, 0x37, 0xA0, 0x03, 0x2E,
	//   0x17, 0xA0, 0x00, 0x14, 0x80, 0x28, 0x37, 0xA0,
	//   0x00, 0x0A, 0x80, 0x2A, 0x37, 0xA0, 0x00, 0x2E,
	//   0x17, 0xA0, 0x00, 0x32, 0x80, 0x28, 0x37, 0xA0,
	//   0x00, 0x32, 0x80, 0x2A, 0x37, 0xA0, 0x01, 0x2E,
	//   0x17, 0xA0, 0x00, 0x1E, 0x80, 0x28, 0x37, 0xA0,
	//   0x00, 0x32, 0x80, 0x2A, 0x37, 0xA0, 0x02, 0x2E,
	//   0x17, 0xA0, 0x00, 0x32, 0x80, 0x28, 0x37, 0xA0,
	//   0x00, 0x1E, 0x80, 0x2A, 0x37, 0xA0, 0x03, 0x2E,
	//   0x17, 0xA0, 0x00, 0x00, 0x62, 0x6C, 0x00, 0x00
	// };
  
  static uint32_t rdaddr;
  rdaddr = (uint32_t)(read_address);
  
  uint8_t rdata = uxn_rom_RAM_SP_RF_1(
	  rdaddr,	// read address
	  0, 		// write value
	  0			// write enable
  );

  return rdata;
}