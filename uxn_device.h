#pragma once
#include "uintN_t.h"  // uintN_t types for any N
#pragma once
#include "ram.h"      // PipelineC RAM declarations

#pragma once
#include "uxn_ram_device.h"
#pragma once
#include "uxn_ram_screen.h"
#pragma once
#include "uxn_stack.h" 

uint8_t screen_dei(uint8_t addr, uint8_t default_value) {
	static uint8_t result;
	static uint16_t width = 320;
	static uint16_t height = 240;
	if (addr == 0x22) {
		result = (uint8_t)(width >> 8);
	}
	else if (addr == 0x23) {
		result = (uint8_t)(width);
	}
	else if (addr == 0x24) {
		result = (uint8_t)(height >> 8);
	}
	else if (addr == 0x25) {
		result = (uint8_t)(height);
	}
	else {
		result = default_value;
	}
	return result;
}

uint8_t datetime_dei(uint8_t addr) {
	static uint8_t result;
	result = 0;
	// TODO: implement
	return result;
}

// TODO: maybe remove?
uint8_t uxn_dei(uint8_t addr) {
	/*
	Varvara
	00  system	    80	controller
	10  console     90	mouse
	20  screen      a0	file
	30  audio	    b0
	40              c0	datetime
	50              d0	Reserved
	60              e0
	70  	        f0
	*/
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
		result = peek_dev(addr);
	}
	
	return result;
}
	
void dei(uint1_t stack_index, uint8_t stack_offset, uint8_t addr) {
	static uint16_t dei_mask[16] = {0x0000, 0x0000, 0x003c, 0x0014, 0x0014, 0x0014, 0x0014, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x07ff, 0x0000, 0x0000, 0x0000};
	static uint16_t is_event; 
	static uint8_t value;
	is_event = (dei_mask[value >> 4] >> (addr & 0x0F)) & 0x0001;
	value = is_event ? uxn_dei(addr) : peek_dev(addr);
	put_stack_old(stack_index, stack_offset, value);
}

uint1_t dei_phased(uint3_t phase, uint8_t sp, uint1_t stack_index, uint8_t stack_offset, uint8_t addr) {
	static uint16_t dei_mask[16] = {0x0000, 0x0000, 0x003c, 0x0014, 0x0014, 0x0014, 0x0014, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x07ff, 0x0000, 0x0000, 0x0000};
	static uint16_t is_event16; 
	static uint8_t d, dev_ram_at_addr, value;
	static uint1_t is_event, result;
	
	if (phase == 0) {
		result = 0;
		d = addr & 0xF0;
		is_event16 = (dei_mask[value >> 4] >> (addr & 0x0F)) & 0x0001;
		is_event = (uint1_t)(is_event16);
		dev_ram_at_addr = peek_dev(addr); // START
	}
	else if (phase == 1) {
		dev_ram_at_addr = peek_dev(addr); // DONE
		if (is_event) {
			if (d == 0x20) {
				value = screen_dei(addr, dev_ram_at_addr); // START
			}
			else if (d == 0xC0) {
				value = datetime_dei(addr); // START
			} 
			else {
				put_stack(sp, stack_index, stack_offset, dev_ram_at_addr); // START
			}
		} else {
			put_stack(sp, stack_index, stack_offset, dev_ram_at_addr); // START
		}
	}
	else if (phase == 2) {
		if (is_event) {
			if (d == 0x20) {
				value = screen_dei(addr, dev_ram_at_addr); // DONE
				put_stack(sp, stack_index, stack_offset, value); // START
			}
			else if (d == 0xC0) {
				value = datetime_dei(addr); // DONE
				put_stack(sp, stack_index, stack_offset, value); // START
			} 
			else {
				result = 1; // DONE
			}
		} else {
			result = 1; // DONE
		}
	}
	else if (phase == 3) {
		result = 1;
	}
	else if (phase == 4) {
		
	}
	else if (phase == 5) {
		
	}
	else if (phase == 6) {
		
	}
	else if (phase == 7) {
		result = 1; // DONE
	}
	
	return result;
}

void system_deo(uint8_t d, uint8_t device_port) {
	if (device_port == 0x03) {
		// TODO: implement
		// system_cmd(u->ram, PEEK2(d + 2));
	} 
	else if (device_port == 0x05) {
		// TODO: implement
		// system_friend(u->ram, PEEK2(d + 4));
	} 
	else if (device_port == 0x0E) {
		// TODO: implement
		// system_friend(u->ram, PEEK2(d + 4));
	}
}

