#ifndef FLAGS_H
#define FLAGS_H

#include <stdio.h>

struct flag {
	const char *name;
	char type;
	unsigned char value;
};

struct flagset {
	struct flag *el;

	unsigned cnt, alloc;

	FILE *categ_out, *doc_out;

	unsigned sorted		: 1;
	unsigned not_collecting	: 1;
	unsigned past_first_cat	: 1;
};

void flagcat(	struct flagset *, const char *catname);
void addflag(	struct flagset *,
		const char *name, char type, unsigned char initialval,
		const char *doc);
void setflag(	struct flagset *, const char *name, unsigned char value);
int flagval(	struct flagset *, const char *name);

/*
 * argcとargvの差すラグがを一つずつ読み、有効なフラグであれば、flagsetに値を書き
 * 込み、argcとargvを先に進めます。フラグではないargvが見つかるかargvをすべて処
 * 理するか"--"を処理すれば関数から戻ります。
 */
void parsflag(struct flagset *, int *argc, char **argv);

void destroy_flagset(struct flagset *);

#endif
