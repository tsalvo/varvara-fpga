#include <stdio.h>

/*
Copyright (c) 2021-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define TRIM 0x0100
#define LENGTH 0x10000

typedef unsigned char Uint8;
typedef signed char Sint8;
typedef unsigned short Uint16;

typedef struct {
	char name[0x40], items[0x40][0x40];
	Uint8 len;
} Macro;

typedef struct {
	char name[0x40];
	Uint16 addr, refs;
} Label;

typedef struct {
	char name[0x40], rune;
	Uint16 addr;
} Reference;

typedef struct {
	Uint8 data[LENGTH];
	Uint8 lambda_stack[0x100], lambda_ptr, lambda_count;
	char scope[0x40], lambda[0x10];
	unsigned int ptr, length;
	Uint16 label_len, macro_len, refs_len;
	Label labels[0x400];
	Macro macros[0x100];
	Reference refs[0x800];
} Program;

Program p;

/* clang-format off */

static char ops[][4] = {
	"LIT", "INC", "POP", "NIP", "SWP", "ROT", "DUP", "OVR",
	"EQU", "NEQ", "GTH", "LTH", "JMP", "JCN", "JSR", "STH",
	"LDZ", "STZ", "LDR", "STR", "LDA", "STA", "DEI", "DEO",
	"ADD", "SUB", "MUL", "DIV", "AND", "ORA", "EOR", "SFT"
};

static int   scmp(char *a, char *b, int len) { int i = 0; while(a[i] == b[i]) if(!a[i] || ++i >= len) return 1; return 0; } /* string compare */
static int   sihx(char *s) { int i = 0; char c; while((c = s[i++])) if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) return 0; return i > 1; } /* string is hexadecimal */
static int   shex(char *s) { int n = 0, i = 0; char c; while((c = s[i++])) if(c >= '0' && c <= '9') n = n * 16 + (c - '0'); else if(c >= 'a' && c <= 'f') n = n * 16 + 10 + (c - 'a'); return n; } /* string to num */
static int   slen(char *s) { int i = 0; while(s[i]) i++; return i; } /* string length */
static int   spos(char *s, char c) { Uint8 i = 0, j; while((j = s[i++])) if(j == c) return i; return -1; } /* character position */
static char *scpy(char *src, char *dst, int len) { int i = 0; while((dst[i] = src[i]) && i < len - 2) i++; dst[i + 1] = '\0'; return dst; } /* string copy */
static char *scat(char *dst, const char *src) { char *ptr = dst + slen(dst); while(*src) *ptr++ = *src++; *ptr = '\0'; return dst; } /* string cat */

/* clang-format on */

static int parse(char *w, FILE *f);

static int
error(const char *name, const char *msg)
{
	fprintf(stderr, "%s: %s\n", name, msg);
	return 0;
}

static char *
sublabel(char *src, char *scope, char *name)
{
	if(slen(scope) + slen(name) >= 0x3f) {
		error("Sublabel length too long", name);
		return NULL;
	}
	return scat(scat(scpy(scope, src, 0x40), "/"), name);
}

static Macro *
findmacro(char *name)
{
	int i;
	for(i = 0; i < p.macro_len; i++)
		if(scmp(p.macros[i].name, name, 0x40))
			return &p.macros[i];
	return NULL;
}

static Label *
findlabel(char *name)
{
	int i;
	for(i = 0; i < p.label_len; i++)
		if(scmp(p.labels[i].name, name, 0x40))
			return &p.labels[i];
	return NULL;
}

static Uint8
findopcode(char *s)
{
	int i;
	for(i = 0; i < 0x20; i++) {
		int m = 0;
		if(!scmp(ops[i], s, 3))
			continue;
		if(!i) i |= (1 << 7); /* force keep for LIT */
		while(s[3 + m]) {
			if(s[3 + m] == '2')
				i |= (1 << 5); /* mode: short */
			else if(s[3 + m] == 'r')
				i |= (1 << 6); /* mode: return */
			else if(s[3 + m] == 'k')
				i |= (1 << 7); /* mode: keep */
			else
				return 0; /* failed to match */
			m++;
		}
		return i;
	}
	return 0;
}

