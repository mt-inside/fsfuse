/*
 * This file deals with "direntrys" - "entries in a directory". This is
 * basically our internal file system tree implemetation.
 *
 * Copyright (C) Matthew Turner 2008-2009. All rights reserved.
 *
 * $Id$
 */

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "common.h"
#include "config.h"
#include "direntry.h"
#include "direntry_internal.h"
#if FEATURE_DIRENTRY_CACHE
#include "direntry_cache.h"
#endif
#include "fetcher.h"
#include "parser.h"
#include "indexnode.h"


TRACE_DEFINE(direntry)

static direntry_t *direntry_new_root (void);
static int fetch_node (const char * const path, direntry_t **de_io);
static direntry_type_t direntry_type_from_string (const char * const s);
static int direntry_children_fetch (const char * const path, direntry_t **de_out);
static int filelist_entries_parse (xmlNodeSetPtr nodes,
                                   const char * const parent_path,
                                   direntry_t **de_out             );
static int filelist_entry_parse (xmlElementPtr node,
                                  direntry_t *de,
                                  const char * const parent );
static void direntry_attribute_add (direntry_t * const de,
                                    const char *name,
                                    const char *value      );
static char *xpath_attr_escape (char *s);


/* direntry module external interface ======================================= */
int direntry_init (void)
{
    direntry_trace("direntry_init()\n");
    direntry_trace_indent();


#if FEATURE_DIRENTRY_CACHE
    {
        direntry_t *de_root;


        direntry_cache_init();

        /* If we're caching, we have one root node, made here, which is posted when
         * it's needed. If we're not, we create them when needed */
        de_root = direntry_new_root();
        direntry_cache_add(de_root);
        direntry_delete(CALLER_INFO de_root);
    }
#endif

    direntry_trace_dedent();


    return 0;
}

void direntry_finalise (void)
{
#if FEATURE_DIRENTRY_CACHE
    direntry_cache_finalise();
#endif
}

/* direntry functions ======================================================= */
direntry_t *direntry_new (CALLER_DECL_ONLY)
{
    direntry_t *de = (direntry_t *)calloc(1, sizeof(direntry_t));


    de->lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(de->lock, NULL);

    de->ref_count = 1;

    direntry_trace("[direntry %p] new (" CALLER_FORMAT ") ref %u\n",
                   de, CALLER_PASS de->ref_count);

    return de;
}

static direntry_t *direntry_new_root (void)
{
    direntry_t *root = direntry_new(CALLER_INFO_ONLY);


    direntry_attribute_add(root, "fs2-name", "");
    direntry_attribute_add(root, "fs2-path", "/");
    direntry_attribute_add(root, "fs2-type", "directory");
    /* Set the link count to something non-0. We can't query this from the
     * indexnode. We could work it out every time, but that would be tedious.
     * It turns out to be vitally important that this is non-0 if samba is going
     * to share the filesystem */
    direntry_attribute_add(root, "fs2-linkcount", "1");


    return root;
}

void direntry_post (CALLER_DECL direntry_t *de)
{
    assert(de->ref_count);

    de->ref_count++;

    direntry_trace("[direntry %p] post (" CALLER_FORMAT ") ref %u\n",
                   de, CALLER_PASS de->ref_count);
}

void direntry_delete (CALLER_DECL direntry_t *de)
{
    unsigned refc;


    /* hacky attempt to detect overflow */
    assert((signed)de->ref_count > 0);

    pthread_mutex_lock(de->lock);
    refc = --de->ref_count;

    direntry_trace("[direntry %p] delete (" CALLER_FORMAT ") ref %u\n",
                   de, CALLER_PASS de->ref_count);
    direntry_trace_indent();

    if (!refc)
    {
        direntry_trace("refcount == 0 => free()ing\n");

#if FEATURE_DIRENTRY_CACHE
        assert(!direntry_cache_get(direntry_get_path(de)));
#endif

        pthread_mutex_unlock(de->lock);
        pthread_mutex_destroy(de->lock);

#if !DEBUG

        free(de->lock);

        if (de->base_name) free(de->base_name);
        if (de->path) free(de->path);
        if (de->hash) free(de->hash);
        if (de->href) free(de->href);

        free(de);
#endif
    }
    else
    {
        pthread_mutex_unlock(de->lock);
    }

    direntry_trace_dedent();
}

void direntry_delete_list (direntry_t *de)
{
    direntry_t *next;

    while (de)
    {
        next = direntry_get_next_sibling(de);
        direntry_delete(CALLER_INFO de);
        de = next;
    }
}

