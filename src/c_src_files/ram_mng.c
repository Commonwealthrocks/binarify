// ram_mng.c
// last updated: 19/01/2026 <d/m/y>
#include "../c_header_files/ram_mng.h"
#include "../c_header_files/vfs_mng.h"
#include "../c_header_files/outs.h"
static void *g_mapped_modules[MAX_MAPPED_MODULES];
static int g_mapped_count = 0;
void *g_main_base = NULL;
void add_mapped_module(void *base)
{
    if (g_mapped_count < MAX_MAPPED_MODULES)
    {
        g_mapped_modules[g_mapped_count++] = base;
    }
}
int is_mapped_module(HMODULE hMod)
{
    for (int i = 0; i < g_mapped_count; i++)
    {
        if (g_mapped_modules[i] == (void *)hMod)
            return 1;
    }
    return 0;
}
void *manual_map_pe(unsigned char *pe_data, size_t pe_size)
{
    (void)pe_size;
    if (!g_silent)
        printf("[DEBUG] Manual mapping PE...\n");
    IMAGE_DOS_HEADER_MINI *dos = (IMAGE_DOS_HEADER_MINI *)pe_data;
    if (dos->e_magic != 0x5A4D)
    {
        if (!g_silent)
            printf("[ERROR] Invalid DOS signature.\n");
        return NULL;
    }
    IMAGE_NT_HEADERS64_FULL *nt = (IMAGE_NT_HEADERS64_FULL *)(pe_data + dos->e_lfanew);
    if (nt->Signature != 0x4550)
    {
        if (!g_silent)
            printf("[ERROR] Invalid PE signature.\n");
        return NULL;
    }
    ULONGLONG imageBase = nt->OptionalHeader.ImageBase;
    DWORD imageSize = nt->OptionalHeader.SizeOfImage;
    void *base = VirtualAlloc(NULL, imageSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!base)
    {
        if (!g_silent)
            printf("[ERROR] I js cannot allocate the memory bro.\n");
        return NULL;
    }
    add_mapped_module(base);
    memcpy(base, pe_data, nt->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER *section = (IMAGE_SECTION_HEADER *)((BYTE *)nt + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + nt->FileHeader.SizeOfOptionalHeader);
    for (int i = 0; i < nt->FileHeader.NumberOfSections; i++)
    {
        if (section[i].SizeOfRawData > 0)
        {
            memcpy((BYTE *)base + section[i].VirtualAddress, pe_data + section[i].PointerToRawData, section[i].SizeOfRawData);
        }
    }
    ULONGLONG delta = (ULONGLONG)base - imageBase;
    if (delta != 0)
    {
        IMAGE_DATA_DIRECTORY *relocDir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
        if (relocDir->Size > 0)
        {
            IMAGE_BASE_RELOCATION *reloc = (IMAGE_BASE_RELOCATION *)((BYTE *)base + relocDir->VirtualAddress);
            while (reloc->VirtualAddress > 0)
            {
                DWORD count = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                WORD *relocData = (WORD *)((BYTE *)reloc + sizeof(IMAGE_BASE_RELOCATION));
                for (DWORD i = 0; i < count; i++)
                {
                    int type = relocData[i] >> 12;
                    int offset = relocData[i] & 0xFFF;
                    if (type == IMAGE_REL_BASED_DIR64)
                    {
                        ULONGLONG *patchAddr = (ULONGLONG *)((BYTE *)base + reloc->VirtualAddress + offset);
                        *patchAddr += delta;
                    }
                }
                reloc = (IMAGE_BASE_RELOCATION *)((BYTE *)reloc + reloc->SizeOfBlock);
            }
        }
    }
    IMAGE_DATA_DIRECTORY *importDir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (importDir->Size > 0)
    {
        IMAGE_IMPORT_DESCRIPTOR *importDesc = (IMAGE_IMPORT_DESCRIPTOR *)((BYTE *)base + importDir->VirtualAddress);
        while (importDesc->Name != 0)
        {
            char *dllName = (char *)((BYTE *)base + importDesc->Name);
            HMODULE hDll = Hooked_LoadLibraryA(dllName);
            if (hDll)
            {
                IMAGE_THUNK_DATA64 *thunk = (IMAGE_THUNK_DATA64 *)((BYTE *)base + importDesc->FirstThunk);
                IMAGE_THUNK_DATA64 *origThunk = (IMAGE_THUNK_DATA64 *)((BYTE *)base + importDesc->OriginalFirstThunk);
                while (thunk->u1.AddressOfData != 0)
                {
                    if (origThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64)
                    {
                        WORD ordinal = IMAGE_ORDINAL64(origThunk->u1.Ordinal);
                        thunk->u1.Function = (ULONGLONG)Hooked_GetProcAddress(hDll, (LPCSTR)(ULONG_PTR)ordinal);
                    }
                    else
                    {
                        IMAGE_IMPORT_BY_NAME *importName = (IMAGE_IMPORT_BY_NAME *)((BYTE *)base + origThunk->u1.AddressOfData);
                        thunk->u1.Function = (ULONGLONG)Hooked_GetProcAddress(hDll, importName->Name);
                    }
                    thunk++;
                    origThunk++;
                }
            }
            importDesc++;
        }
    }
    return base;
}
int execute_entry_point(void *base)
{
    if (!base)
        return 1;
    IMAGE_DOS_HEADER_MINI *dos = (IMAGE_DOS_HEADER_MINI *)base;
    IMAGE_NT_HEADERS64_FULL *nt = (IMAGE_NT_HEADERS64_FULL *)((BYTE *)base + dos->e_lfanew);
    DWORD entryPoint = nt->OptionalHeader.AddressOfEntryPoint;
    if (!g_silent)
        printf("\n[EXECUTE] Jumping to entry point (0x%p)...\n", (BYTE *)base + entryPoint);
    typedef int (*EntryPoint)();
    EntryPoint entry = (EntryPoint)((BYTE *)base + entryPoint);
    return entry();
}
int execute_from_memory(unsigned char *pe_data, size_t pe_size)
{
    void *base = manual_map_pe(pe_data, pe_size);
    if (!base)
        return 1;
    g_main_base = base;

    int exitCode = execute_entry_point(base);

    if (!g_silent)
        printf("[SUCCESS] Process exited with code: %d\n", exitCode);

    return 0;
}
FARPROC get_proc_address_from_memory(void *base, LPCSTR name)
{
    IMAGE_DOS_HEADER_MINI *dos = (IMAGE_DOS_HEADER_MINI *)base;
    IMAGE_NT_HEADERS64_FULL *nt = (IMAGE_NT_HEADERS64_FULL *)((BYTE *)base + dos->e_lfanew);
    IMAGE_DATA_DIRECTORY *exportDir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (exportDir->Size == 0)
        return NULL;
    IMAGE_EXPORT_DIRECTORY *exports = (IMAGE_EXPORT_DIRECTORY *)((BYTE *)base + exportDir->VirtualAddress);
    DWORD *names = (DWORD *)((BYTE *)base + exports->AddressOfNames);
    WORD *ordinals = (WORD *)((BYTE *)base + exports->AddressOfNameOrdinals);
    DWORD *functions = (DWORD *)((BYTE *)base + exports->AddressOfFunctions);
    if ((ULONG_PTR)name <= 0xFFFF)
    {
        DWORD ordinal = (DWORD)(ULONG_PTR)name;
        if (ordinal < exports->Base || ordinal >= exports->Base + exports->NumberOfFunctions)
            return NULL;
        return (FARPROC)((BYTE *)base + functions[ordinal - exports->Base]);
    }
    for (DWORD i = 0; i < exports->NumberOfNames; i++)
    {
        char *funcName = (char *)((BYTE *)base + names[i]);
        if (strcmp(funcName, name) == 0)
        {
            return (FARPROC)((BYTE *)base + functions[ordinals[i]]);
        }
    }

    return NULL;
}

// end