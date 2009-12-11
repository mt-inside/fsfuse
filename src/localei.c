/*
 * locale(isation) module.
 *
 * Copyright (C) Matthew Turner 2009. All rights reserved.
 *
 * $Id: direntry.c 420 2009-12-03 19:52:58Z matt $
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
        trce("Can't set the specified locale! Check LANG, LC_CTYPE, LC_ALL.\n");
        return 1;
    }

    locale_trace("current encoding: %s\n", nl_langinfo(CODESET));

    rc = (strcmp(nl_langinfo(CODESET), "UTF-8")          &&
          strcmp(nl_langinfo(CODESET), "ANSI_X3.4-1968")    ); /* "ascii" */

    if (rc)
    {
        trce("Encoding %s unsupported!\n", nl_langinfo(CODESET));
    }


    return rc;
}

void locale_finalise (void)
{
    /* do nothing */
}
