
	start_test(__FILE__, "test");
	{
		size_t i;
		int *c = xcalloc(4000, sizeof(*c));
		for (i = 0; i < 4000; i++)
			c[i] = i + 1;
		srand(45);
		for (i = 0; i < 3999; i++) {
			size_t swap_with = rand() % (4000 - i) + i;
			int tmp = c[i];
			c[i] = c[swap_with];
			c[swap_with] = tmp;
		}
		rank_coverage_reset(1208, 30);
		for (i = 0; i < 4000; i++)
			rank_coverage_add_kanji(c[i]);
	}
	end_test("");