static int
makemacro(char *name, FILE *f)
{
	Macro *m;
	char word[0x40];
	if(findmacro(name))
		return error("Macro duplicate", name);
	if(sihx(name) && slen(name) % 2 == 0)
		return error("Macro name is hex number", name);
	if(findopcode(name) || scmp(name, "BRK", 4) || !slen(name))
		return error("Macro name is invalid", name);
	if(p.macro_len == 0x100)
		return error("Macros limit exceeded", name);
	m = &p.macros[p.macro_len++];
	scpy(name, m->name, 0x40);
	while(fscanf(f, "%63s", word) == 1) {
		if(word[0] == '{') continue;
		if(word[0] == '}') break;
		if(word[0] == '%')
			return error("Macro error", name);
		if(m->len >= 0x40)
			return error("Macro size exceeded", name);
		scpy(word, m->items[m->len++], 0x40);
	}
	return 1;
}

static int
makelabel(char *name)
{
	Label *l;
	if(findlabel(name))
		return error("Label duplicate", name);
	if(sihx(name) && (slen(name) == 2 || slen(name) == 4))
		return error("Label name is hex number", name);
	if(findopcode(name) || scmp(name, "BRK", 4) || !slen(name))
		return error("Label name is invalid", name);
	if(p.label_len == 0x400)
		return error("Labels limit exceeded", name);
	l = &p.labels[p.label_len++];
	l->addr = p.ptr;
	l->refs = 0;
	scpy(name, l->name, 0x40);
	return 1;
}

static char *
makelambda(int id)
{
	scpy("lambda", p.lambda, 0x07);
	p.lambda[6] = '0' + (id >> 0x4);
	p.lambda[7] = '0' + (id & 0xf);
	return p.lambda;
}

static int
makereference(char *scope, char *label, char rune, Uint16 addr)
{
	char subw[0x40], parent[0x40];
	Reference *r;
	if(p.refs_len >= 0x800)
		return error("References limit exceeded", label);
	r = &p.refs[p.refs_len++];
	if(label[0] == '{') {
		p.lambda_stack[p.lambda_ptr++] = p.lambda_count;
		scpy(makelambda(p.lambda_count++), r->name, 0x40);
	} else if(label[0] == '&') {
		if(!sublabel(subw, scope, label + 1))
			return error("Invalid sublabel", label);
		scpy(subw, r->name, 0x40);
	} else {
		int pos = spos(label, '/');
		if(pos > 0) {
			Label *l;
			if((l = findlabel(scpy(label, parent, pos))))
				l->refs++;
		}
		scpy(label, r->name, 0x40);
	}
	r->rune = rune;
	r->addr = addr;
	return 1;
}

static int
writebyte(Uint8 b)
{
	if(p.ptr < TRIM)
		return error("Writing in zero-page", "");
	else if(p.ptr > 0xffff)
		return error("Writing after the end of RAM", "");
	else if(p.ptr < p.length)
		return error("Memory overwrite", "");
	p.data[p.ptr++] = b;
	p.length = p.ptr;
	return 1;
}

static int
writeopcode(char *w)
{
	return writebyte(findopcode(w));
}

static int
writeshort(Uint16 s, int lit)
{
	if(lit)
		if(!writebyte(findopcode("LIT2"))) return 0;
	return writebyte(s >> 8) && writebyte(s & 0xff);
}

static int
writelitbyte(Uint8 b)
{
	return writebyte(findopcode("LIT")) && writebyte(b);
}

static int
doinclude(const char *filename)
{
	FILE *f;
	char w[0x40];
	if(!(f = fopen(filename, "r")))
		return error("Include missing", filename);
	while(fscanf(f, "%63s", w) == 1)
		if(!parse(w, f))
			return error("Unknown token", w);
	fclose(f);
	return 1;
}