void console_deo(uint8_t d, uint8_t device_port) {
	// TODO: implement
}

void screen_deo(uint8_t d, uint8_t device_port) {
	// TODO: implement
}

uint1_t screen_deo_phased(uint4_t phase, uint8_t device_base_address, uint8_t device_port) {
	static uint16_t x, y;
	static uint8_t draw_ctrl, color, auto_advance;
	static uint1_t is_fill_mode, layer, is_x_valid, is_y_valid, result;
	if (device_port == 0x03) { 	 // resize width
		// unimplemented - static screen size
		result = 1;
	}
	else if (device_port == 0x05) { // resize height
		// unimplemented - static screen size
		result = 1;
	}
	else if (device_port == 0x0E) { // pixel draw / fill
		if (phase == 0x0) {
			result = 0;
			x = peek2_dev(device_base_address + 0x08); // START
		}
		else if (phase == 0x1) {
			x = peek2_dev(device_base_address + 0x0A); // DONE / START
		}
		else if (phase == 0x2) {
			y = peek_dev(device_base_address + 0x0E); // DONE / START
			is_x_valid = x < 320;
			is_y_valid = y < 240;
		}
		else if (phase == 0x3) {
			draw_ctrl = peek_dev(device_base_address + 0x06);  // DONE / START
			color = draw_ctrl & 0x03;
			is_fill_mode = draw_ctrl & 0x80;
			layer = (draw_ctrl & 0x40) ? 1 : 0;
			if (is_fill_mode) { // fill mode
				
				
			}
			else { // pixel mode
				if(is_x_valid & is_y_valid) {
					// TODO: support multiple layers
					// e.g. layer[x + y * width] = color;
					background_vram_update(
						(uint32_t)(x) + ((uint32_t)(y) * (uint32_t)(320)), 					// port 0 address
						(uint2_t)(color),		// port 0 write value
						1,								// port 0 write enable
						0,								// port 0 read enable
						0					// port 1 read address
					);
				}
			}
		}
		else if (phase == 0x4) {
			auto_advance = peek_dev(device_base_address + 0x06); // DONE
			if (is_fill_mode) { // fill mode
				
			}
			else { // pixel mode
				if (auto_advance & 0x01) {
					x += 1;
					poke_dev(device_base_address + 0x08, (uint8_t)(x >> 8)); // START
				}
			}
		}
		else if (phase == 0x5) {
			if (is_fill_mode) { // fill mode
				
			}
			else { // pixel mode
				if (auto_advance & 0x01) {
					poke_dev(device_base_address + 0x09, (uint8_t)(x)); // START
				}
			}
		}
		else if (phase == 0x6) {
			if (is_fill_mode) { // fill mode
				
			}
			else { // pixel mode
				if (auto_advance & 0x02) {
					y += 1;
					poke_dev(device_base_address + 0x0A, (uint8_t)(y >> 8)); // START
				}
			}
		}
		else if (phase == 0x7) {
			if (is_fill_mode) { // fill mode
				
			}
			else { // pixel mode
				if (auto_advance & 0x02) {
					poke_dev(device_base_address + 0x0B, (uint8_t)(y)); // START
				}
			}
		}
		else if (phase == 0x8) {
			if (is_fill_mode) { // fill mode
				
			}
			else { // pixel mode
				
			}
			result = 1; // DONE
		}
	}
	else if (device_port == 0x0F) { // sprite
		result = 1;
	}
	
	return result;
	
	// switch(port) {
	// case 0x3:
	// 	screen_resize(PEEK2(d + 2), uxn_screen.height);
	// 	break;
	// case 0x5:
	// 	screen_resize(uxn_screen.width, PEEK2(d + 4));
	// 	break;
	// case 0xe: {
	// 	Uint8 ctrl = d[0xe];
	// 	Uint8 color = ctrl & 0x3;
	// 	Uint16 x = PEEK2(d + 0x8);
	// 	Uint16 y = PEEK2(d + 0xa);
	// 	Uint8 *layer = (ctrl & 0x40) ? uxn_screen.fg : uxn_screen.bg;
	// 	/* fill mode */
	// 	if(ctrl & 0x80) {
	// 		Uint16 x2 = uxn_screen.width;
	// 		Uint16 y2 = uxn_screen.height;
	// 		if(ctrl & 0x10) x2 = x, x = 0;
	// 		if(ctrl & 0x20) y2 = y, y = 0;
	// 		screen_fill(layer, x, y, x2, y2, color);
	// 		screen_change(x, y, x2, y2);
	// 	}
	// 	/* pixel mode */
	// 	else {
	// 		Uint16 width = uxn_screen.width;
	// 		Uint16 height = uxn_screen.height;
	// 		if(x < width && y < height)
	// 			layer[x + y * width] = color;
	// 		screen_change(x, y, x + 1, y + 1);
	// 		if(d[0x6] & 0x1) POKE2(d + 0x8, x + 1); /* auto x+1 */
	// 		if(d[0x6] & 0x2) POKE2(d + 0xa, y + 1); /* auto y+1 */
	// 	}
	// 	break;
	// }
	// case 0xf: {
	// 	Uint8 i;
	// 	Uint8 ctrl = d[0xf];
	// 	Uint8 move = d[0x6];
	// 	Uint8 length = move >> 4;
	// 	Uint8 twobpp = !!(ctrl & 0x80);
	// 	Uint16 x = PEEK2(d + 0x8);
	// 	Uint16 y = PEEK2(d + 0xa);
	// 	Uint16 addr = PEEK2(d + 0xc);
	// 	Uint16 dx = (move & 0x1) << 3;
	// 	Uint16 dy = (move & 0x2) << 2;
	// 	Uint8 *layer = (ctrl & 0x40) ? uxn_screen.fg : uxn_screen.bg;
	// 	for(i = 0; i <= length; i++) {
	// 		screen_blit(layer, ram, addr, x + dy * i, y + dx * i, ctrl & 0xf, ctrl & 0x10, ctrl & 0x20, twobpp);
	// 		addr += (move & 0x04) << (1 + twobpp);
	// 	}
	// 	screen_change(x, y, x + dy * length + 8, y + dx * length + 8);
	// 	if(move & 0x1) POKE2(d + 0x8, x + dx); /* auto x+8 */
	// 	if(move & 0x2) POKE2(d + 0xa, y + dy); /* auto y+8 */
	// 	if(move & 0x4) POKE2(d + 0xc, addr);   /* auto addr+length */
	// 	break;
	// }
	// }
}

