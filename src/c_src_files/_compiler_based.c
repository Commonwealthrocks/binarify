// _compiler_based.c
// last updated: 19/01/2026 <d/m/y>
#include "../c_header_files/_compiler_based.h"
#include <windows.h>
#include <stdio.h>

#if defined(_M_ARM) || defined(_M_ARM64)
#error "Architecture check failed: ARM / ARM64 is not supported."
#endif

#if !defined(__GNUC__) && !defined(_MSC_VER)
#error "Compiler check failed: unknown compiler, please use GCC."
#endif
void secure_zero(void *ptr, size_t len)
{
    RtlSecureZeroMemory(ptr, len);
}

// end