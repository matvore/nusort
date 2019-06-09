CC = cc -Werror -Wall -Wdeclaration-after-statement -Wformat-security -Wno-format-zero-length -Wold-style-definition -Woverflow -Wpointer-arith -Wstrict-prototypes -Wunused -Wvla

all: nusort

nusort: main.c romazi.c kanji_db.c util.c
	$(CC) -o nusort *.c
