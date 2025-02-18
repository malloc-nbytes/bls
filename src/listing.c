#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#include "listing.h"
#include "config.h"
#include "color.h"
#include "mem.h"
#include "utils.h"

static void show_permissions(const Entry *const entry) {
    if (!BIT_SET(g_flags, FLAG_TYPE_PERMISSIONS))
        return;
    mode_t mode = entry->st->st_mode;
    char perms[11];
    perms[0] = (S_ISDIR(mode))        ? 'd' : '-';
    perms[1] = BIT_SET(mode, S_IRUSR) ? 'r' : '-';
    perms[2] = BIT_SET(mode, S_IWUSR) ? 'w' : '-';
    perms[3] = BIT_SET(mode, S_IXUSR) ? 'x' : '-';
    perms[4] = BIT_SET(mode, S_IRGRP) ? 'r' : '-';
    perms[5] = BIT_SET(mode, S_IWGRP) ? 'w' : '-';
    perms[6] = BIT_SET(mode, S_IXGRP) ? 'x' : '-';
    perms[7] = BIT_SET(mode, S_IROTH) ? 'r' : '-';
    perms[8] = BIT_SET(mode, S_IWOTH) ? 'w' : '-';
    perms[9] = BIT_SET(mode, S_IXOTH) ? 'x' : '-';
    perms[10] = '\0';
    printf("%s ", perms);
}

static void show_group(const Entry *const entry) {
    if (!BIT_SET(g_flags, FLAG_TYPE_GROUP))
        return;
    printf("%s ", entry->pw ? entry->pw->pw_name : "unknown");
}

static void show_user(const Entry *const entry) {
    if (!BIT_SET(g_flags, FLAG_TYPE_USER))
        return;
    printf("%s ", entry->gr ? entry->gr->gr_name : "unknown");
}

static void show_date(const Entry *const entry) {
    if (!BIT_SET(g_flags, FLAG_TYPE_TIME))
        return;
    char *time = ctime(&entry->st->st_mtime);
    if (!time) {
        printf("??????????????");
    }
    else {
        time[strcspn(time, "\n")] = '\0';
        printf("%s ", time);
    }
}

static void show_filesz(const Entry *const entry) {
    if (!BIT_SET(g_flags, FLAG_TYPE_SZ))
        return;
    if (FLAG_IS_VERBOSE(g_flags) && BIT_SET(g_flags, FLAG_TYPE_LONG))
        printf("%*lld ", entry->max_f_sz, (long long)entry->st->st_size);
    else
        printf("%lld ", (long long)entry->st->st_size);
}

static int show_file(const Entry *const entry) {
    if (BIT_SET(g_flags, FLAG_TYPE_DIRS_ONLY))
        return 0;

    const char *format = NULL;

    if (IS_HIDDEN(entry)) {
        if (!BIT_SET(g_flags, FLAG_TYPE_ALL))
            return 0;
        format = BLS_DEFAULT_COLOR_FILE_DT;
    }
    else
        format = BLS_DEFAULT_COLOR_FILE;

    padding();

    if (FLAG_IS_VERBOSE(g_flags) && !BIT_SET(g_flags, FLAG_TYPE_LONG))
        putchar('(');

    show_permissions(entry);
    show_user(entry);
    show_group(entry);
    show_filesz(entry);
    show_date(entry);

    color(format);
    printf("%s" RESET, entry->d_name);

    if (FLAG_IS_VERBOSE(g_flags) && !BIT_SET(g_flags, FLAG_TYPE_LONG))
        putchar(')');

    return 1;
}

static int show_dir(const Entry *const entry) {
    if (BIT_SET(g_flags, FLAG_TYPE_FILES_ONLY))
        return 0;

    const char *format = NULL;

    if (IS_HIDDEN(entry)) {
        if (!BIT_SET(g_flags, FLAG_TYPE_ALL))
            return 0;
        format = BLS_DEFAULT_COLOR_DIR_DT;
    }
    else
        format = BLS_DEFAULT_COLOR_DIR;

    padding();

    if (FLAG_IS_VERBOSE(g_flags) && !BIT_SET(g_flags, FLAG_TYPE_LONG))
        putchar('(');

    show_permissions(entry);
    show_user(entry);
    show_group(entry);
    show_filesz(entry);
    show_date(entry);

    color(format);
    printf("%s" RESET, entry->d_name);

    if (FLAG_IS_VERBOSE(g_flags) && !BIT_SET(g_flags, FLAG_TYPE_LONG))
        putchar(')');

    return 1;
}

