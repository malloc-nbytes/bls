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
#include "color.h"
#include "utils.h"

uint32_t g_flags      = 0x0;
char    *g_progname   = NULL;
size_t   g_term_width = 0;

void usage(void) {
    printf("(MIT License) Copyright (c) 2025 malloc-nbytes\n\n");

    printf("Usage:\n");
    printf("  %s [path] [options]\n", g_progname);
    printf("Options:\n");
    printf("  " FLAG_2HY_HELP ",  -%c         Show this message\n", FLAG_1HY_HELP);
    printf("  " FLAG_2HY_LONG ",  -%c         Show entries vertically\n", FLAG_1HY_LONG);
    printf("  " FLAG_2HY_FILES_ONLY ", -%c         Only show files\n", FLAG_1HY_FILES_ONLY);
    printf("  " FLAG_2HY_DIRS_ONLY ",  -%c         Only show directories\n", FLAG_1HY_DIRS_ONLY);
    printf("  " FLAG_2HY_ALL ",   -%c         Show all entries\n", FLAG_1HY_ALL);
    printf("  " FLAG_2HY_PERMISSIONS ", -%c   Show file permissions\n", FLAG_1HY_PERMISSIONS);
    printf("  " FLAG_2HY_USER ",  -%c         Show owner\n", FLAG_1HY_USER);
    printf("  " FLAG_2HY_GROUP ", -%c         Show group\n", FLAG_1HY_GROUP);
    printf("  " FLAG_2HY_TIME ",  -%c         Show time\n", FLAG_1HY_TIME);
    printf("  " FLAG_2HY_SZ ",  -%c         Show size\n", FLAG_1HY_SZ);
    printf("  " FLAG_2HY_NO_COLOR "          Do not use colors or effects\n");

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
        g_flags |= FLAG_TYPE_PERMISSIONS | FLAG_TYPE_LONG;
    else if (!strcmp(arg, FLAG_2HY_USER))
        g_flags |= FLAG_TYPE_USER | FLAG_TYPE_LONG;
    else if (!strcmp(arg, FLAG_2HY_GROUP))
        g_flags |= FLAG_TYPE_GROUP | FLAG_TYPE_LONG;
    else if (!strcmp(arg, FLAG_2HY_TIME))
        g_flags |= FLAG_TYPE_TIME | FLAG_TYPE_LONG;
    else if (!strcmp(arg, FLAG_2HY_SZ))
        g_flags |= FLAG_TYPE_SZ | FLAG_TYPE_LONG;
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
            g_flags |= FLAG_TYPE_PERMISSIONS;// | FLAG_TYPE_LONG;
        else if (*it == FLAG_1HY_USER)
            g_flags |= FLAG_TYPE_USER;// | FLAG_TYPE_LONG;
        else if (*it == FLAG_1HY_GROUP)
            g_flags |= FLAG_TYPE_GROUP;// | FLAG_TYPE_LONG;
        else if (*it == FLAG_1HY_TIME)
            g_flags |= FLAG_TYPE_TIME;// | FLAG_TYPE_LONG;
        else if (*it == FLAG_1HY_SZ)
            g_flags |= FLAG_TYPE_SZ;// | FLAG_TYPE_LONG;
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
        size_t len, cap;
    } paths = {0};

    g_term_width = get_term_width();
    g_progname = eat(&argc, &argv);

    char *arg = NULL;
    while ((arg = eat(&argc, &argv)) != NULL) {
        if (arg[0] && SAFE_PEEK(arg, 1, '-'))
            handle_2hy_flag(arg, &argc, &argv);
        else if (arg[0] == '-' && arg[1])
            handle_1hy_flag(arg, &argc, &argv);
        else
            da_append(paths.data, paths.len, paths.cap, const char **, arg);
    }


    if (paths.len == 0) {
        printf(".:\n");
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
