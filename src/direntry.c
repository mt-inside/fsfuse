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
#if FEATURE_DIRENTRY_CACHE
#include "direntry_cache.h"
#endif
#include "fetcher.h"
#include "parser.h"
#include "indexnode.h"


/* This is a little horrible, but isn't too bad. I can't think of a way to make
 * handling of "/" non-horrible.
 * If we're caching then there is one "cached" de for "/", kept in this global
 * var, not the cache, so it's can't be deleted. It always has to exist because
 * we can't fetch it. It's an open question as to whether that should be dealt
 * with here (as it is currently), or in the fetcher. This single root de
 * persists and remembers things like any child lists attached to it.
 * If we're not caching, we just make a new one each time, which will be thrown
 * away when done with. TODO: currently, this also serves to take with it any
 * out-of-date information about children, as these data structures aren't
 * currently kept consistent.
 */
#if FEATURE_DIRENTRY_CACHE
direntry_t *de_root = NULL;
#endif


TRACE_DEFINE(direntry)

static direntry_t *direntry_new_root (void);
static direntry_type_t direntry_type_from_string (const char * const s);
static int filelist_entries_parse (xmlNodeSetPtr nodes, direntry_t *parent);
static int filelist_entry_parse (xmlElementPtr node,
                                 direntry_t *de,
                                 const char * const parent,
                                 direntry_t *parent_de      );
static void direntry_attribute_add (direntry_t * const de,
                                    const char *name,
                                    const char *value      );
static char *xpath_attr_escape (char *s);
static int fetch_node (const char * const path, direntry_t **de_io);


/* direntry module external interface ======================================= */
int direntry_init (void)
{
    direntry_trace("direntry_init()\n");
    direntry_trace_indent();


#if FEATURE_DIRENTRY_CACHE
    direntry_cache_init();

    /* If we're caching, we have one root node, made here, which is posted when
     * it's needed. If we're not, we create them when needed */
    de_root = direntry_new_root();
#endif

    direntry_trace_dedent();


    return 0;
}

void direntry_finalise (void)
{
#if FEATURE_DIRENTRY_CACHE
    direntry_cache_finalise();
#endif

    direntry_delete(de_root);
}

/* direntry functions ======================================================= */
direntry_t *direntry_new (void)
{
    direntry_t *de = (direntry_t *)calloc(1, sizeof(direntry_t));


    de->lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(de->lock, NULL);

    de->ref_count = 1;

    return de;
}

static direntry_t *direntry_new_root (void)
{
    direntry_t *root = direntry_new();


    direntry_attribute_add(root, "fs2-name", "");
    direntry_attribute_add(root, "fs2-path", "/");
    direntry_attribute_add(root, "fs2-type", "directory");


    return root;
}

void direntry_post (direntry_t *de)
{
    de->ref_count++;

    direntry_trace("direntry_post(de->base_name==%s): refcount now %u\n",
                   de->base_name, de->ref_count);
}

void direntry_delete (direntry_t *de)
{
    unsigned refc;


    pthread_mutex_lock(de->lock);
    refc = --de->ref_count;

    direntry_trace("direntry_delete(de->base_name==%s): refcount now %u\n",
                   de->base_name, de->ref_count);
    direntry_trace_indent();

    if (!refc)
    {
        direntry_trace("refcount == 0 => free()ing\n");

        pthread_mutex_unlock(de->lock);
        pthread_mutex_destroy(de->lock);
        free(de->lock);

        if (de->base_name) free(de->base_name);
        if (de->path) free(de->path);
        if (de->hash) free(de->hash);
        if (de->href) free(de->href);

        free(de);
    }
    else
    {
        pthread_mutex_unlock(de->lock);
    }

    direntry_trace_dedent();
}

void direntry_delete_with_children (direntry_t *de)
{
    direntry_t *child = direntry_get_first_child(de), *next_child;

    while (child)
    {
        next_child = direntry_get_next_sibling(child);
        direntry_delete(child);
        child = next_child;
    }

    direntry_delete(de);
}

