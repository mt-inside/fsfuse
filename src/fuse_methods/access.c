/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of access().
 */

#include "common.h"

#include <errno.h>

#include "fuse_methods.h"

#include "config_manager.h"
#include "config_reader.h"
#include "direntry.h"
#include "trace.h"


void fsfuse_access (fuse_req_t req, fuse_ino_t ino, int mask)
{
    config_reader_t *config = config_get_reader();
    int rc = 0;
    direntry_t *de;


    method_trace("fsfuse_access(ino %ld, mask %o)\n", ino, mask);
    method_trace_indent();

    rc = direntry_get_by_inode(ino, &de);

    if (!rc)
    {
        switch (direntry_get_type(de))
        {
            case listing_type_DIRECTORY:
                if (mask & ~config_attr_mode_dir(config))  rc = EACCES;
                break;
            case listing_type_FILE:
                if (mask & ~config_attr_mode_file(config)) rc = EACCES;
                break;
        }

        direntry_delete(CALLER_INFO de);
    }

    method_trace_dedent();


    assert(!fuse_reply_err(req, rc));

    config_reader_delete(config);
}