/* direntry tree traversal functions =========================================*/
/* for now, we only provide these simple wrappers around the underlying
 * implementation details. Iterator functions could be built from these */

direntry_t *direntry_get_parent       (direntry_t *de)
{
    return de->parent;
}

direntry_t *direntry_get_first_child  (direntry_t *de)
{
    return de->children;
}

direntry_t *direntry_get_next_sibling (direntry_t *de)
{
    return de->next;
}

/* direntry attribute getters =============================================== */
char *direntry_get_path (direntry_t *de)
{
    return de->path;
}

char *direntry_get_base_name (direntry_t *de)
{
    return de->base_name;
}

char *direntry_get_hash (direntry_t *de)
{
    return de->hash;
}

direntry_type_t direntry_get_type (direntry_t *de)
{
    return de->type;
}

off_t direntry_get_size (direntry_t *de)
{
    return de->size;
}

unsigned long direntry_get_link_count (direntry_t *de)
{
    return de->link_count;
}

char *direntry_get_href (direntry_t *de)
{
    return de->href;
}

int direntry_got_children (direntry_t *de)
{
    return de->looked_for_children;
}

int direntry_is_root (direntry_t *de)
{
    return !strcmp(direntry_get_path(de), "/");
}


/* Get direntry for path. I.e. we want the meta-data for the node at this path,
 * be it a file or directory. */
int direntry_get (const char * const path, direntry_t **de)
{
    int rc = 0;


    *de = NULL;


#if FEATURE_DIRENTRY_CACHE
    *de = direntry_cache_get(path);
#else
    /* special cases */
    if (!strcmp(path, "/"))
    {
        direntry_trace("\"/\" is special\n");

        *de = direntry_new_root(CALLER_INFO);
    }
#endif

    if (!*de)
    {
        rc = fetch_node(path, de);
    }


    return rc;
}


int direntry_get_children (direntry_t *de, direntry_t **de_out)
{
    int rc;


    direntry_trace("direntry_get_children(%s)\n", de->path);
    direntry_trace_indent();


#if FEATURE_DIRENTRY_CACHE
    /* If the direntry is marked as having had its children enumerated then:
     * - they must have been enumerated, QED
     * => they must be in the cache
     * => they must be attached as children of de, which also must be in the
     * cache because it exists.
     * So, we can just return them.
     */
    if (direntry_got_children(de))
    {
        direntry_t *child = direntry_get_first_child(de);
        while (child)
        {
            direntry_post(CALLER_INFO child);
            child = direntry_get_next_sibling(child);
        }

        *de_out = direntry_get_first_child(de);
        rc = 0;
        goto ret;
    }
#endif


    rc = direntry_children_fetch(de->path, de_out);
    if (!rc)
    {
        de->looked_for_children = 1;
    }


ret:
    direntry_trace_dedent();


    return rc;
}


int direntry_de2stat (struct stat *st, direntry_t *de)
{
    int uid = config_attr_id_uid,
        gid = config_attr_id_gid;


    memset((void *)st, 0, sizeof(struct stat));

    st->st_uid = (uid == -1) ? getuid() : (unsigned)uid;
    st->st_gid = (gid == -1) ? getgid() : (unsigned)gid;

    switch (de->type)
    {
        /* Regular file */
        case direntry_type_FILE:
            st->st_mode = S_IFREG | config_attr_mode_file;

            st->st_size = de->size;
            st->st_blksize = FSFUSE_BLKSIZE;
            st->st_blocks = (de->size / 512) + 1;

            /* indexnode doesn't supply link counts for files */
            st->st_nlink = 1;

            break;
        case direntry_type_DIRECTORY:
            st->st_mode = S_IFDIR | config_attr_mode_dir;

            /* indexnode supplies directory's tree size - not what a unix fs
             * wants */
            st->st_size = 0;

            st->st_nlink = de->link_count;

            break;
    }


    return 0;
}

/* There are versions of these functions glibc, but they're not thread-safe */
char *fsfuse_dirname (const char *path)
{
    char *loc = strrchr(path, '/');
    unsigned dir_len;
    char *dir = NULL;


    if (!loc) return strdup("");

    dir_len = loc - path;
    if (!dir_len) return strdup("/");

    dir = (char *)malloc((dir_len + 1) * sizeof(char));
    if (dir)
    {
        strncpy(dir, path, dir_len);
        dir[dir_len] = '\0';
    }


    return dir;
}

