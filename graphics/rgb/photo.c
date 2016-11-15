//      © Christian Sommerfeldt Øien
//      All rights reserved

#include "photo.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <strings.h>

    static bool //note: exist in image.c as well
path_is_jpeg(const char * path)
{
    size_t n = strlen(path);
    if (n >= 4 && 0 == strcasecmp(path + n - 4, ".jpg")) return true;
    if (n >= 5 && 0 == strcasecmp(path + n - 5, ".jpeg")) return true;
    return false;
}

#define hash_len 16
#define n_entries 16

typedef struct {
    char path_hash[hash_len];
    photo * photo;
} cache_entry;

static cache_entry cache_entries[16];

    static char *
path_hash(const char * path)
{
    static char hash[hash_len]; //todo: rewrite to not return static buffer
    size_t n = strlen(path);
    if (n <= hash_len) {
        memcpy(hash, path, n);
        memset(hash + n, 0, hash_len - n);
    } else {
        memcpy(hash, path + (n - hash_len), hash_len);
    }
    return hash;
}

    static cache_entry *
alloc_entry(const char * path)
{
    char * hash = path ? path_hash(path) : NULL;
    for (int i = 0; i < n_entries; i++)
        if (cache_entries[i].path_hash[0] == '\0'
                || (hash && memcmp(hash, cache_entries[i].path_hash, hash_len) == 0))
            return &cache_entries[i];
    return NULL;
}

    static void
free_entry(const photo * ph)
{
    for (int i = 0; i < n_entries; i++)
        if (cache_entries[i].photo == ph) {
            cache_entries[i].path_hash[0] = '\0';
            break;
        }
}

    static void
insert_entry(const char * path, photo * ph)
{
    cache_entry * e = alloc_entry(NULL);
    if (e) {
        assert(e->path_hash[0] == '\0');
        memcpy(e->path_hash, path_hash(path), hash_len);
        e->photo = ph;
    } else {
        fprintf(stderr, "photo-cache full\n");
    }
    ph->ref_count_ = 1;
}

    static photo *
cached_photo(const char * path)
{
    cache_entry * e = alloc_entry(path);
    if ( ! e || e->path_hash[0] == '\0')
        return NULL;
    else
        return e->photo;
}

    photo *
photo_create(const char * path)
{
    extern FILE * popen(const char *, const char *);
    extern int pclose(FILE *);
    photo * p = cached_photo(path);
    if (p) {
        p->ref_count_++;
        return p;
    }
    int (*_close)(FILE *) = fclose;
    FILE * file;
    char cmd[256];
    const char * name = path;
    if (path_is_jpeg(path)) {
        name = cmd;
        snprintf(cmd, sizeof cmd, "jpegtopnm %s", path);
        _close = pclose;
        file = popen(cmd, "r");
    } else
        file = fopen(path, "rb");
    if (file == NULL) {
        perror(path);
        return NULL;
    }
    photo * ph = NULL;
    char pnm_type;
    int width, height, depth;
    const int n = fscanf(file, "P%c %d %d %d\n",
            &pnm_type, &width, &height, &depth);
    if (n != 4 || !isdigit(pnm_type)) {
        fprintf(stderr, "'%s' is not PNM\n", name);
        goto e;
    }
    if (pnm_type != '5' && pnm_type != '6') {
        fprintf(stderr, "'%s' is not P5 or P6\n", name);
        goto e;
    }
    if (depth != 255) {
        fprintf(stderr, "'%s' depth is not 255\n", name);
        goto e;
    }
    const int tup = (pnm_type == '5') ? 1 : 3;
    ph = malloc((sizeof *ph) + tup * width * height);
    ph->width = width;
    ph->height = height;
    ph->grey = (tup == 1);
    const int r = fread(ph->data, tup, width * height, file);
    if (r != width * height) {
        fprintf(stderr, "'%s' gave %s after %u of %d values. "
                "filling with last value till expected size\n", name,
                feof(file) ? "EOF" : "error", r, width * height);
        const int s = width * height;
        const char * const q = ph->data + (r - 1) * tup;
        for (int i=r; i<s; i++) memcpy(ph->data + i * tup, q, tup);
    }
    insert_entry(path, ph);
e:  _close(file);
    return ph;
}

    void
photo_delete(photo * ph)
{
    if ( ! ph) return;
    const int count = -- ph->ref_count_;
    if (count == 0) {
        free_entry(ph);
        free(ph);
    } else if (count < 0)
        fprintf(stderr, "ref-count underflow\n");
}

    static unsigned char *
photo_pixel(const photo * ph, int x, int y)
{
    if (x < 0 || y < 0 || x >= ph->width || y >= ph->height) return NULL;
    else return (unsigned char *)
            & ph->data[(ph->grey ? 1 : 3) * (ph->width * y + x)];
}

    unsigned
photo_rgb(const photo * ph, int x, int y)
{
    unsigned char * v = photo_pixel(ph, x, y);
    if (v == NULL) return 0;
    else if (ph->grey) {
        unsigned w = *v;
        return w | w << 8 | w << 16;
    } else
        return v[0] | v[1] << 8 | v[2] << 16;
}

    compact_color
photo_color(const photo * ph, int x, int y)
{
    unsigned char * v = photo_pixel(ph, x, y);
    if (v == NULL)
        return (compact_color){0, 0, 0};
    else if (ph->grey)
        return (compact_color){*v, *v, *v};
    else
        return str_color(v);
}
