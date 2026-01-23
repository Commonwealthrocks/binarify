// outs.c
// last updated: 18/01/2026 <d/m/y>
#include "../c_header_files/outs.h"
#include <ctype.h>
int g_silent = 0;

int str_icmp(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2))
            return 0;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) == tolower((unsigned char)*s2);
}

// end