char *fsfuse_basename (const char *path)
{
    char *loc = strrchr(path, '/');
    unsigned base_len;
    char *base = NULL;


    if (!loc) return strdup(path);

    base_len = strlen(path) - (loc - path) + 1;
    if (!base_len) return strdup("");

    base = (char *)malloc((base_len + 1) * sizeof(char));
    if (base)
    {
        strncpy(base, loc + 1, base_len);
        base[base_len] = '\0';
    }


    return base;
}


/* static helpers =========================================================== */

/* Get a direntry_t for the node at path. Could be a file or directory */
static int fetch_node (const char * const path, direntry_t **de_io)
{
    direntry_t *de_tmp, *de_parent = NULL;
    const char *parent_path = path;
    int rc = -ENOENT;


#if FEATURE_DIRENTRY_CACHE
    /* this is an internal function - node cannot be in the cache */

    parent_path = fsfuse_dirname(parent_path);
    de_parent = direntry_cache_get(parent_path);
    if (!de_parent)
    {
        fetch_node(parent_path, &de_parent);
    }
    if (!de_parent) return rc;
    free((char *)parent_path);

    /* We have to call the external function here, as it won't double-fetch if
     * the children are already cached, and will set the looked_for_children
     * flag on de_parent if it has to go and get them */
    direntry_get_children(de_parent, &de_tmp);
    direntry_delete_list(de_tmp);

    *de_io = direntry_cache_get(path);

    direntry_delete(CALLER_INFO de_parent);


    if (*de_io)
    {
        rc = 0;
    }

#else

    /* We call the fetcher direct here, so that we don't have to go and acquire a
     * de for the parent, which is pointless because:
     * a) it won't have anything cached (cache is off)
     * b) we'll set a flag on it, but it's gonna be thrown away instantly
     */
    parent_path = fsfuse_dirname(path);
    rc = direntry_children_fetch(parent_path, &de_tmp);
    free(parent_path);

    if (!rc)
    {
        rc = -ENOENT;
        while (de_tmp)
        {
            if (!strcmp(direntry_get_path(de_tmp), path))
            {
                *de_io = de_tmp;
                rc = 0;
                break;
            }
            de_tmp = direntry_get_next_sibling(de_tmp);
        }
    }

#endif


    return rc;
}

static int direntry_children_fetch (const char * const path, direntry_t **de_out)
{
    xmlParserCtxtPtr parser;
    xmlXPathObjectPtr xpathObj;
    xmlDocPtr doc;
    int rc;


    direntry_trace("direntry_children_fetch(%s)\n", path);

    /* make a new parser for this thread */
    parser = parser_new();

    /* fetch the directory listing and feed it into the parser */
    rc = fetcher_fetch(path,
                       fetcher_url_type_t_BROWSE,
                       NULL,
                       (curl_write_callback)&parser_consumer,
                       (void *)parser                         );

#if FEATURE_DIRENTRY_CACHE
    /* if we're caching, then path must have a de in the cache for us to be
     * interested in its children. If it's disappeared we won't be able to list
     * it and therefore must find its de and mark it stale */
    if (rc == -ENOENT)
    {
        direntry_t *de = direntry_cache_get(path);
        direntry_cache_notify_stale(de);
        direntry_delete(CALLER_INFO de);
    }
#endif

    if (rc == 0)
    {
        /* The fetcher has returned, so that's all the document.
         * Indicate to the parser that that's it */
        doc = parser_done(parser);

        xpathObj = parser_xhtml_xpath(doc, "//xhtml:div[@id='fs2-filelist']/xhtml:a[@fs2-name]");
        if (xpathObj->type == XPATH_NODESET)
        {
            rc = filelist_entries_parse(xpathObj->nodesetval, path, de_out);
        }

        xmlXPathFreeObject(xpathObj);
        xmlFreeDoc(doc);
    }

    parser_delete(parser);


    return rc;
}

static direntry_type_t direntry_type_from_string (const char * const s)
{
    if (!strcmp(s, "file"))
    {
        return direntry_type_FILE;
    }
    else if (!strcmp(s, "directory"))
    {
        return direntry_type_DIRECTORY;
    }
    else
    {
        errx(1, "unknown fs2 type");
    }
}

/* Parse a nodeset representing the A tags in an fs2-filelist,
 * building direntries */
