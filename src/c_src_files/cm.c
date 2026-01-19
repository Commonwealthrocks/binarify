// cm.c
// last updated: 19/01/2026 <d/m/y>
#include "../c_header_files/cm.h"
#include "../c_header_files/cm.h"
#include <shlobj.h>
int install_registry_key(HKEY root, const char *key_path, const char *value_name, const char *value_data)
{
    HKEY hKey;
    if (RegCreateKeyExA(root, key_path, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        return 0;
    }
    if (RegSetValueExA(hKey, value_name, 0, REG_SZ, (BYTE *)value_data, strlen(value_data) + 1) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return 0;
    }
    RegCloseKey(hKey);
    return 1;
}
int install_context_menu()
{
    printf("Installing .bxe file association...\n");

    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    HKEY root = HKEY_CLASSES_ROOT;
    char cmd[512];
    snprintf(cmd, 512, "\"%s\" \"%%1\"", exe_path);
    int success = 1;
    if (!install_registry_key(root, ".bxe", NULL, "bxe_file"))
        success = 0;
    if (!install_registry_key(root, "bxe_file", NULL, "Binf Executable"))
        success = 0;
    if (!install_registry_key(root, "bxe_file\\DefaultIcon", NULL, exe_path))
        success = 0; // rough icon
    if (!install_registry_key(root, "bxe_file\\shell\\open\\command", NULL, cmd))
        success = 0;
    if (!success)
    {
        printf("[WARN] Failed to write to HKEY_CLASSES_ROOT (Access Denied?). Retrying with HKEY_CURRENT_USER...\n");
        root = HKEY_CURRENT_USER;
        success = 1;
        if (!install_registry_key(root, "Software\\Classes\\.bxe", NULL, "bxe_file"))
            success = 0;
        if (!install_registry_key(root, "Software\\Classes\\bxe_file", NULL, "Binf Executable"))
            success = 0;
        if (!install_registry_key(root, "Software\\Classes\\bxe_file\\DefaultIcon", NULL, exe_path))
            success = 0;
        if (!install_registry_key(root, "Software\\Classes\\bxe_file\\shell\\open\\command", NULL, cmd))
            success = 0;
    }
    if (success)
    {
        printf("[SUCCESS] Installation complete.\n");
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
        return 0;
    }
    else
    {
        printf("[ERROR] Failed to install association. Try running as Administrator.\n");
        return 1;
    }
}
int uninstall_context_menu()
{
    printf("Uninstalling .bxe file association...\n");
    RegDeleteTreeA(HKEY_CLASSES_ROOT, ".bxe");
    RegDeleteTreeA(HKEY_CLASSES_ROOT, "bxe_file");
    RegDeleteTreeA(HKEY_CURRENT_USER, "Software\\Classes\\.bxe");
    RegDeleteTreeA(HKEY_CURRENT_USER, "Software\\Classes\\bxe_file");
    printf("[SUCCESS] Uninstallation complete.\n");
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    return 0;
}
int add_to_path()
{
    char exe_path[MAX_PATH];
    char exe_dir[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    strcpy(exe_dir, exe_path);
    char *last_slash = strrchr(exe_dir, '\\');
    if (last_slash)
        *last_slash = '\0';
    printf("Adding '%s' to User PATH...\n", exe_dir);
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS)
    {
        printf("[ERROR] Cannot open Environment registry key.\n");
        return 1;
    }
    DWORD size = 0;
    if (RegQueryValueExA(hKey, "Path", NULL, NULL, NULL, &size) != ERROR_SUCCESS)
    {
        size = 1; // dummy
    }
    char *path_val = malloc(size + strlen(exe_dir) + 2); // + ';' + null
    path_val[0] = '\0';
    DWORD type = REG_SZ;
    if (RegQueryValueExA(hKey, "Path", NULL, &type, (BYTE *)path_val, &size) == ERROR_SUCCESS)
    {
        if (strstr(path_val, exe_dir))
        {
            printf("[INFO] Already in PATH.\n");
            free(path_val);
            RegCloseKey(hKey);
            return 0;
        }
    }
    if (strlen(path_val) > 0 && path_val[strlen(path_val) - 1] != ';')
    {
        strcat(path_val, ";");
    }
    strcat(path_val, exe_dir);
    if (RegSetValueExA(hKey, "Path", 0, type, (BYTE *)path_val, strlen(path_val) + 1) != ERROR_SUCCESS)
    {
        printf("[ERROR] Failed to write PATH.\n");
        free(path_val);
        RegCloseKey(hKey);
        return 1;
    }
    free(path_val);
    RegCloseKey(hKey);
    printf("[SUCCESS] Added to PATH; you may need to restart your terminal.\n");
    SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", SMTO_ABORTIFHUNG, 5000, NULL);
    return 0;
}

// end