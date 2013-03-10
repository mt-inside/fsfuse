/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * FUSE filesystem teardown function.
 */

#include "common.h"

#include <errno.h>

#include "fuse_methods.h"
#include "indexnodes.h"
#include "trace.h"


void fsfuse_destroy (void *userdata)
{
    fsfuse_ctxt_t *ctxt = (fsfuse_ctxt_t *)userdata;


    method_trace("fsfuse_destroy()\n");
    method_trace_indent();

    indexnodes_delete(ctxt->indexnodes);

    method_trace_dedent();


    /* No reply */
}
