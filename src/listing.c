#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

#include "listing.h"
#include "config.h"
#include "color.h"
#include "mem.h"
#include "utils.h"

static void show_permissions(mode_t mode) {
    char perms[11];
    perms[0] = (S_ISDIR(mode)) ? 'd' : '-';
    perms[1] = (mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (mode & S_IROTH) ? 'r' : '-';
    perms[8] = (mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (mode & S_IXOTH) ? 'x' : '-';
    perms[10] = '\0';
    printf("%s ", perms);
}

static void show_groups(const struct passwd *const pw) {
    printf("%s", pw ? pw->pw_name : "unknown");
}

static void show_users(const struct group *const gr) {
    printf("%s", gr ? gr->gr_name : "unknown");
}

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
    //show_groups(entry->);
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
        printf("%s" RESET, listing->entries.actual[listing->current]->d_name);
        printf(BIT_SET(g_flags, FLAG_TYPE_LONG) ? "\n" : "  ");

        color(GRAY UNDERLINE BOLD);
        printf("%s" RESET, listing->entries.actual[listing->parent]->d_name);
        printf(BIT_SET(g_flags, FLAG_TYPE_LONG) ? "\n" : "  ");
    }

    // Go through all entries listed.
    for (size_t i = 0, char_acc = 0; i < listing->entries.len; ++i) {

        // Skip special dirs.
        if (i == listing->parent || i == listing->current) continue;

        struct dirent *entry = listing->entries.actual[i];

        int showed = 0;
        if (IS_DIR(entry)) showed = show_dir(entry);
        else               showed = show_file(entry);

        // Do not put extra characters after we have
        // gone through all entries.
        if (showed && i != listing->entries.len-1) {
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

    const size_t fullpath_len = 1024;
    char fullpath[fullpath_len];

    Listing listing = {0};
    listing.parent = listing.current = -1;

    dir = opendir(path);
    if (!dir)
        err_wargs("%s", strerror(errno));

    size_t i = 0;
    while ((entry = readdir(dir)) != NULL) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat *stat_ = s_malloc(sizeof(struct stat));

        if (stat(fullpath, stat_) == -1) {
            perror("stat");
            continue;
        }

        struct passwd *pw = getpwuid(stat_->st_uid);
        struct group *gr = getgrgid(stat_->st_gid);

        // Capture indices of special directories.
        if (entry->d_name[0] == '.' && !entry->d_name[1])
            listing.current = i;
        else if (entry->d_name[0] == '.' && entry->d_name[1] && entry->d_name[1] == '.' && !entry->d_name[2])
            listing.parent = i;

        da_append(listing.entries.actual, listing.entries.len, listing.entries.cap, struct dirent **, entry);
        da_append(listing.entries.stats,  listing.entries.len, listing.entries.cap, struct stat **,   stat_);
        da_append(listing.entries.pws,    listing.entries.len, listing.entries.cap, struct passwd **, pw);
        da_append(listing.entries.grs,    listing.entries.len, listing.entries.cap, struct group **,  gr);

        (void)memset(fullpath, '\0', fullpath_len);
        ++i;
    }

    closedir(dir);

    return listing;
}