static int
parse(char *w, FILE *f)
{
	int i;
	char word[0x40], subw[0x40], c;
	Label *l;
	Macro *m;
	if(slen(w) >= 63)
		return error("Invalid token", w);
	switch(w[0]) {
	case '(': /* comment */
		if(slen(w) != 1) fprintf(stderr, "-- Malformed comment: %s\n", w);
		i = 1; /* track nested comment depth */
		while(fscanf(f, "%63s", word) == 1) {
			if(slen(word) != 1)
				continue;
			else if(word[0] == '(')
				i++;
			else if(word[0] == ')' && --i < 1)
				break;
		}
		break;
	case '~': /* include */
		if(!doinclude(w + 1))
			return error("Invalid include", w);
		break;
	case '%': /* macro */
		if(!makemacro(w + 1, f))
			return error("Invalid macro", w);
		break;
	case '|': /* pad-absolute */
		if(sihx(w + 1))
			p.ptr = shex(w + 1);
		else if(w[1] == '&') {
			if(!sublabel(subw, p.scope, w + 2) || !(l = findlabel(subw)))
				return error("Invalid sublabel", w);
			p.ptr = l->addr;
		} else {
			if(!(l = findlabel(w + 1)))
				return error("Invalid label", w);
			p.ptr = l->addr;
		}
		break;
	case '$': /* pad-relative */
		if(sihx(w + 1))
			p.ptr += shex(w + 1);
		else if(w[1] == '&') {
			if(!sublabel(subw, p.scope, w + 2) || !(l = findlabel(subw)))
				return error("Invalid sublabel", w);
			p.ptr += l->addr;
		} else {
			if(!(l = findlabel(w + 1)))
				return error("Invalid label", w);
			p.ptr += l->addr;
		}
		break;
	case '@': /* label */
		if(!makelabel(w + 1))
			return error("Invalid label", w);
		scpy(w + 1, p.scope, 0x40);
		break;
	case '&': /* sublabel */
		if(!sublabel(subw, p.scope, w + 1) || !makelabel(subw))
			return error("Invalid sublabel", w);
		break;
	case '#': /* literals hex */
		if(sihx(w + 1) && slen(w) == 3)
			return writelitbyte(shex(w + 1));
		else if(sihx(w + 1) && slen(w) == 5)
			return writeshort(shex(w + 1), 1);
		else
			return error("Invalid hex literal", w);
		break;
	case '_': /* raw byte relative */
		makereference(p.scope, w + 1, w[0], p.ptr);
		return writebyte(0xff);
	case ',': /* literal byte relative */
		makereference(p.scope, w + 1, w[0], p.ptr + 1);
		return writelitbyte(0xff);
	case '-': /* raw byte absolute */
		makereference(p.scope, w + 1, w[0], p.ptr);
		return writebyte(0xff);
	case '.': /* literal byte zero-page */
		makereference(p.scope, w + 1, w[0], p.ptr + 1);
		return writelitbyte(0xff);
	case ':':
	case '=': /* raw short absolute */
		makereference(p.scope, w + 1, w[0], p.ptr);
		return writeshort(0xffff, 0);
	case ';': /* literal short absolute */
		makereference(p.scope, w + 1, w[0], p.ptr + 1);
		return writeshort(0xffff, 1);
	case '?': /* JCI */
		makereference(p.scope, w + 1, w[0], p.ptr + 1);
		return writebyte(0x20) && writeshort(0xffff, 0);
	case '!': /* JMI */
		makereference(p.scope, w + 1, w[0], p.ptr + 1);
		return writebyte(0x40) && writeshort(0xffff, 0);
	case '"': /* raw string */
		i = 0;
		while((c = w[++i]))
			if(!writebyte(c)) return 0;
		break;
	case '}': /* lambda end */
		if(!makelabel(makelambda(p.lambda_stack[--p.lambda_ptr])))
			return error("Invalid label", w);
		break;
	case '[':
	case ']':
		if(slen(w) == 1) break; /* else fallthrough */
	default:
		/* opcode */
		if(findopcode(w) || scmp(w, "BRK", 4))
			return writeopcode(w);
		/* raw byte */
		else if(sihx(w) && slen(w) == 2)
			return writebyte(shex(w));
		/* raw short */
		else if(sihx(w) && slen(w) == 4)
			return writeshort(shex(w), 0);
		/* macro */
		else if((m = findmacro(w))) {
			for(i = 0; i < m->len; i++)
				if(!parse(m->items[i], f))
					return 0;
			return 1;
		} else {
			makereference(p.scope, w, ' ', p.ptr + 1);
			return writebyte(0x60) && writeshort(0xffff, 0);
		}
	}
	return 1;
}

