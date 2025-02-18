#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "listing.h"
#include "config.h"
#include "color.h"
#include "utils.h"

static int show_file(const struct dirent *const entry) {
    if (BIT_SET(g_flags, FLAG_TYPE_DIRS_ONLY))
        return 0;
    if (IS_HIDDEN(entry)) {
        if (!BIT_SET(g_flags, FLAG_TYPE_ALL))
            return 0;
        color(BLS_DEFAULT_COLOR_FILE_DT);
    }
    else
        color(BLS_DEFAULT_COLOR_FILE);
    printf("%s" RESET, entry->d_name);
    return 1;
}

static int show_dir(const struct dirent *const entry) {
    if (BIT_SET(g_flags, FLAG_TYPE_FILES_ONLY))
        return 0;
    if (IS_HIDDEN(entry)) {
        if (!BIT_SET(g_flags, FLAG_TYPE_ALL))
            return 0;
        color(BLS_DEFAULT_COLOR_DIR_DT);
    }
    else
        color(BLS_DEFAULT_COLOR_DIR);
    printf("%s" RESET, entry->d_name);
    return 1;
}

void listing_show(Listing *listing) {
    // Handle the special dirs '.' and '..' first.
    if (BIT_SET(g_flags, FLAG_TYPE_ALL)) {
        color(GRAY UNDERLINE BOLD);
        printf("%s" RESET, listing->data[listing->current]->d_name);
        printf(BIT_SET(g_flags, FLAG_TYPE_LONG) ? "\n" : "  ");

        color(GRAY UNDERLINE BOLD);
        printf("%s" RESET, listing->data[listing->parent]->d_name);
        printf(BIT_SET(g_flags, FLAG_TYPE_LONG) ? "\n" : "  ");
    }

    // Go through all entries listed.
    for (size_t i = 0, char_acc = 0; i < listing->len; ++i) {

        // Skip special dirs.
        if (i == listing->parent || i == listing->current) continue;

        struct dirent *entry = listing->data[i];

        int showed = 0;
        if (IS_DIR(entry)) showed = show_dir(entry);
        else               showed = show_file(entry);

        // Do not put extra characters after we have
        // gone through all entries.
        if (showed && i != listing->len-1) {
            if (BIT_SET(g_flags, FLAG_TYPE_LONG))
                putchar('\n');
            else
                printf("  ");
        }

        // Keep track of how many characters we have printed
        // and put a newline, but only if LONG is not enabled.
        if (showed) {
            char_acc += strlen(entry->d_name);
            if (!BIT_SET(g_flags, FLAG_TYPE_LONG) && char_acc >= g_term_width/2) {
                char_acc = 0, putchar('\n');
            }
        }
    }

    putchar('\n');
}

Listing listing_ls(const char *const path) {
    DIR *dir;
    struct dirent *entry;

    Listing listing = {0};
    listing.parent = listing.current = -1;

    dir = opendir(path);
    if (!dir)
        err_wargs("%s", strerror(errno));

    size_t i = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' && !entry->d_name[1])
            listing.current = i;
        else if (entry->d_name[0] == '.' && entry->d_name[1] && entry->d_name[1] == '.' && !entry->d_name[2])
            listing.parent = i;
        da_append(listing.data, listing.len, listing.cap, struct dirent **, entry);
        ++i;
    }

    closedir(dir);

    return listing;
}