void screen_palette(uint8_t device_port) {
	// TODO: implement
}

void file_deo(uint1_t file_index, uint8_t d, uint8_t p) {
	// TODO: implement
}

void uxn_deo(uint8_t addr)
{
	/*
	Varvara
	00  system	    80	controller
	10  console     90	mouse
	20  screen      a0	file
	30  audio	    b0
	40              c0	datetime
	50              d0	Reserved
	60              e0
	70  	        f0
	*/
	static uint8_t device_port;
	static uint8_t device_index;
	static uint1_t port_range_palette_lo;
	static uint1_t port_range_palette_hi;
	device_port = addr & 0x0F;
	device_index = addr & 0xF0;
	if (device_index == 0x00) { // system
		system_deo(peek_dev(device_index), device_port);
		port_range_palette_lo = device_port > 0x07 ? 1 : 0;
		port_range_palette_hi = device_port < 0x0E ? 1 : 0;
		if (port_range_palette_lo & port_range_palette_hi) {
			// set system palette colors
			//----------------------------------
			// color0  color1  color2  color3
			// #fff	   #000    #7ec    #f00
			//----------------------------------
			// #f07f .System/r DEO2
			// #f0e0 .System/g DEO2
			// #f0c0 .System/b DEO2
			screen_palette(peek_dev(0x08));
		}
	}
	else if (device_index == 0x10) { // console
		console_deo(peek_dev(device_index), device_port);
	}
	else if (device_index == 0x20) { // screen
		screen_deo(peek_dev(device_index), device_port);
	}
	else if (device_index == 0xA0) { // file 1
		file_deo(0, peek_dev(device_index), device_port);
	}
	else if (device_index == 0xB0) { // file 2
		file_deo(1, peek_dev(device_index), device_port);
	}
}

void deo(uint8_t device_address, uint8_t value) {
	// #define DEO(a, b) { u->dev[(a)] = (b); if((deo_mask[(a) >> 4] >> ((a) & 0xf)) & 0x1) uxn_deo(u, (a)); }
	static uint16_t deo_mask[16] = {0xff28, 0x0300, 0xc028, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0xa260, 0xa260, 0x0000, 0x0000, 0x0000, 0x0000};
	poke_dev(device_address, value);
	if ((deo_mask[(device_address) >> 4] >> ((device_address) & 0x0F)) & 0x0001) {
		uxn_deo(device_address); 
	}
}

void system_deo_phased() {
	
}

