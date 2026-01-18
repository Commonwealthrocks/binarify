// binf.c
// last updated: 18/01/2026 <d/m/y>
#include "../c_header_files/common.h"
#include "../c_header_files/cli.h"
#include "../c_header_files/cm.h"
#include "../c_header_files/core64.h"
#include "../c_header_files/vfs_mng.h"
#include "../c_header_files/outs.h"
#include "../c_header_files/_compiler_based.h"
int main(int argc, char *argv[])
{
    init_hooks();
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-silent") == 0)
        {
            g_silent = 1;
            break;
        }
    }
    if (argc < 2)
    {
        print_usage();
        return 1;
    }
    if (argc >= 3 && strcmp(argv[1], "install") == 0 && strcmp(argv[2], "cm") == 0)
    {
        return install_context_menu();
    }
    if (argc >= 3 && strcmp(argv[1], "uninstall") == 0 && strcmp(argv[2], "cm") == 0)
    {
        return uninstall_context_menu();
    }
    if (argc >= 2 && (strcmp(argv[1], "path") == 0 || strcmp(argv[1], "PATH") == 0))
    {
        return add_to_path();
    }
    if (strstr(argv[1], ".bxe"))
    {
        const char *password = NULL;
        int has_custom_password = 0;
        for (int i = 2; i < argc; i++)
        {
            if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
            {
                password = argv[i + 1];
                has_custom_password = 1;
                break;
            }
        }
        if (!has_custom_password)
        {
            password = DEFAULT_PASSWORD;
            if (!g_silent)
                printf("Using default password.\n");
        }
        else
        {
            if (!g_silent)
                printf("Using custom password.\n");
        }
        return decrypt_and_run(argv[1], password);
    }
    if (argc >= 3 && !strstr(argv[1], ".bxe"))
    {
        const char *password = NULL;
        for (int i = 3; i < argc; i++)
        {
            if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
            {
                password = argv[i + 1];
                break;
            }
        }
        if (!password)
        {
            password = DEFAULT_PASSWORD;
            if (!g_silent)
                printf("Using default password.\n");
        }
        else
        {
            if (!g_silent)
                printf("Using custom password.\n");
        }
        int result = encrypt_and_binaryify(argv[1], argv[2], password);
        if (password != DEFAULT_PASSWORD)
        {
            for (int i = 3; i < argc; i++)
            {
                if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
                {
                    secure_zero(argv[i + 1], strlen(argv[i + 1]));
                    break;
                }
            }
        }
        return result;
    }
    print_usage();
    return 1;
}

// end