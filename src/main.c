#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "config.h"
#include "listing.h"
#include "utils.h"

uint32_t g_flags      = 0x0;
char    *g_progname   = NULL;
size_t   g_term_width = 0;

void usage(void) {
    printf("Usage: ");
    printf("%s [path] [options]\n", g_progname);
    exit(0);
}

char *eat(int *argc, char ***argv) {
    if (!(*argc))
        return NULL;
    (*argc)--;
    return *(*argv)++;
}

void handle_2hy_flag(const char *arg, int *argc, char ***argv) {
    if (!strcmp(arg, FLAG_2HY_HELP))
        usage();
    else if (!strcmp(arg, FLAG_2HY_LONG))
        g_flags |= FLAG_TYPE_LONG;
    else if (!strcmp(arg, FLAG_2HY_FILES_ONLY))
        g_flags |= FLAG_TYPE_FILES_ONLY;
    else if (!strcmp(arg, FLAG_2HY_DIRS_ONLY))
        g_flags |= FLAG_TYPE_DIRS_ONLY;
    else if (!strcmp(arg, FLAG_2HY_ALL))
        g_flags |= FLAG_TYPE_ALL;
    else if (!strcmp(arg, FLAG_2HY_NO_COLOR))
        g_flags |= FLAG_TYPE_NO_COLOR;
    else if (!strcmp(arg, FLAG_2HY_PERMISSIONS))
        g_flags |= FLAG_TYPE_PERMISSIONS;
    else
        err_wargs("Unknown option: `%s`", arg);
}

void handle_1hy_flag(const char *arg, int *argc, char ***argv) {
    const char *it = arg+1;
    while (it && *it != ' ' && *it != '\0') {
        if (*it == FLAG_1HY_HELP)
            usage();
        else if (*it == FLAG_1HY_LONG)
            g_flags |= FLAG_TYPE_LONG;
        else if (*it == FLAG_1HY_FILES_ONLY)
            g_flags |= FLAG_TYPE_FILES_ONLY;
        else if (*it == FLAG_1HY_DIRS_ONLY)
            g_flags |= FLAG_TYPE_DIRS_ONLY;
        else if (*it == FLAG_1HY_ALL)
            g_flags |= FLAG_TYPE_ALL;
        else if (*it == FLAG_1HY_PERMISSIONS)
            g_flags |= FLAG_TYPE_PERMISSIONS;
        else
            err_wargs("Unknown option: `%c`", *it);
            ++it;
    }
}

int get_term_width(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        return BLS_DEFAULT_TERM_WIDTH;
    return w.ws_col;
}

int main(int argc, char **argv) {
    struct {
        const char **data;
        size_t len;
        size_t cap;
    } paths;

    char *arg = NULL;

    g_term_width = get_term_width();
    g_progname = eat(&argc, &argv);

    while ((arg = eat(&argc, &argv)) != NULL) {
        if (arg[0] && safe_peek(arg, 1, '-'))
            handle_2hy_flag(arg, &argc, &argv);
        else if (arg[0] == '-' && arg[1])
            handle_1hy_flag(arg, &argc, &argv);
        else
            da_append(paths.data, paths.len, paths.cap, const char **, arg);
    }

    if (paths.len == 0) {
        Listing listing = listing_ls(".");
        listing_show(&listing);
    }
    else {
        for (size_t i = 0; i < paths.len; ++i) {
            printf("%s:\n", paths.data[i]);
            Listing listing = listing_ls(paths.data[i]);
            listing_show(&listing);
        }
    }

    return 0;
}
