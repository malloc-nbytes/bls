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

#define BLS_DEFAULT_COLOR_DIR GRAY
#define BLS_DEFAULT_COLOR_DIR_DT GRAY DIM

#define BLS_DEFAULT_COLOR_FILE WHITE
#define BLS_DEFAULT_COLOR_FILE_DT WHITE DIM

// Default terminal width if retrieving it fails.
#define BLS_DEFAULT_TERM_WIDTH 80

// One hyphen flags
#define FLAG_1HY_HELP       'h'
#define FLAG_1HY_LONG       'l'
#define FLAG_1HY_FILES_ONLY 'f'
#define FLAG_1HY_DIRS_ONLY  'd'
#define FLAG_1HY_ALL        'a'

// Two hyphen flags
#define FLAG_2HY_HELP       "--help"
#define FLAG_2HY_LONG       "--long"
#define FLAG_2HY_FILES_ONLY "--files"
#define FLAG_2HY_DIRS_ONLY  "--dirs"
#define FLAG_2HY_ALL        "--all"
#define FLAG_2HY_NO_COLOR   "--no-color"

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

#define IS_HIDDEN(e) ((typeof(e)) (e))->d_name[0] == '.'

///////////////////////
// Structures
///////////////////////

typedef enum uint32_t {
    FLAG_TYPE_HELP       = 1 << 0,
    FLAG_TYPE_LONG       = 1 << 1,
    FLAG_TYPE_FILES_ONLY = 1 << 2,
    FLAG_TYPE_DIRS_ONLY  = 1 << 3,
    FLAG_TYPE_ALL        = 1 << 4,
    FLAG_TYPE_NO_COLOR   = 1 << 5,
} Flag_Type;

typedef struct {
    struct dirent **data;
    size_t len;
    size_t cap;
    size_t stride;
    int current; // .  (-1 default)
    int parent;  // .. (-1 default)
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

void color(const char *c) {
    if (BIT_SET(g_flags, FLAG_TYPE_NO_COLOR))
        return;
    printf(c);
}

int show_file(const struct dirent *const entry) {
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

int show_dir(const struct dirent *const entry) {
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

void show(Listing *listing) {

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
    char *path = ".", *arg = NULL;

    g_term_width = get_term_width();
    g_progname = eat(&argc, &argv);

    while ((arg = eat(&argc, &argv)) != NULL) {
        if (arg[0] == '-' && arg[1] && arg[1] == '-')
            handle_2hy_flag(arg, &argc, &argv);
        else if (arg[0] == '-' && arg[1])
            handle_1hy_flag(arg, &argc, &argv);
        else
            path = arg;
    }

    Listing listing = ls(path);
    show(&listing);

    return 0;
}