void listing_show(Listing *listing) {
    for (size_t i = 0, char_acc = 0; i < listing->entries.len; ++i) {
        Entry *entry = &listing->entries.data[i];

        int showed = 0;
        if (IS_DIR(entry)) showed = show_dir(entry);
        else               showed = show_file(entry);

        // Do not put extra characters after we have
        // gone through all entries.
        if (showed && i != listing->entries.len-1) {
            if (BIT_SET(g_flags, FLAG_TYPE_LONG))
                putchar('\n');
        }

        // Keep track of how many characters we have printed
        // and put a newline, but only if LONG is not enabled.
        if (showed) {
            char_acc += strlen(entry->d_name);
            if (!BIT_SET(g_flags, FLAG_TYPE_LONG)
                && char_acc >= g_term_width/2
                && i        != listing->entries.len-1)
                char_acc = 0, putchar('\n');
        }
    }

    putchar('\n');
}

Listing listing_ls(const char *const path) {
    DIR *dir;
    struct dirent *entry;
    struct stat path_check_stat;

    (void)stat(path, &path_check_stat);

    Listing listing = {0};
    listing.parent = listing.current = -1;

    if (!S_ISDIR(path_check_stat.st_mode)) {
        struct passwd *pw = getpwuid(path_check_stat.st_uid);
        struct group *gr = getgrgid(path_check_stat.st_gid);

        listing.max_f_sz = snprintf(NULL, 0, "%lld", (long long)path_check_stat.st_size);

        Entry entry_obj = {
            .d_name = strdup(path),
            .d_type = S_ISDIR(path_check_stat.st_mode) ? DT_DIR : DT_REG,
            .pw = pw,
            .gr = gr,
            .st = s_malloc(sizeof(struct stat)),
            .max_f_sz = listing.max_f_sz,
        };

        *(entry_obj.st) = path_check_stat;

        da_append(listing.entries.data, listing.entries.len, listing.entries.cap, Entry *, entry_obj);
        return listing;
    }

    dir = opendir(path);
    if (!dir)
        err_wargs("%s", strerror(errno));

    const size_t fullpath_len = 1024;
    char fullpath[fullpath_len];
    size_t i = 0;
    while ((entry = readdir(dir)) != NULL) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat *stat_ = s_malloc(sizeof(struct stat));

        (void)stat(fullpath, stat_);

        struct passwd *pw = getpwuid(stat_->st_uid);
        struct group  *gr = getgrgid(stat_->st_gid);

        // Capture indices of special directories.
        if (entry->d_name[0] == '.' && !entry->d_name[1])
            listing.current = i;
        else if (entry->d_name[0] == '.' && entry->d_name[1] && entry->d_name[1] == '.' && !entry->d_name[2])
            listing.parent = i;

        Entry entry_obj = (Entry) {
            .d_name = strdup(entry->d_name),
            .d_type = entry->d_type,
            .pw = pw,
            .gr = gr,
            .st = stat_,
            .max_f_sz = 0,
        };

        da_append(listing.entries.data, listing.entries.len, listing.entries.cap, Entry *, entry_obj);

        (void)memset(fullpath, '\0', fullpath_len);

        // Get the length of the filesize digits.
        size_t file_sz_len = snprintf(NULL, 0, "%lld", (long long)entry_obj.st->st_size);
        if (file_sz_len > listing.max_f_sz)
            listing.max_f_sz = file_sz_len;

        ++i;
    }

    closedir(dir);

    if (listing.current != -1 && listing.parent != -1) {
        // Swap current (.) with the first entry if it's not already there
        if (listing.current != 0) {
            Entry temp = listing.entries.data[0];
            listing.entries.data[0] = listing.entries.data[listing.current];
            listing.entries.data[listing.current] = temp;

            // Update index of current (.)
            listing.current = 0;
        }

        // Swap parent (..) with the second entry if it's not already there
        if (listing.parent != 1) {
            Entry temp = listing.entries.data[1];
            listing.entries.data[1] = listing.entries.data[listing.parent];
            listing.entries.data[listing.parent] = temp;

            // Update index of parent (..)
            listing.parent = 1;
        }
    }

    for (size_t i = 0; i < listing.entries.len; ++i)
        listing.entries.data[i].max_f_sz = listing.max_f_sz;

    return listing;
}
