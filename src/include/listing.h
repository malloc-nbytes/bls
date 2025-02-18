#ifndef LISTING_H
#define LISTING_H

#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

#define IS_DIR(e) ((typeof(e)) (e))->dirent->d_type == DT_DIR

#define IS_HIDDEN(e) ((typeof(e)) (e))->dirent->d_name[0] == '.'

typedef struct {
    struct dirent *dirent;
    struct passwd *pw;
    struct group *gr;
    struct stat *st;
} Entry;

typedef struct {
    struct {
        Entry *data;
        size_t len;
        size_t cap;
    } entries;
    int current; // .  (-1 default)
    int parent;  // .. (-1 default)
} Listing;

Listing listing_ls(const char *const path);
void listing_show(Listing *listing);

#endif // LISTING_H
