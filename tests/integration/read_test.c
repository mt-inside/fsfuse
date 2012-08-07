/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * File read (read()) integration tests
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


static void test_zero_len (const char * const test_dir);
static void test_1_nul    (const char * const test_dir);
static void test_1_rand   (const char * const test_dir);
static void test_1M_nul   (const char * const test_dir);
static void test_1M_rand  (const char * const test_dir);


void read_test (const char * const test_dir)
{
    test_zero_len(test_dir);
    test_1_nul   (test_dir);
    test_1_rand  (test_dir);
    test_1M_nul  (test_dir);
    test_1M_rand (test_dir);
}

static void test_zero_len (const char * const test_dir)
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

static void test_1_nul (const char * const test_dir)
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

static void test_1_rand (const char * const test_dir)
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

static void test_1M_nul (const char * const test_dir)
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

static void test_1M_rand (const char * const test_dir)
{
    NOT_USED(test_dir);
    /* TODO: check data at various points, as above */
    /* TODO: md5sum all data and check checksum */
}
