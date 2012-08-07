/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Hash module unit tests
 */

#include <string.h>

#include "common.h"
#include "tests.h"
#include "hash_table.h"


void hash_table_test (void)
{
    char *ak = "one",
         *bk = "two",
         *ck = "three";

    char *ad = "one data",
         *bd = "two datums",
         *cd = "three datas";


    hash_table_t *tbl = hash_table_new(16, 0.5f, 1.0f);

    hash_table_add(tbl, ak, (void *)ad);
    hash_table_add(tbl, bk, (void *)bd);
    hash_table_add(tbl, ck, (void *)cd);

    assert(!strcmp((char *)hash_table_find(tbl, ak), ad));
    assert(!strcmp((char *)hash_table_find(tbl, bk), bd));
    assert(!strcmp((char *)hash_table_find(tbl, ck), cd));

    assert(hash_table_del(tbl, ak) == 1);

    assert(hash_table_find(tbl, ak) == NULL);
    assert(!strcmp((char *)hash_table_find(tbl, bk), bd));
    assert(!strcmp((char *)hash_table_find(tbl, ck), cd));

    assert(hash_table_del(tbl, ak) == 0);
    assert(hash_table_del(tbl, bk) == 1);
    assert(hash_table_del(tbl, ck) == 1);

    assert(hash_table_find(tbl, ak) == NULL);
    assert(hash_table_find(tbl, bk) == NULL);
    assert(hash_table_find(tbl, ck) == NULL);

    hash_table_add(tbl, ak, (void *)ad);
    hash_table_add(tbl, bk, (void *)bd);
    hash_table_add(tbl, ck, (void *)cd);


    /* TODO: iterator tests */
}
