#ifndef LISTING_H
#define LISTING_H

#include <stddef.h>
#include <dirent.h>

#define IS_DIR(e) ((typeof(e)) (e))->d_type == DT_DIR

#define IS_HIDDEN(e) ((typeof(e)) (e))->d_name[0] == '.'

typedef struct {
    struct dirent **data;
    size_t len;
    size_t cap;
    size_t stride;
    int current; // .  (-1 default)
    int parent;  // .. (-1 default)
} Listing;

Listing listing_ls(const char *const path);
void listing_show(Listing *listing);

#endif // LISTING_H
