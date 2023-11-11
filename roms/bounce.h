#pragma once
#include "uintN_t.h"  // uintN_t types for any N

/* BOUNCE (assembly)
	|0100
	  ;L.screen.hook #20 DEO2
	  LIT2r 0000 main_ POP2r BRK
	  @L.screen.hook LIT2r 0000 on_screen_ POP2 POP2r BRK
	( bss )
	( data )
	@dy_
	  0001
	@dx_
	  0001
	@y_
	  0000
	@x_
	  0000
	@ball_color_
	  01
	@bg_color_
	  00
	@size_
	  000c
	( text )
	@sext
	  #80 ANDk EQU #ff MUL SWP JMP2r
	
	@on_screen_ ( -- result* )
		OVR2r LIT2r 0004 SUB2r #0000 #28 DEO2 #0000 #2a DEO2 ;bg_color_ LDA sext
		#0080 ORA2 NIP #2e DEO ;x_ LDA2k ;dx_ LDA2 ADD2 SWP2 STA2
		;y_ LDA2k ;dy_ LDA2 ADD2 SWP2 STA2
		#018f ;size_ LDA2 SUB2 #8000 EOR2 ;x_ LDA2 #8000 EOR2 LTH2 #00 ORA ?&true.2
		;x_ LDA2 #0000 EQU2 #00 ORA ?&true.2
		#0000 !&end.2
	
	  &true.2
		#0001
	
	  &end.2
		#0000 EQU2 ?&end.1
		;bg_color_ LDAk sext
		INC2 SWP2 STA
		POP ;ball_color_ LDAk sext
		INC2 SWP2 STA
		POP #0000 ;dx_ LDA2 SUB2 ;dx_ STA2
	
	  &end.1
		#0167 ;size_ LDA2 SUB2 #8000 EOR2 ;y_ LDA2 #8000 EOR2 LTH2 #00 ORA ?&true.4
		;y_ LDA2 #0000 EQU2 #00 ORA ?&true.4
		#0000 !&end.4
	
	  &true.4
		#0001
	
	  &end.4
		#0000 EQU2 ?&end.3
		;bg_color_ LDAk sext
		INC2 SWP2 STA
		POP ;ball_color_ LDAk sext
		INC2 SWP2 STA
		POP #0000 ;dy_ LDA2 SUB2 ;dy_ STA2
	
	  &end.3
		#8003 ;bg_color_ LDA sext
		#8000 EOR2 LTH2 #00 EQU ?&end.5
		#0000 ;bg_color_ STA
		POP
	
	  &end.5
		#8003 ;ball_color_ LDA sext
		#8000 EOR2 LTH2 #00 EQU ?&end.6
		#0000 ;ball_color_ STA
		POP
	
	  &end.6
		;x_ LDA2 STH2kr INC2 INC2 STA2
	
	  &begin.7
		STH2kr INC2 INC2 LDA2 #8000 EOR2 ;x_ LDA2 ;size_ LDA2 ADD2 #8000 EOR2 LTH2 #00 EQU ?&break.7
		;y_ LDA2 STH2kr STA2
	
	  &begin.8
		STH2kr LDA2 #8000 EOR2 ;y_ LDA2 ;size_ LDA2 ADD2 #8000 EOR2 LTH2 #00 EQU ?&break.8
		STH2kr INC2 INC2 LDA2 #28 DEO2 STH2kr LDA2 #2a DEO2 ;ball_color_ LDA #2e DEO
	
	  &continue.8
		STH2kr LDA2k INC2k ROT2 STA2
		POP2 !&begin.8
	
	  &break.8
	
	  &continue.7
		STH2kr INC2 INC2 LDA2k INC2k ROT2 STA2
		POP2 !&begin.7
	
	  &break.7
		#0000
	
	  &return
		POP2r JMP2r
	
	@main_ ( -- result* )
		OVR2r #08df #08 DEO2 #12bf #0a DEO2 #549d #0c DEO2 #0190 #22 DEO2 #0168 #24 DEO2 #0000
	
	  &return
		POP2r JMP2r

*/

