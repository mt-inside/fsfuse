#include <stdio.h>

#include "hash.h"


int main (int argc, char *argv[])
{
    char *ak = "one",
         *bk = "two",
         *ck = "three";

    char *ad = "one data",
         *bd = "two datums",
         *cd = "three datas";


    hash_table_t *tbl = hash_new(16);
    hash_iterator_t *iter;

    hash_add(tbl, ak, (void *)ad);
    hash_add(tbl, bk, (void *)bd);
    hash_add(tbl, ck, (void *)cd);

    printf("one: %s\n", (char *)hash_find(tbl, ak));
    printf("two: %s\n", (char *)hash_find(tbl, bk));
    printf("three: %s\n", (char *)hash_find(tbl, ck));

    hash_del(tbl, ak);

    printf("one: %s\n", (char *)hash_find(tbl, ak));
    printf("two: %s\n", (char *)hash_find(tbl, bk));
    printf("three: %s\n", (char *)hash_find(tbl, ck));

    hash_del(tbl, ak);
    hash_del(tbl, bk);
    hash_del(tbl, ck);

    printf("one: %s\n", (char *)hash_find(tbl, ak));
    printf("two: %s\n", (char *)hash_find(tbl, bk));
    printf("three: %s\n", (char *)hash_find(tbl, ck));

    hash_add(tbl, ak, (void *)ad);
    hash_add(tbl, bk, (void *)bd);
    hash_add(tbl, ck, (void *)cd);

    iter = hash_iterator_new(tbl);

    while (!hash_iterator_at_end(iter))
    {
        printf("%s: %s\n",
                hash_iterator_current_key(iter),
                hash_iterator_current_data(iter)
              );

        hash_iterator_next(iter);
    }


    return 0;
}
