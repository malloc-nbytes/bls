#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>

///////////////////////
// Macros
///////////////////////

// Forgrounds
#define YELLOW        "\033[93m"
#define GREEN         "\033[32m"
#define BRIGHT_GREEN  "\033[92m"
#define GRAY          "\033[90m"
#define RED           "\033[31m"
#define BLUE          "\033[94m"
#define CYAN          "\033[96m"
#define MAGENTA       "\033[95m"
#define WHITE         "\033[97m"
#define BLACK         "\033[30m"
#define CYAN          "\033[96m"
#define PINK          "\033[95m"
#define BRIGHT_PINK   "\033[38;5;213m"
#define PURPLE        "\033[35m"
#define BRIGHT_PURPLE "\033[95m"
#define ORANGE        "\033[38;5;214m"
#define BROWN         "\033[38;5;94m"

// Effects
#define UNDERLINE "\033[4m"
#define BOLD      "\033[1m"
#define ITALIC    "\033[3m"
#define DIM       "\033[2m"
#define INVERT    "\033[7m"
#define RESET     "\033[0m"

// Default terminal width if retrieving it fails.
#define BLS_DEFAULT_TERM_WIDTH 80

// One hyphen flags
#define FLAG_1HY_HELP       "-h"
#define FLAG_1HY_LONG       "-l"
#define FLAG_1HY_FILES_ONLY "-f"
#define FLAG_1HY_DIRS_ONLY  "-d"

// Two hyphen flags
#define FLAG_2HY_HELP       "--help"
#define FLAG_2HY_LONG       "--long"
#define FLAG_2HY_FILES_ONLY "--files"
#define FLAG_2HY_DIRS_ONLY  "--dirs"

///////////////////////
// Macros (Functions)
///////////////////////

#define BIT_SET(bits, bit) ((bits) & (bit)) != 0

#define da_append(arr, len, cap, ty, value)                       \
    do {                                                          \
        if ((len) >= (cap)) {                                     \
            (cap) = !(cap) ? 2 : (cap) * 2;                       \
            (arr) = (ty)realloc((arr), (cap) * sizeof((arr)[0])); \
        }                                                         \
        (arr)[(len)] = (value);                                   \
        (len) += 1;                                               \
    } while (0)

#define err(msg)                                \
    do {                                        \
        fprintf(stderr, "[Error]: " msg "\n");  \
        exit(1);                                \
    } while (0)

#define err_wargs(msg, ...)                                     \
    do {                                                        \
        fprintf(stderr, "[Error]: " msg "\n", __VA_ARGS__);     \
        exit(1);                                                \
    } while (0)

#define IS_DIR(e) ((typeof(e)) (e))->d_type == DT_DIR

///////////////////////
// Structures
///////////////////////

typedef enum uint32_t {
    FLAG_TYPE_HELP       = 1 << 0,
    FLAG_TYPE_LONG       = 1 << 1,
    FLAG_TYPE_FILES_ONLY = 1 << 2,
    FLAG_TYPE_DIRS_ONLY  = 1 << 3,
} Flag_Type;

typedef struct {
    struct dirent **data;
    size_t len;
    size_t cap;
    size_t stride;
    size_t current; // .
    size_t parent;  // ..
} Listing;

///////////////////////
// Globals
///////////////////////

static uint32_t g_flags = 0x0;
static char *g_progname = NULL;
static size_t g_term_width = 0;

///////////////////////
// Code
///////////////////////

Listing ls(const char *const path) {
    Listing listing = {0};
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (!dir)
        err_wargs("%s", strerror(errno));

    size_t i = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] && !entry->d_name[1])
            listing.current = i;
        else if (entry->d_name[0] && entry->d_name[1] == '.' && !entry->d_name[2])
            listing.parent = i;
        da_append(listing.data, listing.len, listing.cap, struct dirent **, entry);
        ++i;
    }

    closedir(dir);

    return listing;
}

void show_file(const struct dirent *const entry) {
    if (BIT_SET(g_flags, FLAG_TYPE_DIRS_ONLY))
        return;
    printf(WHITE);
}

void show_dir(const struct dirent *const entry) {
    if (BIT_SET(g_flags, FLAG_TYPE_FILES_ONLY))
        return;
    printf(GRAY UNDERLINE BOLD);
}

void dump_name(const struct dirent *const entry) {
    printf("%s" RESET, entry->d_name);
}

void show(Listing *listing) {

    // Handle the special dirs '.' and '..' first.
    printf(WHITE BOLD);
    printf("%s%s",
           listing->data[listing->current]->d_name,
           BIT_SET(g_flags, FLAG_TYPE_LONG) ? "\n" : "  ");
    printf("%s%s",
           listing->data[listing->parent]->d_name,
           BIT_SET(g_flags, FLAG_TYPE_LONG) ? "\n" : "  ");
    printf(RESET);

    // Go through all entries listed.
    for (size_t i = 0, char_acc = 0; i < listing->len; ++i) {

        // Skip special dirs.
        if (i == listing->parent || i == listing->current) continue;

        struct dirent *entry = listing->data[i];

        if (IS_DIR(entry)) show_dir(entry);
        else               show_file(entry);

        dump_name(entry);

        // Do not put extra characters after we have
        // gone through all entries.
        if (i != listing->len-1) {
            if (BIT_SET(g_flags, FLAG_TYPE_LONG))
                putchar('\n');
            else
                printf("  ");
        }

        // Keep track of how many characters we have printed
        // and put a newline, but only if LONG is not enabled.
        char_acc += strlen(entry->d_name);
        if (!BIT_SET(g_flags, FLAG_TYPE_LONG) && char_acc >= g_term_width/2) {
            char_acc = 0, putchar('\n');
        }
    }

    putchar('\n');
}

void usage(void) {
    printf("Usage: ");
    printf("%s [path] [options]\n", g_progname);
    exit(0);
}

char *eat(int *argc, char ***argv) {
    if (!(*argc)) return NULL;
    (*argc)--;
    return *(*argv)++;
}

void handle_flag(const char *arg, int *argc, char ***argv) {
    if (!strcmp(arg, FLAG_1HY_HELP) || !strcmp(arg, FLAG_2HY_HELP))
        usage();
    else if (!strcmp(arg, FLAG_1HY_LONG) || !strcmp(arg, FLAG_2HY_LONG))
        g_flags |= FLAG_TYPE_LONG;
    else if (!strcmp(arg, FLAG_1HY_FILES_ONLY) || !strcmp(arg, FLAG_2HY_FILES_ONLY))
        g_flags |= FLAG_TYPE_FILES_ONLY;
    else if (!strcmp(arg, FLAG_1HY_DIRS_ONLY) || !strcmp(arg, FLAG_2HY_DIRS_ONLY))
        g_flags |= FLAG_TYPE_DIRS_ONLY;
    else
        err_wargs("Unknown option: `%s`", arg);
}

int get_term_width(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        return BLS_DEFAULT_TERM_WIDTH;
    return w.ws_col;
}

int main(int argc, char **argv) {
    char *path = ".", *arg = NULL;

    g_term_width = get_term_width();
    g_progname = eat(&argc, &argv);

    while ((arg = eat(&argc, &argv)) != NULL) {
        if (arg[0] == '-')
            handle_flag(arg, &argc, &argv);
        else
            path = arg;
    }

    Listing listing = ls(path);
    show(&listing);

    return 0;
}
