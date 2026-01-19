// vfs_mng.c
// last updated: 19/01/2026 <d/m/y>
#include "../c_header_files/vfs_mng.h"
#include "../c_header_files/ram_mng.h"
#include "../c_header_files/outs.h"
VirtualFile g_vfs[MAX_VIRTUAL_FILES];
VirtualHandle g_vhandles[MAX_VIRTUAL_HANDLES];
typedef HANDLE(WINAPI *P_CreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef HANDLE(WINAPI *P_CreateFileW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef BOOL(WINAPI *P_ReadFile)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL(WINAPI *P_CloseHandle)(HANDLE);
typedef DWORD(WINAPI *P_GetFileSize)(HANDLE, LPDWORD);
typedef DWORD(WINAPI *P_SetFilePointer)(HANDLE, LONG, PLONG, DWORD);
typedef HMODULE(WINAPI *P_LoadLibraryA)(LPCSTR);
typedef HMODULE(WINAPI *P_LoadLibraryW)(LPCWSTR);
typedef FARPROC(WINAPI *P_GetProcAddress)(HMODULE, LPCSTR);
typedef DWORD(WINAPI *P_GetFileAttributesA)(LPCSTR);
typedef DWORD(WINAPI *P_GetFileAttributesW)(LPCWSTR);
typedef HMODULE(WINAPI *P_GetModuleHandleA)(LPCSTR);
typedef HMODULE(WINAPI *P_GetModuleHandleW)(LPCWSTR);
typedef LPSTR(WINAPI *P_GetCommandLineA)(void);
typedef LPWSTR(WINAPI *P_GetCommandLineW)(void);
static P_CreateFileA g_origCreateFileA = NULL;
static P_CreateFileW g_origCreateFileW = NULL;
static P_ReadFile g_origReadFile = NULL;
static P_CloseHandle g_origCloseHandle = NULL;
static P_GetFileSize g_origGetFileSize = NULL;
static P_SetFilePointer g_origSetFilePointer = NULL;
static P_LoadLibraryA g_origLoadLibraryA = NULL;
static P_LoadLibraryW g_origLoadLibraryW = NULL;
static P_GetProcAddress g_origGetProcAddress = NULL;
static P_GetFileAttributesA g_origGetFileAttributesA = NULL;
static P_GetFileAttributesW g_origGetFileAttributesW = NULL;
static P_GetModuleHandleA g_origGetModuleHandleA = NULL;
static P_GetModuleHandleW g_origGetModuleHandleW = NULL;
static P_GetCommandLineA g_origGetCommandLineA = NULL;
static P_GetCommandLineW g_origGetCommandLineW = NULL;
static char *g_fake_cmdline_a = NULL;
static wchar_t *g_fake_cmdline_w = NULL;
void set_fake_cmdline(const char *cmd)
{
    if (g_fake_cmdline_a)
        free(g_fake_cmdline_a);
    if (g_fake_cmdline_w)
        free(g_fake_cmdline_w);
    if (cmd)
    {
        g_fake_cmdline_a = _strdup(cmd);
        int len = MultiByteToWideChar(CP_ACP, 0, cmd, -1, NULL, 0);
        g_fake_cmdline_w = malloc(len * sizeof(wchar_t));
        MultiByteToWideChar(CP_ACP, 0, cmd, -1, g_fake_cmdline_w, len);
    }
    else
    {
        g_fake_cmdline_a = NULL;
        g_fake_cmdline_w = NULL;
    }
}
LPSTR WINAPI Hooked_GetCommandLineA(void)
{
    if (g_fake_cmdline_a)
        return g_fake_cmdline_a;
    if (g_origGetCommandLineA)
        return g_origGetCommandLineA();
    return GetCommandLineA();
}
LPWSTR WINAPI Hooked_GetCommandLineW(void)
{
    if (g_fake_cmdline_w)
        return g_fake_cmdline_w;
    if (g_origGetCommandLineW)
        return g_origGetCommandLineW();
    return GetCommandLineW();
}
int vfs_find(const char *path)
{
    for (int i = 0; i < MAX_VIRTUAL_FILES; i++)
    {
        if (g_vfs[i].used && str_icmp(g_vfs[i].path, path))
        {
            return i;
        }
    }
    return -1;
}
int vfs_add(const char *path, unsigned char *data, DWORD size)
{
    for (int i = 0; i < MAX_VIRTUAL_FILES; i++)
    {
        if (!g_vfs[i].used)
        {
            strncpy(g_vfs[i].path, path, MAX_PATH - 1);
            g_vfs[i].data = data;
            g_vfs[i].size = size;
            g_vfs[i].used = 1;
            return i;
        }
    }
    return -1;
}
int vfs_lazy_load(const char *path)
{
    int idx = vfs_find(path);
    if (idx != -1)
        return idx;
    FILE *f = fopen(path, "rb");
    if (!f)
        return -1;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *data = malloc(size);
    if (!data)
    {
        fclose(f);
        return -1;
    }
    fread(data, 1, size, f);
    fclose(f);
    if (!g_silent)
        printf("[VFS] Lazy loaded '%s' into RAM (%ld bytes)\n", path, size);

    return vfs_add(path, data, size);
}
HANDLE WINAPI Hooked_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    int idx = vfs_lazy_load(lpFileName);
    if (idx != -1)
    {
        for (int i = 0; i < MAX_VIRTUAL_HANDLES; i++)
        {
            if (!g_vhandles[i].used)
            {
                g_vhandles[i].file_index = idx;
                g_vhandles[i].position = 0;
                g_vhandles[i].used = 1;
                return (HANDLE)(uintptr_t)(VIRTUAL_HANDLE_MAGIC | i);
            }
        }
    }
    if (g_origCreateFileA)
        return g_origCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    return CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}
HANDLE WINAPI Hooked_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    char name[MAX_PATH];
    wcstombs(name, lpFileName, MAX_PATH);
    return Hooked_CreateFileA(name, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}
BOOL WINAPI Hooked_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    uintptr_t handle_val = (uintptr_t)hFile;
    if ((handle_val & 0xFFF00000) == VIRTUAL_HANDLE_MAGIC)
    {
        int hIdx = handle_val & 0xFFFF;
        if (hIdx < MAX_VIRTUAL_HANDLES && g_vhandles[hIdx].used)
        {
            VirtualHandle *vh = &g_vhandles[hIdx];
            VirtualFile *vf = &g_vfs[vh->file_index];
            DWORD available = vf->size - vh->position;
            DWORD to_read = (nNumberOfBytesToRead < available) ? nNumberOfBytesToRead : available;
            memcpy(lpBuffer, vf->data + vh->position, to_read);
            vh->position += to_read;
            if (lpNumberOfBytesRead)
                *lpNumberOfBytesRead = to_read;
            return TRUE;
        }
    }
    if (g_origReadFile)
        return g_origReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}