uint1_t deo_phased(uint4_t phase, uint8_t device_address, uint8_t value) {
	static uint16_t deo_mask[16] = {0xff28, 0x0300, 0xc028, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0xa260, 0xa260, 0x0000, 0x0000, 0x0000, 0x0000};
	static uint8_t device_port, device_base_address;
	static uint1_t port_range_palette_lo, port_range_palette_hi, result;
	
	if (phase == 0x0) {
		result = 0;
		device_port = device_address & 0x0F;
		device_base_address = device_address & 0xF0;
		poke_dev(device_address, value); // START
	}
	else if (phase == 0x1) {
		result = ~((deo_mask[(device_address) >> 4] >> ((device_address) & 0x0F)) & 0x0001);
		if (device_base_address == 0x00) {      // system
			
		}
		else if (device_base_address == 0x10) { // console
			
		}
		else if (device_base_address == 0x20) { // screen
			result = screen_deo_phased(0x0, device_base_address, device_port);
		}
		else if (device_base_address == 0xA0) { // file 1
			
		}
		else if (device_base_address == 0xB0) { // file 2
			
		}
	}
	else if (phase == 0x2) {
		if (device_base_address == 0x00) {		 // system
			
		}
		else if (device_base_address == 0x10) { // console
			
		}
		else if (device_base_address == 0x20) { // screen
			result = screen_deo_phased(0x1, device_base_address, device_port);
		}
		else if (device_base_address == 0xA0) { // file 1
			
		}
		else if (device_base_address == 0xB0) { // file 2
			
		}
	}
	else if (phase == 0x3) {
		if (device_base_address == 0x00) {		 // system
			
		}
		else if (device_base_address == 0x10) { // console
			
		}
		else if (device_base_address == 0x20) { // screen
			result = screen_deo_phased(0x2, device_base_address, device_port);
		}
		else if (device_base_address == 0xA0) { // file 1
			
		}
		else if (device_base_address == 0xB0) { // file 2
			
		}
	}
	else if (phase == 0x4) {
		if (device_base_address == 0x00) {		 // system
			
		}
		else if (device_base_address == 0x10) { // console
			
		}
		else if (device_base_address == 0x20) { // screen
			result = screen_deo_phased(0x3, device_base_address, device_port);
		}
		else if (device_base_address == 0xA0) { // file 1
			
		}
		else if (device_base_address == 0xB0) { // file 2
			
		}
	}
	else if (phase == 0x5) {
		if (device_base_address == 0x00) {		 // system
			
		}
		else if (device_base_address == 0x10) { // console
			
		}
		else if (device_base_address == 0x20) { // screen
			result = screen_deo_phased(0x4, device_base_address, device_port);
		}
		else if (device_base_address == 0xA0) { // file 1
			
		}
		else if (device_base_address == 0xB0) { // file 2
			
		}
	}
	else if (phase == 0x6) {
		if (device_base_address == 0x00) {		 // system
			
		}
		else if (device_base_address == 0x10) { // console
			
		}
		else if (device_base_address == 0x20) { // screen
			result = screen_deo_phased(0x5, device_base_address, device_port);
		}
		else if (device_base_address == 0xA0) { // file 1
			
		}
		else if (device_base_address == 0xB0) { // file 2
			
		}
	}
	else if (phase == 0x7) {
		if (device_base_address == 0x00) {		 // system
			
		}
		else if (device_base_address == 0x10) { // console
			
		}
		else if (device_base_address == 0x20) { // screen
			result = screen_deo_phased(0x6, device_base_address, device_port);
		}
		else if (device_base_address == 0xA0) { // file 1
			
		}
		else if (device_base_address == 0xB0) { // file 2
			
		}
	}
	else if (phase == 0x8) {
		if (device_base_address == 0x00) {		 // system
			
		}
		else if (device_base_address == 0x10) { // console
			
		}
		else if (device_base_address == 0x20) { // screen
			result = screen_deo_phased(0x7, device_base_address, device_port);
		}
		else if (device_base_address == 0xA0) { // file 1
			
		}
		else if (device_base_address == 0xB0) { // file 2
			
		}
	}
	else if (phase == 0x9) {
		if (device_base_address == 0x00) {		 // system
			
		}
		else if (device_base_address == 0x10) { // console
			
		}
		else if (device_base_address == 0x20) { // screen
			result = screen_deo_phased(0x8, device_base_address, device_port);
		}
		else if (device_base_address == 0xA0) { // file 1
			
		}
		else if (device_base_address == 0xB0) { // file 2
			
		}
	}
	
	return result;
}
