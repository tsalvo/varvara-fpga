#include <stdio.h>
#include <stdlib.h>

#include "uxn.h"
#include "devices/system.h"
#include "devices/file.h"
#include "devices/datetime.h"

/*
Copyright (c) 2021-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

Uint16 deo_mask[] = {0xc028, 0x0300, 0xc028, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0xa260, 0xa260, 0x0000, 0x0000, 0x0000, 0x0000};
Uint16 dei_mask[] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x07ff, 0x0000, 0x0000, 0x0000};

Uint8
emu_dei(Uxn *u, Uint8 addr)
{
	switch(addr & 0xf0) {
	case 0xc0: return datetime_dei(u, addr);
	}
	return u->dev[addr];
}

void
emu_deo(Uxn *u, Uint8 addr)
{
	Uint8 p = addr & 0x0f, d = addr & 0xf0;
	switch(d) {
	case 0x00: system_deo(u, &u->dev[d], p); break;
	case 0x10: console_deo(&u->dev[d], p); break;
	case 0xa0: file_deo(0, u->ram, &u->dev[d], p); break;
	case 0xb0: file_deo(1, u->ram, &u->dev[d], p); break;
	}
}

int
main(int argc, char **argv)
{
	Uxn u;
	int i = 1;
	if(i == argc)
		return system_error("usage", "uxncli file.rom [args..]");
	if(!uxn_boot(&u, (Uint8 *)calloc(0x10000 * RAM_PAGES, sizeof(Uint8))))
		return system_error("Boot", "Failed");
	if(!system_load(&u, argv[i++]))
		return system_error("Load", "Failed");
	u.dev[0x17] = argc - i;
	if(uxn_eval(&u, PAGE_PROGRAM)) {
		for(; i < argc; i++) {
			char *p = argv[i];
			while(*p) console_input(&u, *p++, CONSOLE_ARG);
			console_input(&u, '\n', i == argc - 1 ? CONSOLE_END : CONSOLE_EOA);
		}
		while(!u.dev[0x0f]) {
			int c = fgetc(stdin);
			if(c == EOF) break;
			console_input(&u, (Uint8)c, CONSOLE_STD);
		}
	}
	free(u.ram);
	return u.dev[0x0f] & 0x7f;
}
