/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * locale(isation) module.
 */

#include <locale.h>
#include <langinfo.h>
#include <string.h>

#include "common.h"
#include "localei.h"



TRACE_DEFINE(locale)


int locale_init (void)
{
    int rc;


    if (!setlocale(LC_CTYPE, ""))
    {
        trace_warn("Can't set the specified locale! Check LANG, LC_CTYPE, LC_ALL.\n");
        return 1;
    }

    locale_trace("current encoding: %s\n", nl_langinfo(CODESET));

    rc = (strcmp(nl_langinfo(CODESET), "UTF-8")          &&
          strcmp(nl_langinfo(CODESET), "ANSI_X3.4-1968")    ); /* "ascii" */

    if (rc)
    {
        trace_error("Encoding %s unsupported!\n", nl_langinfo(CODESET));
    }


    return rc;
}

void locale_finalise (void)
{
    /* do nothing */
}
