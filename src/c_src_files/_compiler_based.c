// _compiler_based.c
// last updated: 18/01/2026 <d/m/y>
#include "../c_header_files/_compiler_based.h"
#include <windows.h>
void secure_zero(void *ptr, size_t len)
{
    RtlSecureZeroMemory(ptr, len);
}

// end