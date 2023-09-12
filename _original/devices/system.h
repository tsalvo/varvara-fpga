/*
Copyright (c) 2022 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define SYSTEM_VERSION 2
#define SYSTEM_DEIMASK 0x0030
#define SYSTEM_DEOMASK 0xff38

#define RAM_PAGES 0x10

extern char *boot_rom;

void system_connect(Uint8 device, Uint8 ver, Uint16 dei, Uint16 deo);
void system_reboot(Uxn *u, char *rom, int soft);
void system_inspect(Uxn *u);
int system_version(char *emulator, char *date);
int system_error(char *msg, const char *err);
int system_init(Uxn *u, Uint8 *ram, char *rom);

Uint8 system_dei(Uxn *u, Uint8 addr);
void system_deo(Uxn *u, Uint8 *d, Uint8 port);
