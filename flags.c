#include "flags.h"
#include "streams.h"
#include "util.h"

#include <string.h>

static int namecmp(struct flagset *fs, unsigned a, unsigned b)
{
	const char *an =	fs->el[a].name;
	int cmp = strcmp(an,	fs->el[b].name);

	if (!cmp) DIE(0, "フラグ名がダブっています: %s", an);
	return cmp;
}

static struct flag *find(struct flagset *fs, const char *name, int req)
{
	struct flag *f;
	if (!fs->sorted) {
		QSORT(,	fs->el, fs->cnt, namecmp(fs, a, b) < 0);
		fs->sorted = 1;
	}

	BSEARCH(f, fs->el, fs->cnt, strcmp(f->name, name));
	if (req && !f) DIE(0, "フラグが見つかりません: '%s'", name);
	return f;
}

void flagcat(struct flagset *fs, const char *catname)
{
	if (fs->doc_out) {
		fprintf(fs->doc_out, "%s%sフラグ:\n",
			fs->past_first_cat ? "\n" : "",
			catname);
	}

	if (fs->categ_out) {
		fprintf(fs->categ_out, "%s%sフラグ",
			fs->past_first_cat ? "|" : " {",
			catname);
	}

	fs->past_first_cat = 1;
}

void addflag(	struct flagset *fs,
		const char *name, char type, unsigned char initialval,
		const char *doc)
{
	struct flag *nf;

	if (!name) DIE(0, "null name?");

	if (fs->doc_out		) fprintf(fs->doc_out, "%s:%s\n", name, doc);
	if (fs->not_collecting	) return;

	GROW_ARRAY_BY(*fs, 1);
	nf = fs->el + fs->cnt - 1;
	nf->name = name;
	nf->type = type;
	nf->value = initialval;
}

void setflag(struct flagset *fs, const char *name, unsigned char value)
{
	find(fs, name, 1)->value = value;
}

int flagval(struct flagset *fs, const char *name)
{
	return find(fs, name, 1)->value;
}

static void shift(int *argc, char **argv)
{
	if (*argc < 1) return;
	memmove(argv, argv + 1, --(*argc) * sizeof(*argv));
	argv[*argc] = 0;
}

void parsflag(struct flagset *fs, int *argc, char **argv)
{
	struct flag *f;

	if (*argc < 0) DIE(0, "parsflag argc: %d", *argc);

	while (*argc > 0) {
		if (!strcmp(*argv, "--")) { shift(argc, argv); return; }

		f = find(fs, *argv, 0);
		if (!f) return;

		shift(argc, argv);
		switch (f->type) {
		case 'b': f->value = 1; break;
		case 'c':
			if (!*argc || strlen(*argv) != 1)
				DIE(0,	"%sの値として一文字を渡してください",
					f->name);
			f->value = **argv;
			shift(argc, argv);
			break;
		default: DIE(0,	"バグ: フラグの type が無効です: %c (%x)",
				f->type, f->type);
		}
	}
}

void destroy_flagset(struct flagset *fs)
{
	DESTROY_ARRAY(*fs);
	memset(fs, 0, sizeof *fs);
}
