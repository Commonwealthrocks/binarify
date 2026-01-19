// ram_mng.h
// last updated: 19/01/2026 <d/m/y>
#ifndef RAM_MNG_H
#define RAM_MNG_H
#include "../c_header_files/common.h"
#define MAX_MAPPED_MODULES 64
void add_mapped_module(void *base);
int is_mapped_module(HMODULE hMod);
extern void *g_main_base;
void *manual_map_pe(unsigned char *pe_data, size_t pe_size);
int execute_entry_point(void *base);
int execute_from_memory(unsigned char *pe_data, size_t pe_size);
FARPROC get_proc_address_from_memory(void *base, LPCSTR name);
#endif

// end