BOOL WINAPI Hooked_CloseHandle(HANDLE hObject)
{
    uintptr_t handle_val = (uintptr_t)hObject;
    if ((handle_val & 0xFFF00000) == VIRTUAL_HANDLE_MAGIC)
    {
        int hIdx = handle_val & 0xFFFF;
        if (hIdx < MAX_VIRTUAL_HANDLES)
        {
            g_vhandles[hIdx].used = 0;
            return TRUE;
        }
    }
    if (g_origCloseHandle)
        return g_origCloseHandle(hObject);
    return CloseHandle(hObject);
}
DWORD WINAPI Hooked_GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh)
{
    uintptr_t handle_val = (uintptr_t)hFile;
    if ((handle_val & 0xFFF00000) == VIRTUAL_HANDLE_MAGIC)
    {
        int hIdx = handle_val & 0xFFFF;
        if (hIdx < MAX_VIRTUAL_HANDLES && g_vhandles[hIdx].used)
        {
            if (lpFileSizeHigh)
                *lpFileSizeHigh = 0;
            return g_vfs[g_vhandles[hIdx].file_index].size;
        }
    }
    if (g_origGetFileSize)
        return g_origGetFileSize(hFile, lpFileSizeHigh);
    return GetFileSize(hFile, lpFileSizeHigh);
}
DWORD WINAPI Hooked_SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
    uintptr_t handle_val = (uintptr_t)hFile;
    if ((handle_val & 0xFFF00000) == VIRTUAL_HANDLE_MAGIC)
    {
        int hIdx = handle_val & 0xFFFF;
        if (hIdx < MAX_VIRTUAL_HANDLES && g_vhandles[hIdx].used)
        {
            VirtualHandle *vh = &g_vhandles[hIdx];
            DWORD total_size = g_vfs[vh->file_index].size;
            LONG new_pos = 0;
            if (dwMoveMethod == FILE_BEGIN)
                new_pos = lDistanceToMove;
            else if (dwMoveMethod == FILE_CURRENT)
                new_pos = vh->position + lDistanceToMove;
            else if (dwMoveMethod == FILE_END)
                new_pos = total_size + lDistanceToMove;
            if (new_pos < 0)
                new_pos = 0;
            if (new_pos > (LONG)total_size)
                new_pos = total_size;
            vh->position = new_pos;
            return new_pos;
        }
    }
    if (g_origSetFilePointer)
        return g_origSetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
    return SetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
}
HMODULE WINAPI Hooked_LoadLibraryA(LPCSTR lpLibFileName)
{
    if (!g_silent)
        printf("[VFS] Hooked_LoadLibraryA: %s\n", lpLibFileName);
    int idx = vfs_lazy_load(lpLibFileName);
    if (idx != -1)
    {
        if (!g_silent)
            printf("[VFS] Manually mapping %s from RAM...\n", lpLibFileName);
        void *base = manual_map_pe(g_vfs[idx].data, g_vfs[idx].size);
        IMAGE_DOS_HEADER_MINI *dos = (IMAGE_DOS_HEADER_MINI *)base;
        IMAGE_NT_HEADERS64_FULL *nt = (IMAGE_NT_HEADERS64_FULL *)((BYTE *)base + dos->e_lfanew);
        if (nt->OptionalHeader.AddressOfEntryPoint != 0)
        {
            typedef BOOL(WINAPI * DllEntryProc)(HINSTANCE, DWORD, LPVOID);
            DllEntryProc entry = (DllEntryProc)((BYTE *)base + nt->OptionalHeader.AddressOfEntryPoint);
            if (!g_silent)
                printf("[VFS] Calling DllMain for %s\n", lpLibFileName);
            entry((HINSTANCE)base, DLL_PROCESS_ATTACH, NULL);
        }
        return (HMODULE)base;
    }
    if (g_origLoadLibraryA)
        return g_origLoadLibraryA(lpLibFileName);
    return LoadLibraryA(lpLibFileName);
}
DWORD WINAPI Hooked_GetFileAttributesA(LPCSTR lpFileName)
{
    int idx = vfs_find(lpFileName);
    if (idx != -1)
        return FILE_ATTRIBUTE_NORMAL;
    DWORD attrs = INVALID_FILE_ATTRIBUTES;
    if (g_origGetFileAttributesA)
        attrs = g_origGetFileAttributesA(lpFileName);
    else
        attrs = GetFileAttributesA(lpFileName);
    if (attrs != INVALID_FILE_ATTRIBUTES)
        return attrs;
    return INVALID_FILE_ATTRIBUTES;
}
DWORD WINAPI Hooked_GetFileAttributesW(LPCWSTR lpFileName)
{
    char name[MAX_PATH];
    wcstombs(name, lpFileName, MAX_PATH);
    return Hooked_GetFileAttributesA(name);
}
HMODULE WINAPI Hooked_GetModuleHandleA(LPCSTR lpModuleName)
{
    if (lpModuleName == NULL)
    {
        if (g_main_base)
            return (HMODULE)g_main_base;
    }
    if (g_origGetModuleHandleA)
        return g_origGetModuleHandleA(lpModuleName);
    return GetModuleHandleA(lpModuleName);
}
HMODULE WINAPI Hooked_GetModuleHandleW(LPCWSTR lpModuleName)
{
    if (lpModuleName == NULL)
    {
        if (g_main_base)
            return (HMODULE)g_main_base;
    }
    if (g_origGetModuleHandleW)
        return g_origGetModuleHandleW(lpModuleName);
    return GetModuleHandleW(lpModuleName);
}
FARPROC WINAPI Hooked_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    if ((ULONG_PTR)lpProcName > 0xFFFF)
    {
        if (strcmp(lpProcName, "CreateFileA") == 0)
            return (FARPROC)(void *)Hooked_CreateFileA;
        if (strcmp(lpProcName, "CreateFileW") == 0)
            return (FARPROC)(void *)Hooked_CreateFileW;
        if (strcmp(lpProcName, "ReadFile") == 0)
            return (FARPROC)(void *)Hooked_ReadFile;
        if (strcmp(lpProcName, "CloseHandle") == 0)
            return (FARPROC)(void *)Hooked_CloseHandle;
        if (strcmp(lpProcName, "GetFileSize") == 0)
            return (FARPROC)(void *)Hooked_GetFileSize;
        if (strcmp(lpProcName, "SetFilePointer") == 0)
            return (FARPROC)(void *)Hooked_SetFilePointer;
        if (strcmp(lpProcName, "LoadLibraryA") == 0)
            return (FARPROC)(void *)Hooked_LoadLibraryA;
        if (strcmp(lpProcName, "GetProcAddress") == 0)
            return (FARPROC)(void *)Hooked_GetProcAddress;
        if (strcmp(lpProcName, "GetFileAttributesA") == 0)
            return (FARPROC)(void *)Hooked_GetFileAttributesA;
        if (strcmp(lpProcName, "GetFileAttributesW") == 0)
            return (FARPROC)(void *)Hooked_GetFileAttributesW;
        if (strcmp(lpProcName, "GetModuleHandleA") == 0)
            return (FARPROC)(void *)Hooked_GetModuleHandleA;
        if (strcmp(lpProcName, "GetModuleHandleW") == 0)
            return (FARPROC)(void *)Hooked_GetModuleHandleW;
        if (strcmp(lpProcName, "GetCommandLineA") == 0)
            return (FARPROC)(void *)Hooked_GetCommandLineA;
        if (strcmp(lpProcName, "GetCommandLineW") == 0)
            return (FARPROC)(void *)Hooked_GetCommandLineW;
    }
    if (is_mapped_module(hModule))
    {
        return get_proc_address_from_memory((void *)hModule, lpProcName);
    }
    if (g_origGetProcAddress)
        return g_origGetProcAddress(hModule, lpProcName);
    return GetProcAddress(hModule, lpProcName);
}
void init_hooks()
{
    g_origCreateFileA = (P_CreateFileA)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "CreateFileA");
    g_origCreateFileW = (P_CreateFileW)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "CreateFileW");
    g_origReadFile = (P_ReadFile)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "ReadFile");
    g_origCloseHandle = (P_CloseHandle)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "CloseHandle");
    g_origGetFileSize = (P_GetFileSize)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetFileSize");
    g_origSetFilePointer = (P_SetFilePointer)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "SetFilePointer");
    g_origLoadLibraryA = (P_LoadLibraryA)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    g_origLoadLibraryW = (P_LoadLibraryW)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryW");
    g_origGetProcAddress = (P_GetProcAddress)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetProcAddress");
    g_origGetFileAttributesA = (P_GetFileAttributesA)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetFileAttributesA");
    g_origGetFileAttributesW = (P_GetFileAttributesW)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetFileAttributesW");
    g_origGetModuleHandleA = (P_GetModuleHandleA)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetModuleHandleA");
    g_origGetModuleHandleW = (P_GetModuleHandleW)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetModuleHandleW");
    g_origGetCommandLineA = (P_GetCommandLineA)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetCommandLineA");
    g_origGetCommandLineW = (P_GetCommandLineW)(void *)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetCommandLineW");
}

// end