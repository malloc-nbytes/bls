#ifndef LISTING_H
#define LISTING_H

#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

#define IS_DIR(e) ((typeof(e)) (e))->d_type == DT_DIR

#define BACK_ONE_DIR2(e) (((typeof(e)) (e))->d_name[0] == '.'           \
                          && ((typeof(e)) (e))->d_name[1]               \
                          && ((typeof(e)) (e))->d_name[1] == '.'        \
                          && ((typeof(e)) (e))->d_name[2]               \
                          && ((typeof(e)) (e))->d_name[2] == '/')

#define BACK_ONE_DIR1(e) (((typeof(e)) (e))->d_name[0] == '.'           \
                          && ((typeof(e)) (e))->d_name[1]               \
                          && ((typeof(e)) (e))->d_name[1] == '/')

#define CUR_DIR(e) (((typeof(e)) (e))->d_name[0] == '.'                 \
                    && ((typeof(e)) (e))->d_name[1]                     \
                    && ((typeof(e)) (e))->d_name[0] == '/')

#define IS_HIDDEN(e)                                                    \
    (((typeof(e)) (e))->d_name[0] == '.'                                \
     && !BACK_ONE_DIR1(e) && !BACK_ONE_DIR2(e) && !CUR_DIR(e))

typedef struct {
    const char *d_name;
    unsigned char d_type;
    struct passwd *pw;
    struct group *gr;
    struct stat *st;
    size_t max_f_sz; // Should be same a `Listing`
} Entry;

typedef struct {
    struct {
        Entry *data;
        size_t len;
        size_t cap;
    } entries;
    int current;     // .  (-1 default)
    int parent;      // .. (-1 default)
    size_t max_f_sz; // Max filesize digits length
} Listing;

Listing listing_ls(const char *const path);
void listing_show(Listing *listing);

#endif // LISTING_H
