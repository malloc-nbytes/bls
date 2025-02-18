#ifndef LISTING_H
#define LISTING_H

#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

#define IS_DIR(e) ((typeof(e)) (e))->d_type == DT_DIR

#define IS_HIDDEN(e) ((typeof(e)) (e))->d_name[0] == '.'

typedef struct {
    const char *d_name;
    unsigned char d_type;
    struct passwd *pw;
    struct group *gr;
    struct stat *st;
    size_t max_f_sz;
} Entry;

typedef struct {
    struct {
        Entry *data;
        size_t len;
        size_t cap;
    } entries;
    int current; // .  (-1 default)
    int parent;  // .. (-1 default)
    size_t max_f_sz;
} Listing;

Listing listing_ls(const char *const path);
void listing_show(Listing *listing);

#endif // LISTING_H
