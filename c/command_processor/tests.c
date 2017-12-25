/*
 * Copyright (C) 2017 SilkID 
 * tests.c: Registers all possible test cases
 *
 * Author: Prem Mallappa <prem@silkid.com>
 */
#include <stdio.h>

#include "tests.h"

extern test_init __test_section_start, __test_section_end;

void init_tests(void)
{
	test_init *fn_entry;
	int ret = 0;

	fn_entry = &__test_section_start;

	do {
		printf("registering: %p\n", fn_entry);
		ret = (*fn_entry)();
		if (ret != 0) {
			printf("Tests register returned error, continuing\n");
		}
		++fn_entry;
	} while(fn_entry < &__test_section_end);
}

