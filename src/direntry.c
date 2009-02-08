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


typedef void (*direntry_traverse_cb_t)(direntry_t *);


direntry_t *de_root = NULL;


TRACE_DEFINE(direntry)

static int direntry_get_internal (const char * const path,
                                  direntry_t **de_io,
                                  int with_children        );
static void post_callback (direntry_t *de);
static void direntry_walk_children (direntry_t *de, direntry_traverse_cb_t cb);
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


/* external interface ======================================================= */
int direntry_init (void)
{
    direntry_trace("direntry_init()\n");
    direntry_trace_indent();


#if FEATURE_DIRENTRY_CACHE
    /* cache init */
    direntry_cache_init();
#endif


    /* specials init */
    de_root = direntry_new();
    direntry_attribute_add(de_root, "fs2-name", "");
    direntry_attribute_add(de_root, "fs2-path", "/");
    direntry_attribute_add(de_root, "fs2-type", "directory");


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

direntry_t *direntry_new (void)
{
    direntry_t *de = (direntry_t *)calloc(1, sizeof(direntry_t));

    de->ref_count = 1;

    return de;
}

void direntry_post (direntry_t *de)
{
    de->ref_count++;

    direntry_trace("direntry_post(de->base_name==%s): refcount now %u\n",
                   de->base_name, de->ref_count);
}

direntry_t *direntry_copy (direntry_t *de)
{
    direntry_t *copy = direntry_new();


    memcpy(copy, de, sizeof(direntry_t));

    if (de->base_name) copy->base_name = strdup(de->base_name);
    if (de->path)      copy->path      = strdup(de->path);
    if (de->hash)      copy->hash      = strdup(de->hash);
    if (de->href)      copy->href      = strdup(de->href);


    return copy;
}

void direntry_delete (direntry_t *de)
{
    de->ref_count--;

    direntry_trace("direntry_delete(de->base_name==%s): refcount now %u\n",
                   de->base_name, de->ref_count);
    direntry_trace_indent();

    if (!de->ref_count)
    {
        direntry_trace("refcount == 0 => free()ing\n");

        if (de->base_name) free(de->base_name);
        if (de->path) free(de->path);
        if (de->hash) free(de->hash);
        if (de->href) free(de->href);

        free(de);
    }

    direntry_trace_dedent();
}

/* direntry_delete_tree() ?? */
void direntry_delete_with_children (direntry_t *de)
{
    direntry_t *child = de->children;
    direntry_t *next_child;

    while (child)
    {
        next_child = child->next;
        direntry_delete(child);
        child = next_child;
    }

    direntry_delete(de);
}

/* Get direntry for path.
 * Doesn't guarantee its children list is populated. */
int direntry_get (const char * const path, direntry_t **de)
{
    return direntry_get_internal(path, de, 0);
}

/* Get direntry for path.
 * Guarantee its children list is populated => if it's not then there are no
 * children.
 * Fills in de but returns -ENOTDIR if path isn't a directory */
int direntry_get_with_children (const char * const path, direntry_t **de)
{
    return direntry_get_internal(path, de, 1);
}

int populate_directory (direntry_t *de)
{
    int rc = -EIO, fetcher_rc;
    xmlParserCtxtPtr parser;
    xmlXPathObjectPtr xpathObj;
    xmlDocPtr doc;


    direntry_trace("populate_directory(%s)\n", de->path);
    direntry_trace_indent();


    /* make a new parser for this thread */
    parser = parser_new();

    /* fetch the directory listing and feed it into the parser */
    fetcher_rc = fetcher_fetch(de->path,
                               fetcher_url_type_t_BROWSE,
                               NULL,
                               (curl_write_callback)&parser_consumer,
                               (void *)parser                         );

    if (fetcher_rc == 0)
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

static int direntry_get_internal (const char * const path,
                                  direntry_t **de_io,
                                  int with_children        )
{
    int rc = 0;
    direntry_t *de = NULL;


    direntry_trace("direntry_get_internal(path==%s, with_children==%d)\n",
                   path, with_children);
    direntry_trace_indent();

    /* special cases */
    if (!strcmp(path, "/"))
    {
        direntry_trace("\"/\" is special\n");

        /* If we're caching then there should be one, cached root de, which will
         * persist and remember things like looked_for_children.
         * If we're not, copy it so that the one with looked_for_children
         * incorrectly set (these will be deleted as the cache is normally the
         * only thing to semi-permanently hold onto them) is thrown away each
         * time.
         */
#if FEATURE_DIRENTRY_CACHE
        de = de_root;
        direntry_post(de);
#else
        de = direntry_copy(de_root);
#endif
    }
    else
    {
#if FEATURE_DIRENTRY_CACHE
        /* try the cache */
        de = direntry_cache_get(path);

        /* Optimisation - if we've fetched the path's parent and enumed its
         * children and we don't have the node in the cache (we don't if we're
         * here), then it must not exist. Don't try to fetch it.
         * TODO: move this logic into the cache, which will need some way of
         * indicating whether it has something, doesn't have it (usual) or
         * doesn't have it and can assert its non-existence. */
        if (!de)
        {
            char *parent = fsfuse_dirname(path);
            direntry_t *parent_de = NULL;

            parent_de = direntry_cache_get(parent);
            free(parent);

            if (parent_de &&
                parent_de->looked_for_children)
            {
                rc = -ENOENT;
                goto bail;
            }
        }
#endif
        if (!de)
        {
            rc = fetch_node(path, &de);
        }
    }

    if (!rc && with_children)
    {
        if (de->type == direntry_type_DIRECTORY)
        {
            if (!de->looked_for_children)
            {
                rc = populate_directory(de);
            }
#if FEATURE_DIRENTRY_CACHE
            else
            {
                /* TODO: this is ugly, and probably very error-prone */
                /* if we've not had to go and look for the children then they're
                 * already in the cache. Post all their refcounts as they'll be
                 * deleted by whomever we return them to */
                direntry_walk_children(de, &post_callback);
            }
#endif
        }
        else
        {
            rc = -ENOTDIR;
        }
    }

bail:
    direntry_trace_dedent();


    *de_io = de;
    return rc;
}

static void post_callback (direntry_t *de)
{
    direntry_post(de);
}

/* Get a direntry_t for the node at path. Could be a file or directory */
static int fetch_node (const char * const path, direntry_t **de_io)
{
    int rc = -EIO, fetcher_rc;
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
    fetcher_rc = fetcher_fetch(parent,
                               fetcher_url_type_t_BROWSE,
                               NULL,
                               (curl_write_callback)&parser_consumer,
                               (void *)parser                         );

    if (fetcher_rc == 0)
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

/* enmerate the direct children of a node and call the callback on them.
 * Safe against the callback free()ing the node */
static void direntry_walk_children (direntry_t *de, direntry_traverse_cb_t cb)
{
    direntry_t *child = de->children, *next;


    while (child)
    {
        next = child->next;
        (*cb)(child);
        child = next;
    }
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

    /* attatch to parent */
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
