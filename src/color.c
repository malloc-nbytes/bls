#include <stdio.h>

#include "color.h"
#include "config.h"
#include "utils.h"

void color(const char *c) {
    if (BIT_SET(g_flags, FLAG_TYPE_NO_COLOR))
        return;
    printf(c);
}

void padding(void) {
    //printf("%*s", 2, "");
    if (!BIT_SET(g_flags, FLAG_TYPE_LONG))
        printf(" ─ ");
    else
        printf("├─ ");
}
