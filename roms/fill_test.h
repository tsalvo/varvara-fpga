#pragma once
#include "uintN_t.h"  // uintN_t types for any N

/* Fill Drawing test (assembly)
	|0100
  	LIT2r 0000 main_ POP2r BRK
	( bss )
	( data )
	( text )
	
	@main_ ( -- result* )
		OVR2r #2ce9 #08 DEO2 #01c0 #0a DEO2 #7ce5 #0c DEO2 #0190 #22 DEO2 #0168 #24 DEO2 #0000 #28 DEO2 #0000 #2a DEO2 #802e DEO #0014 #28 DEO2 #0014 #2a DEO2 #812e DEO #0028 #28 DEO2 #0028 #2a DEO2 #822e DEO #003c #28 DEO2 #003c #2a DEO2 #832e DEO #0050 #28 DEO2 #0050 #2a DEO2 #802e DEO #0064 #28 DEO2 #0064 #2a DEO2 #812e DEO #0078 #28 DEO2 #0078 #2a DEO2 #822e DEO #008c #28 DEO2 #008c #2a DEO2 #832e DEO #00a0 #28 DEO2 #00a0 #2a DEO2 #b02e DEO #008c #28 DEO2 #008c #2a DEO2 #b32e DEO #0078 #28 DEO2 #0078 #2a DEO2 #b22e DEO #0064 #28 DEO2 #0064 #2a DEO2 #b12e DEO #0050 #28 DEO2 #0050 #2a DEO2 #b02e DEO #003c #28 DEO2 #003c #2a DEO2 #b32e DEO #0028 #28 DEO2 #0028 #2a DEO2 #b22e DEO #0014 #28 DEO2 #0014 #2a DEO2 #b12e DEO #0000
	
  	&return
		POP2r JMP2r
	*/

/* Fill Drawing Test (chibicc)
	#include <varvara.h>
	
	void main() {
  	set_palette(0x2ce9, 0x01c0, 0x7ce5);
  	set_screen_size(400, 360);
  	set_screen_xy(0, 0);
  	draw_pixel(BgFillBR | 0x00);
  	set_screen_xy(20, 20);
  	draw_pixel(BgFillBR | 0x01);
  	set_screen_xy(40, 40);
  	draw_pixel(BgFillBR | 0x02);
  	set_screen_xy(60, 60);
  	draw_pixel(BgFillBR | 0x03);
  	set_screen_xy(80, 80);
  	draw_pixel(BgFillBR | 0x00);
  	set_screen_xy(100, 100);
  	draw_pixel(BgFillBR | 0x01);
  	set_screen_xy(120, 120);
  	draw_pixel(BgFillBR | 0x02);
  	set_screen_xy(140, 140);
  	draw_pixel(BgFillBR | 0x03);
  	
  	set_screen_xy(160, 160);
  	draw_pixel(BgFillTL | 0x00);
  	set_screen_xy(140, 140);
  	draw_pixel(BgFillTL | 0x03);
  	set_screen_xy(120, 120);
  	draw_pixel(BgFillTL | 0x02);
  	set_screen_xy(100, 100);
  	draw_pixel(BgFillTL | 0x01);
  	set_screen_xy(80, 80);
  	draw_pixel(BgFillTL | 0x00);
  	set_screen_xy(60, 60);
  	draw_pixel(BgFillTL | 0x03);
  	set_screen_xy(40, 40);
  	draw_pixel(BgFillTL | 0x02);
  	set_screen_xy(20, 20);
  	draw_pixel(BgFillTL | 0x01);
	}
*/
#define ROM_SIZE 512

uint8_t read_rom_byte(uint16_t read_address)
{
	static uint8_t uxn_rom[ROM_SIZE] = {
		0xE0, 0x00, 0x00, 0x60, 0x00, 0x02, 0x62, 0x00,
		0x67, 0xA0, 0x2C, 0xE9, 0x80, 0x08, 0x37, 0xA0,
		0x01, 0xC0, 0x80, 0x0A, 0x37, 0xA0, 0x7C, 0xE5,
		0x80, 0x0C, 0x37, 0xA0, 0x01, 0x90, 0x80, 0x22,
		0x37, 0xA0, 0x01, 0x68, 0x80, 0x24, 0x37, 0xA0,
		0x00, 0x00, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x00,
		0x80, 0x2A, 0x37, 0xA0, 0x80, 0x2E, 0x17, 0xA0,
		0x00, 0x14, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x14,
		0x80, 0x2A, 0x37, 0xA0, 0x81, 0x2E, 0x17, 0xA0,
		0x00, 0x28, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x28,
		0x80, 0x2A, 0x37, 0xA0, 0x82, 0x2E, 0x17, 0xA0,
		0x00, 0x3C, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x3C,
		0x80, 0x2A, 0x37, 0xA0, 0x83, 0x2E, 0x17, 0xA0,
		0x00, 0x50, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x50,
		0x80, 0x2A, 0x37, 0xA0, 0x80, 0x2E, 0x17, 0xA0,
		0x00, 0x64, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x64,
		0x80, 0x2A, 0x37, 0xA0, 0x81, 0x2E, 0x17, 0xA0,
		0x00, 0x78, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x78,
		0x80, 0x2A, 0x37, 0xA0, 0x82, 0x2E, 0x17, 0xA0,
		0x00, 0x8C, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x8C,
		0x80, 0x2A, 0x37, 0xA0, 0x83, 0x2E, 0x17, 0xA0,
		0x00, 0xA0, 0x80, 0x28, 0x37, 0xA0, 0x00, 0xA0,
		0x80, 0x2A, 0x37, 0xA0, 0xB0, 0x2E, 0x17, 0xA0,
		0x00, 0x8C, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x8C,
		0x80, 0x2A, 0x37, 0xA0, 0xB3, 0x2E, 0x17, 0xA0,
		0x00, 0x78, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x78,
		0x80, 0x2A, 0x37, 0xA0, 0xB2, 0x2E, 0x17, 0xA0,
		0x00, 0x64, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x64,
		0x80, 0x2A, 0x37, 0xA0, 0xB1, 0x2E, 0x17, 0xA0,
		0x00, 0x50, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x50,
		0x80, 0x2A, 0x37, 0xA0, 0xB0, 0x2E, 0x17, 0xA0,
		0x00, 0x3C, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x3C,
		0x80, 0x2A, 0x37, 0xA0, 0xB3, 0x2E, 0x17, 0xA0,
		0x00, 0x28, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x28,
		0x80, 0x2A, 0x37, 0xA0, 0xB2, 0x2E, 0x17, 0xA0,
		0x00, 0x14, 0x80, 0x28, 0x37, 0xA0, 0x00, 0x14,
		0x80, 0x2A, 0x37, 0xA0, 0xB1, 0x2E, 0x17, 0xA0,
		0x00, 0x00, 0x62, 0x6C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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
  
  static uint32_t rdaddr;
  rdaddr = (uint32_t)(read_address);
  
  uint8_t rdata = uxn_rom_RAM_SP_RF_1(
	  rdaddr,	// read address
	  0, 		// write value
	  0			// write enable
  );
  
  return rdata;
}