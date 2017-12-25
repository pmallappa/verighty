/*
 * Copyright (C) 2017 SilkID
 * tests.h: All tests registration functions should go here
 *
 * Author: Prem Mallappa <prem@silkid.com>
 */
#ifndef __TESTS_H__
#define __TESTS_H__

typedef int (*test_init)(void);

#define INITCALL	__attribute__((section ("tests_section")))

#define INIT_FUNCTION(x)	test_init _##x INITCALL = x

void init_tests(void);
 
#endif  /* TESTS_H */
