/*
 * Uniform Resource Identifier Class
 *
 * Copyright (C) Matthew Turner 2008-2012. All rights reserved.
 *
 * $Id: common.h 586 2012-03-26 18:14:00Z matt $
 */

#ifndef _INCLUDED_URI_H
#define _INCLUDED_URI_H

typedef struct _uri_t uri_t;


/* See RFC2396.
 * We assume a "generic uri" with a "server-based naming authority".
 * e.g.:
 *
 * scheme://userinfo@host:port/path?query#fragment
 *          \-- authority  --/
 */
extern uri_t *uri_new (void);
extern uri_t *uri_from_string (const char *str);
extern void uri_delete (uri_t *uri);

extern void uri_set_scheme   (uri_t *uri, const char *scheme  );
extern void uri_set_userinfo (uri_t *uri, const char *userinfo);
extern void uri_set_host     (uri_t *uri, const char *host    );
extern void uri_set_port     (uri_t *uri, const char *port    );
extern void uri_set_path     (uri_t *uri, const char *path    );
extern void uri_set_query    (uri_t *uri, const char *query   );
extern void uri_set_fragment (uri_t *uri, const char *fragment);

extern char *uri_get           (uri_t *uri);
extern char *uri_get_scheme    (uri_t *uri);
extern char *uri_get_authority (uri_t *uri);
extern char *uri_get_userinfo  (uri_t *uri);
extern char *uri_get_host      (uri_t *uri);
extern char *uri_get_port      (uri_t *uri);
extern char *uri_get_path      (uri_t *uri);
extern char *uri_get_query     (uri_t *uri);
extern char *uri_get_fragment  (uri_t *uri);

#endif /* _INCLUDED_URI_H */