static int
resolve(void)
{
	Label *l;
	int i;
	Uint16 a;
	for(i = 0; i < p.refs_len; i++) {
		Reference *r = &p.refs[i];
		switch(r->rune) {
		case '_':
		case ',':
			if(!(l = findlabel(r->name)))
				return error("Unknown relative reference", r->name);
			p.data[r->addr] = (Sint8)(l->addr - r->addr - 2);
			if((Sint8)p.data[r->addr] != (l->addr - r->addr - 2))
				return error("Relative reference is too far", r->name);
			l->refs++;
			break;
		case '-':
		case '.':
			if(!(l = findlabel(r->name)))
				return error("Unknown zero-page reference", r->name);
			p.data[r->addr] = l->addr & 0xff;
			l->refs++;
			break;
		case ':':
		case '=':
		case ';':
			if(!(l = findlabel(r->name)))
				return error("Unknown absolute reference", r->name);
			p.data[r->addr] = l->addr >> 0x8;
			p.data[r->addr + 1] = l->addr & 0xff;
			l->refs++;
			break;
		case '?':
		case '!':
		default:
			if(!(l = findlabel(r->name)))
				return error("Unknown absolute reference", r->name);
			a = l->addr - r->addr - 2;
			p.data[r->addr] = a >> 0x8;
			p.data[r->addr + 1] = a & 0xff;
			l->refs++;
			break;
		}
	}
	return 1;
}

static int
assemble(FILE *f)
{
	char w[0x40];
	p.ptr = 0x100;
	scpy("on-reset", p.scope, 0x40);
	while(fscanf(f, "%62s", w) == 1)
		if(slen(w) > 0x3d || !parse(w, f))
			return error("Invalid token", w);
	return resolve();
}

static void
review(char *filename)
{
	int i;
	for(i = 0; i < p.label_len; i++)
		if(p.labels[i].name[0] >= 'A' && p.labels[i].name[0] <= 'Z')
			continue; /* Ignore capitalized labels(devices) */
		else if(!p.labels[i].refs)
			fprintf(stderr, "-- Unused label: %s\n", p.labels[i].name);
	fprintf(stderr,
		"Assembled %s in %d bytes(%.2f%% used), %d labels, %d macros.\n",
		filename,
		p.length - TRIM,
		(p.length - TRIM) / 652.80,
		p.label_len,
		p.macro_len);
}

static void
writesym(char *filename)
{
	int i;
	char symdst[0x60];
	FILE *fp;
	if(slen(filename) > 0x60 - 5)
		return;
	fp = fopen(scat(scpy(filename, symdst, slen(filename) + 1), ".sym"), "w");
	if(fp != NULL) {
		for(i = 0; i < p.label_len; i++) {
			Uint8 hb = p.labels[i].addr >> 8, lb = p.labels[i].addr & 0xff;
			fwrite(&hb, 1, 1, fp);
			fwrite(&lb, 1, 1, fp);
			fwrite(p.labels[i].name, slen(p.labels[i].name) + 1, 1, fp);
		}
	}
	fclose(fp);
}

int
main(int argc, char *argv[])
{
	FILE *src, *dst;
	if(argc == 1)
		return error("usage", "uxnasm [-v] input.tal output.rom");
	if(argv[1][0] == '-' && argv[1][1] == 'v')
		return !fprintf(stdout, "Uxnasm - Uxntal Assembler, 8 Aug 2023.\n");
	if(!(src = fopen(argv[1], "r")))
		return !error("Invalid input", argv[1]);
	if(!assemble(src))
		return !error("Assembly", "Failed to assemble rom.");
	if(!(dst = fopen(argv[2], "wb")))
		return !error("Invalid Output", argv[2]);
	if(p.length <= TRIM)
		return !error("Assembly", "Output rom is empty.");
	fwrite(p.data + TRIM, p.length - TRIM, 1, dst);
	review(argv[2]);
	writesym(argv[2]);
	return 0;
}