/* direntry tree traversal functions =========================================*/
/* for now, we only provide these simple wrappers around the underlying
 * implementation details. Iterator functions could be built from these */

direntry_t *direntry_get_first_child (direntry_t *de)
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


/* Get direntry for path. I.e. we want the meta-data for the node at this path,
 * be it a file or directory. */
int direntry_get (const char * const path, direntry_t **de)
{
    int rc = 0;


    /* special cases */
    if (!strcmp(path, "/"))
    {
        direntry_trace("\"/\" is special\n");

#if FEATURE_DIRENTRY_CACHE
        *de = de_root;
        direntry_post(*de);
#else
        *de = direntry_new_root();
#endif
    }
    else
    {

#if FEATURE_DIRENTRY_CACHE
        *de = direntry_cache_get(path);
#else
        *de = NULL;
#endif

        if (!*de)
        {
            rc = fetch_node(path, de);
        }
    }


    return rc;
}

int direntry_get_with_children (const char * const path, direntry_t **de)
{
    int rc = 0;
    direntry_t *child;


    /* special cases */
    if (!strcmp(path, "/"))
    {
        direntry_trace("\"/\" is special\n");

#if FEATURE_DIRENTRY_CACHE
        *de = de_root;
        direntry_post(*de);

        if (*de && direntry_got_children(*de))
        {
            child = direntry_get_first_child(*de);
            while (child)
            {
                direntry_post(child);
                child = direntry_get_next_sibling(child);
            }
        }
#else
        *de = direntry_new_root();
#endif
    }
    else
    {
#if FEATURE_DIRENTRY_CACHE
        *de = direntry_cache_get(path);

        if (*de && direntry_got_children(*de))
        {
            child = direntry_get_first_child(*de);
            while (child)
            {
                direntry_post(child);
                child = direntry_get_next_sibling(child);
            }
        }
#else
        NOT_USED(child);
        *de = NULL;
#endif

        if (!*de)
        {
            rc = fetch_node(path, de);
        }
    }


    assert(direntry_get_type(*de) == direntry_type_DIRECTORY);

    if (!rc && !direntry_got_children(*de))
    {
        rc = populate_directory(*de);

        if (rc)
        {
            direntry_delete(*de);
        }
    }


    return rc;
}


int populate_directory (direntry_t *de)
{
    int rc;
    xmlParserCtxtPtr parser;
    xmlXPathObjectPtr xpathObj;
    xmlDocPtr doc;


    direntry_trace("populate_directory(%s)\n", de->path);
    direntry_trace_indent();


    /* make a new parser for this thread */
    parser = parser_new();

    /* fetch the directory listing and feed it into the parser */
    rc = fetcher_fetch(de->path,
                       fetcher_url_type_t_BROWSE,
                       NULL,
                       (curl_write_callback)&parser_consumer,
                       (void *)parser                         );

    if (rc == 0)
    {
        /* The fetcher has returned, so that's all the document.
         * Indicate to the parser that that's it */
        doc = parser_done(parser);

        xpathObj = parser_xhtml_xpath(doc, "//xhtml:div[@id='fs2-filelist']/xhtml:a[@fs2-name]");
        if (xpathObj->type == XPATH_NODESET)
        {
            rc = filelist_entries_parse(xpathObj->nodesetval, de);
            de->looked_for_children = 1;
        }

        xmlXPathFreeObject(xpathObj);
        xmlFreeDoc(doc);
    }

    parser_delete(parser);

    direntry_trace_dedent();


    return rc;
}

