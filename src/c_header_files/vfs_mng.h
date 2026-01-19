// vfs_mng.h
// last updated: 19/01/2026 <d/m/y>
#ifndef VFS_MNG_H
#define VFS_MNG_H
#include "../c_header_files/common.h"
#define MAX_VIRTUAL_FILES 128
#define MAX_VIRTUAL_HANDLES 256
#define VIRTUAL_HANDLE_MAGIC 0xB1300000
typedef struct
{
    char path[MAX_PATH];
    unsigned char *data;
    DWORD size;
    int used;
} VirtualFile;
typedef struct
{
    int file_index;
    DWORD position;
    int used;
} VirtualHandle;
extern VirtualFile g_vfs[MAX_VIRTUAL_FILES];
extern VirtualHandle g_vhandles[MAX_VIRTUAL_HANDLES];
int vfs_find(const char *path);
int vfs_add(const char *path, unsigned char *data, DWORD size);
int vfs_lazy_load(const char *path);
HANDLE WINAPI Hooked_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
HANDLE WINAPI Hooked_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
BOOL WINAPI Hooked_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
BOOL WINAPI Hooked_CloseHandle(HANDLE hObject);
DWORD WINAPI Hooked_GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
DWORD WINAPI Hooked_SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
HMODULE WINAPI Hooked_LoadLibraryA(LPCSTR lpLibFileName);
DWORD WINAPI Hooked_GetFileAttributesA(LPCSTR lpFileName);
DWORD WINAPI Hooked_GetFileAttributesW(LPCWSTR lpFileName);
HMODULE WINAPI Hooked_GetModuleHandleA(LPCSTR lpModuleName);
HMODULE WINAPI Hooked_GetModuleHandleW(LPCWSTR lpModuleName);
FARPROC WINAPI Hooked_GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
void init_hooks();
void set_fake_cmdline(const char *cmd);
#endif

// end
