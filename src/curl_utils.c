/*
 * Copyright (C) 2008-2012 Matthew Turner.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 */

#include "curl_utils.h"


CURL *curl_eh_new (void)
{
    CURL *eh = curl_easy_init();


    curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1);


    return eh;
}

void curl_eh_delete (CURL *eh)
{
    curl_easy_cleanup(eh);
}