/* BOUNCE.C
#include <varvara.h>

int size = 12;
char bg_color = 0x00;
char ball_color = 0x01;
int x = 0;
int y = 0;
int dx = 1;
int dy = 1;

void on_screen(void) {
	
	set_screen_xy(0, 0);
	draw_pixel(BgFillBR | bg_color);
	
	x += dx;
	y += dy;
	
	if (x > (399 - size) || x == 0) { 
		bg_color += 1;
		ball_color += 1; 
		dx = -dx;
	}
	
	if (y > (359 - size) || y == 0) { 
		bg_color += 1;
		ball_color += 1; 
		dy = -dy; 
	}
	
	if (bg_color > 0x03) {
		bg_color = 0x00;
	}
	
	if (ball_color > 0x03) {
		ball_color = 0x00;
	}
	
	for (int ball_x = x; ball_x < (x + size); ball_x++) {
		for (int ball_y = y; ball_y < (y + size); ball_y++) {
			set_screen_xy(ball_x, ball_y);
			draw_pixel(ball_color);
		}
	}
}

void main(void) {
	set_palette(0x08df, 0x12bf, 0x549d);
	set_screen_size(400, 360);
}

*/

#define ROM_SIZE 512

uint8_t read_rom_byte(uint16_t read_address)
{
	static uint8_t uxn_rom[ROM_SIZE] = {
		0xA0, 0x01, 0x0E, 0x80, 0x20, 0x37, 0xE0, 0x00,
		0x00, 0x60, 0x01, 0xB6, 0x62, 0x00, 0xE0, 0x00,
		0x00, 0x60, 0x00, 0x18, 0x22, 0x62, 0x00, 0x00,
		0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x00, 0x0C, 0x80, 0x80, 0x9C, 0x08, 0x80,
		0xFF, 0x1A, 0x04, 0x6C, 0x67, 0xE0, 0x00, 0x04,
		0x79, 0xA0, 0x00, 0x00, 0x80, 0x28, 0x37, 0xA0,
		0x00, 0x00, 0x80, 0x2A, 0x37, 0xA0, 0x01, 0x20,
		0x14, 0x60, 0xFF, 0xDF, 0xA0, 0x00, 0x80, 0x3D,
		0x03, 0x80, 0x2E, 0x17, 0xA0, 0x01, 0x1D, 0xB4,
		0xA0, 0x01, 0x19, 0x34, 0x38, 0x24, 0x35, 0xA0,
		0x01, 0x1B, 0xB4, 0xA0, 0x01, 0x17, 0x34, 0x38,
		0x24, 0x35, 0xA0, 0x01, 0x8F, 0xA0, 0x01, 0x21,
		0x34, 0x39, 0xA0, 0x80, 0x00, 0x3E, 0xA0, 0x01,
		0x1D, 0x34, 0xA0, 0x80, 0x00, 0x3E, 0x2B, 0x80,
		0x00, 0x1D, 0x20, 0x00, 0x14, 0xA0, 0x01, 0x1D,
		0x34, 0xA0, 0x00, 0x00, 0x28, 0x80, 0x00, 0x1D,
		0x20, 0x00, 0x06, 0xA0, 0x00, 0x00, 0x40, 0x00,
		0x03, 0xA0, 0x00, 0x01, 0xA0, 0x00, 0x00, 0x28,
		0x20, 0x00, 0x22, 0xA0, 0x01, 0x20, 0x94, 0x60,
		0xFF, 0x81, 0x21, 0x24, 0x15, 0x02, 0xA0, 0x01,
		0x1F, 0x94, 0x60, 0xFF, 0x76, 0x21, 0x24, 0x15,
		0x02, 0xA0, 0x00, 0x00, 0xA0, 0x01, 0x19, 0x34,
		0x39, 0xA0, 0x01, 0x19, 0x35, 0xA0, 0x01, 0x67,
		0xA0, 0x01, 0x21, 0x34, 0x39, 0xA0, 0x80, 0x00,
		0x3E, 0xA0, 0x01, 0x1B, 0x34, 0xA0, 0x80, 0x00,
		0x3E, 0x2B, 0x80, 0x00, 0x1D, 0x20, 0x00, 0x14,
		0xA0, 0x01, 0x1B, 0x34, 0xA0, 0x00, 0x00, 0x28,
		0x80, 0x00, 0x1D, 0x20, 0x00, 0x06, 0xA0, 0x00,
		0x00, 0x40, 0x00, 0x03, 0xA0, 0x00, 0x01, 0xA0,
		0x00, 0x00, 0x28, 0x20, 0x00, 0x22, 0xA0, 0x01,
		0x20, 0x94, 0x60, 0xFF, 0x26, 0x21, 0x24, 0x15,
		0x02, 0xA0, 0x01, 0x1F, 0x94, 0x60, 0xFF, 0x1B,
		0x21, 0x24, 0x15, 0x02, 0xA0, 0x00, 0x00, 0xA0,
		0x01, 0x17, 0x34, 0x39, 0xA0, 0x01, 0x17, 0x35,
		0xA0, 0x80, 0x03, 0xA0, 0x01, 0x20, 0x14, 0x60,
		0xFF, 0x01, 0xA0, 0x80, 0x00, 0x3E, 0x2B, 0x80,
		0x00, 0x08, 0x20, 0x00, 0x08, 0xA0, 0x00, 0x00,
		0xA0, 0x01, 0x20, 0x15, 0x02, 0xA0, 0x80, 0x03,
		0xA0, 0x01, 0x1F, 0x14, 0x60, 0xFE, 0xE4, 0xA0,
		0x80, 0x00, 0x3E, 0x2B, 0x80, 0x00, 0x08, 0x20,
		0x00, 0x08, 0xA0, 0x00, 0x00, 0xA0, 0x01, 0x1F,
		0x15, 0x02, 0xA0, 0x01, 0x1D, 0x34, 0xEF, 0x21,
		0x21, 0x35, 0xEF, 0x21, 0x21, 0x34, 0xA0, 0x80,
		0x00, 0x3E, 0xA0, 0x01, 0x1D, 0x34, 0xA0, 0x01,
		0x21, 0x34, 0x38, 0xA0, 0x80, 0x00, 0x3E, 0x2B,
		0x80, 0x00, 0x08, 0x20, 0x00, 0x47, 0xA0, 0x01,
		0x1B, 0x34, 0xEF, 0x35, 0xEF, 0x34, 0xA0, 0x80,
		0x00, 0x3E, 0xA0, 0x01, 0x1B, 0x34, 0xA0, 0x01,
		0x21, 0x34, 0x38, 0xA0, 0x80, 0x00, 0x3E, 0x2B,
		0x80, 0x00, 0x08, 0x20, 0x00, 0x1C, 0xEF, 0x21,
		0x21, 0x34, 0x80, 0x28, 0x37, 0xEF, 0x34, 0x80,
		0x2A, 0x37, 0xA0, 0x01, 0x1F, 0x14, 0x80, 0x2E,
		0x17, 0xEF, 0xB4, 0xA1, 0x25, 0x35, 0x22, 0x40,
		0xFF, 0xCA, 0xEF, 0x21, 0x21, 0xB4, 0xA1, 0x25,
		0x35, 0x22, 0x40, 0xFF, 0x9D, 0xA0, 0x00, 0x00,
		0x62, 0x6C, 0x67, 0xA0, 0x08, 0xDF, 0x80, 0x08,
		0x37, 0xA0, 0x12, 0xBF, 0x80, 0x0A, 0x37, 0xA0,
		0x54, 0x9D, 0x80, 0x0C, 0x37, 0xA0, 0x01, 0x90,
		0x80, 0x22, 0x37, 0xA0, 0x01, 0x68, 0x80, 0x24,
		0x37, 0xA0, 0x00, 0x00, 0x62, 0x6C, 0x00, 0x00,
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