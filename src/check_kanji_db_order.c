#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int process_rad_so_line(const char *line)
{
	const char *curs = line;
	if (curs[0] == '#' || curs[0] == '\n')
		return 0;
	if (strncmp("U+", curs, 2)) {
		fprintf(stderr,
			"警告: 行の形式が間違っています：%s\n",
			curs);
		return 0;
	}
	curs += 2;
	return 0;
}

int check_kanji_db_order(void)
{
	char rad_so_db_path[512];
	int size = snprintf(
		rad_so_db_path, sizeof(rad_so_db_path),
		"%s/Desktop/Unihan/Unihan_RadicalStrokeCounts.txt",
		getenv("HOME"));
	FILE *db_stream = NULL;
	int res = 0;
	char line[512];

	if (size >= sizeof(rad_so_db_path)) {
		fprintf(stderr, "path too long\n");
		return 1;
	}
	if (!(db_stream = fopen(rad_so_db_path, "r"))) {
		fprintf(stderr, "could not open %s\n:\t%s\n",
			rad_so_db_path, strerror(errno));
		return 2;
	}
	while (fgets(line, sizeof(line), db_stream)) {
		int res = process_rad_so_line(line);
		if (res)
			break;
	}
	if (errno) {
		fprintf(stderr, "error reading %s:\t%s\n", rad_so_db_path,
			strerror(errno));
		res = 3;
	}
	if (fclose(db_stream)) {
		fprintf(stderr,
			"WARNING: did not close %s properly\n:\t%s\n",
			rad_so_db_path, strerror(errno));
	}
	return res;
}
