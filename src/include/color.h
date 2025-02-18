#ifndef COLOR_H
#define COLOR_H

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

void color(const char *c);

#endif // COLOR_H