static int filelist_entries_parse (xmlNodeSetPtr nodes,
                                   const char * const parent_path,
                                   direntry_t **de_out             )
{
    direntry_t *de, *prev;
    int rc = 0, size, i;


    size = (nodes) ? nodes->nodeNr : 0;

    direntry_trace("filelist_entries_parse(): enumerating %d nodes\n", size);
    direntry_trace_indent();

    /* Enumerate the A elements */
    for (i = 0, prev = NULL; i < size && !rc; i++)
    {
        if (!(de = direntry_new(CALLER_INFO_ONLY)))
        {
            rc = -EIO;
            break;
        }

        rc = filelist_entry_parse((xmlElementPtr)nodes->nodeTab[i],
                                  de,
                                  parent_path);
        de->next = prev;
        prev = de;
    }
    *de_out = prev;

    direntry_trace_dedent();


    return rc;
}

static int filelist_entry_parse (xmlElementPtr node,
                                  direntry_t *de,
                                  const char * const parent )
{
    xmlAttributePtr curAttr = NULL;


    assert(node);
    assert(de);

    direntry_trace("filelist_entry_parse(element content==%s)\n", node->children->content);
    direntry_trace_indent();

    if (node->type != XML_ELEMENT_NODE ||
        strcmp((char *)node->name, "a"))
    {
        return -EIO;
    }

    /* Enumerate the element's attributes */
    curAttr = (xmlAttributePtr)node->attributes;
    while (curAttr)
    {
        if (curAttr->type == XML_ATTRIBUTE_NODE &&
            curAttr->children &&
            curAttr->children->type == XML_TEXT_NODE &&
            !curAttr->children->next)
        {
            /* ignore what the indexnode says the path is for now */
            if (strcmp((char *)curAttr->name, "fs2-path"))
            {
                direntry_trace("Attribute %s == %s\n", curAttr->name, curAttr->children->content);
                direntry_attribute_add(de, (char *)curAttr->name, (char *)curAttr->children->content);
            }
        }

        curAttr = (xmlAttributePtr)curAttr->next;
    }

    /* Currently, the indexnode gives us paths for directories, but not
     * files. In addition, the paths is offers up for directories are
     * bollocks, so we ignore them and make our own paths for both */
    if (!de->path)
    {
        char *path = (char *)malloc(strlen(de->base_name) + strlen(parent) + 2);

        direntry_trace("constructing path: parent==%s, /, de->base_name==%s\n", parent, de->base_name);
        strcpy(path, parent);
        if (strcmp(parent, "/")) strcat(path, "/"); /* special case: don't append a trailing "/" onto the parent path if it's the root ("/"), because the root is essentially a directory with an empty name, and we store paths normalised. */
        strcat(path, de->base_name);

        direntry_attribute_add(de, "fs2-path", path);

        free(path);
    }

#if FEATURE_DIRENTRY_CACHE
    direntry_cache_add(de);
#endif

    direntry_trace_dedent();


    return 0;
}

static void direntry_attribute_add (direntry_t * const de,
                                    const char *name,
                                    const char *value      )
{
    direntry_trace_indent();

    if (!strcmp(name, "fs2-name"))
    {
        de->base_name = strdup(value);
        parser_trace("adding attribute base_name==%s\n", de->base_name);
    }
    else if (!strcmp(name, "fs2-path"))
    {
        de->path = strdup(value);
        parser_trace("adding attribute path==%s\n", de->path);
    }
    else if (!strcmp(name, "fs2-hash"))
    {
        de->hash = strdup(value);
        parser_trace("adding attribute hash==%s\n", de->hash);
    }
    else if (!strcmp(name, "fs2-type"))
    {
        de->type = direntry_type_from_string(value);
        parser_trace("adding attribute type==%s\n", value);
    }
    else if (!strcmp(name, "fs2-size"))
    {
        de->size = atoll(value);
        parser_trace("adding attribute size==%llu\n", de->size);
    }
    else if (!strcmp(name, "fs2-linkcount"))
    {
        de->link_count = atol(value);
        parser_trace("adding attribute link_count==%lu\n", de->link_count);
    }
    else if (!strcmp(name, "href"))
    {
        de->href = strdup(value);
        parser_trace("adding attribute href==%s\n", de->href);
    }
    else
    {
        direntry_trace("Unknown attribute %s == %s\n", name, value);
    }

    direntry_trace_dedent();
}

static char *xpath_attr_escape (char *s)
{
    char *c, *n = (char *)malloc(PATH_MAX * strlen("&quot;") * sizeof(char)), *np = n;


    while ((c = strchr(s, '"')))
    {
        strncpy(np, s, c - s);
        np += (c - s);
        strcpy(np, "&quot;");
        np += strlen("&quot;");
        s = c + 1;
    }
    strcpy(np, s);


    return n;
}
