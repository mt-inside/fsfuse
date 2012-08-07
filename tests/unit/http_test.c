/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * HTTP module unit tests
 */

#include <string.h>

#include "common.h"
#include "tests.h"
#include "http.h"


extern void http_req_dump_headers (http_req_t *req);


static void test_header_cb (http_req_t *req,
                            void *ctxt,
                            const char *key,
                            const char *value)
{
    printf("test_header_cb( %p, %p, %s, %s )\n",
            (void *)req, (void *)ctxt, key, value);
}

static void test_data_cb (http_req_t *req,
                          void *ctxt,
                          void *data,
                          unsigned len)
{
    printf("test_data_cb( %p, %p, %p, %u )\n",
            (void *)req, (void *)ctxt, (void *)data, len);
}


/* ========================================================================== */

void http_test (void)
{
    http_req_t *req;


    http_init();

    req = http_req_new();

    //http_req_set_uri(req, "http://foo.com/bar.html"); //not yet implemented
    http_req_set_components(req, "localhost", "1338", "/bar.html");

    /* add a header */
    http_req_set_header(req, "key1", "FAIL");

    /* change the only header */
    http_req_set_header(req, "key1", "FAIL2");

    /* add another header */
    http_req_set_header(req, "key2", "value2");

    /* change one header of many */
    http_req_set_header(req, "key1", "value1");

    /* add after a change */
    http_req_set_header(req, "key3", "FAIL3");
    /* change last header */
    http_req_set_header(req, "key3", "FAIL4");
    /* change header for the 2nd time */
    http_req_set_header(req, "key3", "value3");

    http_req_set_header_cb(req, (http_header_cb_t)&test_header_cb, NULL);
    http_req_set_data_cb(  req, (http_data_cb_t)  &test_data_cb,   NULL);

    http_req_send(req);

    http_req_delete(req);

    http_finalise();
}
