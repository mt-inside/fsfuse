/*
 * File read (read()) integration tests
 *
 * Copyright (C) Matthew Turner 2008-2012. All rights reserved.
 *
 * $Id: hash_table_test.c 595 2012-03-27 13:53:09Z matt $
 */

/* Note that testing the result of lseek() in these tests is a bit pointless
 * because you can ask to seek beyond the end of the file and that's quite
 * alright. Seek returns where you asked to seek to, not truncating it to the
 * length of the file. It's not an error to read from there either; you'll just
 * get 0 bytes */

#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "tests.h"
#include "utils.h"


#define BUF_LEN 1024


static const char * const test_dir = "../../../tests/testdata/file_contents";


static void test_zero_len (void);
static void test_1_nul    (void);
static void test_1_rand   (void);
static void test_1M_nul   (void);
static void test_1M_rand  (void);


void read_test (void)
{
    test_zero_len();
    test_1_nul();
    test_1_rand();
    test_1M_nul();
    test_1M_rand();
}

static void test_zero_len (void)
{
    unsigned char buf[BUF_LEN];
    int fd = open(path_combine(test_dir, "zero_length"), O_RDONLY);
    int rc;


    assert(fd != -1);

    rc = read(fd, buf, BUF_LEN);
    assert(rc == 0);

    rc = read(fd, buf, BUF_LEN);
    assert(rc == 0);
}

static void test_1_nul (void)
{
    unsigned char buf[BUF_LEN];
    int fd = open(path_combine(test_dir, "1_nul"), O_RDONLY);
    int rc;


    assert(fd != -1);

    rc = read(fd, buf, BUF_LEN);
    assert(rc == 1);
    assert(buf[0] == '\0');

    rc = read(fd, buf, BUF_LEN);
    assert(rc == 0);
}

static void test_1_rand (void)
{
    unsigned char buf[BUF_LEN];
    int fd = open(path_combine(test_dir, "1_rand"), O_RDONLY);
    int rc;


    assert(fd != -1);

    rc = read(fd, buf, BUF_LEN);
    assert(rc == 1);
    assert(buf[0] == 'x');

    rc = read(fd, buf, BUF_LEN);
    assert(rc == 0);
}

static void test_1M_nul (void)
{
    unsigned char buf[BUF_LEN];
    int fd = open(path_combine(test_dir, "1M_nul"), O_RDONLY);
    int rc;


    assert(fd != -1);

    rc = read(fd, buf, BUF_LEN);
    assert(rc == BUF_LEN);
    assert(buf[0] == '\0');
    assert(buf[BUF_LEN - 1] == '\0');

    rc = read(fd, buf, BUF_LEN);
    assert(rc == BUF_LEN);
    assert(buf[0] == '\0');
    assert(buf[BUF_LEN - 1] == '\0');

    rc = lseek(fd, 1 << 19, SEEK_SET);
    assert(rc == 1 << 19);
    rc = read(fd, buf, BUF_LEN);
    assert(rc == BUF_LEN);
    assert(buf[0] == '\0');
    assert(buf[BUF_LEN - 1] == '\0');

    rc = lseek(fd, -5, SEEK_END);
    assert(rc == (1 << 20) - 5);
    rc = read(fd, buf, BUF_LEN);
    assert(rc == 5);
    assert(buf[0] == '\0');
    assert(buf[4] == '\0');

    rc = lseek(fd, 1 << 21, SEEK_SET);
    assert(rc == 1 << 21);
    rc = read(fd, buf, BUF_LEN);
    assert(rc == 0);


    /* TODO: md5sum all data and check checksum */
}

static void test_1M_rand (void)
{
    /* TODO: check data at various points, as above */
    /* TODO: md5sum all data and check checksum */
}
