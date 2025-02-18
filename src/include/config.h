#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stddef.h>

// Default terminal width if retrieving it fails.
#define BLS_DEFAULT_TERM_WIDTH 80

// One hyphen flags
#define FLAG_1HY_HELP        'h'
#define FLAG_1HY_LONG        'l'
#define FLAG_1HY_FILES_ONLY  'f'
#define FLAG_1HY_DIRS_ONLY   'd'
#define FLAG_1HY_ALL         'a'
#define FLAG_1HY_PERMISSIONS 'p'
#define FLAG_1HY_USER        'u'
#define FLAG_1HY_GROUP       'g'

#define FLAG_1HY_ASCPL {                        \
        FLAG_1HY_HELP,                          \
        FLAG_1HY_LONG,                          \
        FLAG_1HY_FILES_ONLY,                    \
        FLAG_1HY_DIRS_ONLY,                     \
        FLAG_1HY_ALL,                           \
        FLAG_1HY_PERMISSIONS,                   \
        FLAG_1HY_USER,                          \
        FLAG_1HY_GROUP                          \
}

// Two hyphen flags
#define FLAG_2HY_HELP        "--help"
#define FLAG_2HY_LONG        "--long"
#define FLAG_2HY_FILES_ONLY  "--files"
#define FLAG_2HY_DIRS_ONLY   "--dirs"
#define FLAG_2HY_ALL         "--all"
#define FLAG_2HY_NO_COLOR    "--no-color"
#define FLAG_2HY_PERMISSIONS "--permissions"
#define FLAG_2HY_USER        "--user"
#define FLAG_2HY_GROUP       "--group"

#define FLAG_2HY_ASCPL {         \
        FLAG_2HY_HELP,           \
        FLAG_2HY_LONG,           \
        FLAG_2HY_FILES_ONLY,     \
        FLAG_2HY_DIRS_ONLY,      \
        FLAG_2HY_ALL,            \
        FLAG_2HY_NO_COLOR,       \
        FLAG_2HY_PERMISSIONS,    \
        FLAG_2HY_USER,           \
        FLAG_2HY_GROUP,          \
}

typedef enum uint32_t {
    FLAG_TYPE_HELP        = 1 << 0,
    FLAG_TYPE_LONG        = 1 << 1,
    FLAG_TYPE_FILES_ONLY  = 1 << 2,
    FLAG_TYPE_DIRS_ONLY   = 1 << 3,
    FLAG_TYPE_ALL         = 1 << 4,
    FLAG_TYPE_NO_COLOR    = 1 << 5,
    FLAG_TYPE_PERMISSIONS = 1 << 6,
    FLAG_TYPE_USER        = 1 << 7,
    FLAG_TYPE_GROUP       = 1 << 8,
} Flag_Type;

extern uint32_t g_flags;
extern char    *g_progname;
extern size_t   g_term_width;

#endif // CONFIG_H
