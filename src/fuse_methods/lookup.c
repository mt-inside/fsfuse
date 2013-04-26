/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * Implementation of lookup().
 */

#include "common.h"

#include <errno.h>

#include "direntry.h"
#include "fuse_methods.h"
#include "trace.h"


/* TODO: is it a requirement for lookup to fill in the struct stat in
 * fuse_entry? This seems to duplicate the work in access(), open(), getattr()
 * etc. It would be neater if lookup were just for generating inode numbers.
 */
void fsfuse_lookup (fuse_req_t req,
                    fuse_ino_t parent,
                    const char *name)
{
    direntry_t *de;
    struct fuse_entry_param entry;
    int rc;


    method_trace("fsfuse_lookup(parent %lu, name %s)\n",
         parent, name);
    method_trace_indent();


    rc = direntry_get_child_by_name(parent, name, &de);

    if (!rc)
    {
        direntry_de2fuse_entry(de, &entry);
        entry.attr_timeout = 1.0;
        entry.entry_timeout = 1.0;
    }

    method_trace_dedent();


    /* Don't delete the de, as the success branch usurps the copy to be owned by
     * the looked-up file, and the error branch doesn't have a de to delete */
    if (!rc)
    {
        assert(!fuse_reply_entry(req, &entry));
    }
    else
    {
        assert(!fuse_reply_err(req, rc));
    }
}