int direntry_de2stat (struct stat *st, direntry_t *de)
{
    int uid = config_get(config_key_FS_UID).int_val,
        gid = config_get(config_key_FS_GID).int_val;


    memset((void *)st, 0, sizeof(struct stat));

    st->st_uid = (uid == -1) ? getuid() : (unsigned)uid;
    st->st_gid = (gid == -1) ? getgid() : (unsigned)gid;

    switch (de->type)
    {
        /* Regular file */
        case direntry_type_FILE:
            st->st_mode = S_IFREG | config_get(config_key_FILE_MODE).int_val;

            st->st_size = de->size;
            st->st_blksize = FSFUSE_BLKSIZE;
            st->st_blocks = (de->size / 512) + 1;

            /* indexnode doesn't supply link counts for files */
            st->st_nlink = 1;

            break;
        case direntry_type_DIRECTORY:
            st->st_mode = S_IFDIR | config_get(config_key_DIR_MODE).int_val;

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
        direntry_trace("fsfuse_dirname(path==%s) = %s (len %u)\n",
                path, dir, dir_len);
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
        direntry_trace("fsfuse_basename(path==%s) = %s (len %u)\n",
                path, base, base_len);
    }


    return base;
}


/* static helpers =========================================================== */

/* Get a direntry_t for the node at path. Could be a file or directory */
static int fetch_node (const char * const path, direntry_t **de_io)
{
    int rc;
    char *parent;
    direntry_t *de = NULL;
    xmlParserCtxtPtr parser;
    xmlXPathObjectPtr xpathObj;
    xmlDocPtr doc;


    direntry_trace("fetch_node(%s)\n", path);
    direntry_trace_indent();

    parent = fsfuse_dirname(path);
    parser = parser_new();

    /* fetch the parent's directory listing and feed it into the parser */
    rc = fetcher_fetch(parent,
                       fetcher_url_type_t_BROWSE,
                       NULL,
                       (curl_write_callback)&parser_consumer,
                       (void *)parser                         );

    if (rc == 0)
    {
        char *basename;
        char *xpath = (char *)malloc((PATH_MAX * 6 + 64) * sizeof(*xpath));
        /* The fetcher has returned, so that's all the document.
         * Indicate to the parser that that's it */
        doc = parser_done(parser);

        if (doc)
        {
            basename = fsfuse_basename(path);
            /* FIXME can't deal with names that contain both '"' and '''!!
             * The internet literally seems to have no idea how to get around
             * this, save for some funny trick with xpath variables */
            /* TODO: security */
            if (strchr(basename, '"'))
            {
                sprintf(xpath, "//xhtml:div[@id='fs2-filelist']/xhtml:a[@fs2-name='%s']", basename);
            }
            else
            {
                sprintf(xpath, "//xhtml:div[@id='fs2-filelist']/xhtml:a[@fs2-name=\"%s\"]", basename);
            }
            free(basename);

            xpathObj = parser_xhtml_xpath(doc, xpath);
            if (xpathObj &&
                xpathObj->type == XPATH_NODESET &&
                xpathObj->nodesetval->nodeNr <= 1)
            {
                if (xpathObj->nodesetval->nodeNr)
                {
                    de = direntry_new();
                    filelist_entry_parse((xmlElementPtr)xpathObj->nodesetval->nodeTab[0], de, parent, NULL);
                    rc = 0;
                }
                else
                {
                    rc = -ENOENT;
                }
            }

            xmlXPathFreeObject(xpathObj);
            xmlFreeDoc(doc);
        }

        free(xpath);
    }

    parser_delete(parser);
    free(parent);

    direntry_trace_dedent();


    *de_io = de;
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
static int filelist_entries_parse (xmlNodeSetPtr nodes, direntry_t *parent)
{
    direntry_t *de;
    int rc = 0, size, i;


    size = (nodes) ? nodes->nodeNr : 0;

    direntry_trace("filelist_entries_parse(): enumerating %d nodes\n", size);
    direntry_trace_indent();

    /* Enumerate the A elements */
    for (i = 0; i < size && !rc; i++)
    {
        if (!(de = direntry_new()))
        {
            rc = -EIO;
            break;
        }

        rc = filelist_entry_parse((xmlElementPtr)nodes->nodeTab[i],
                                  de,
                                  parent->path,
                                  parent);
    }

    direntry_trace_dedent();


    return rc;
}

static int filelist_entry_parse (xmlElementPtr node,
                                  direntry_t *de,
                                  const char * const parent,
                                  direntry_t *parent_de      )
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

    /* attach to parent */
    if (parent_de)
    {
        de->next = parent_de->children;
        parent_de->children = de;
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
