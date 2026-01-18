// cli.c
// last updated: 18/01/2026 <d/m/y>
#include "../c_header_files/cli.h"
void print_usage()
{
    printf("Binf tool\n\n");
    printf("USAGE:\n");
    printf("  binf <input.exe> <output.bxe> [-p password] [-silent]  encrypt n' binaryify\n");
    printf("  binf <input.bxe> [-silent]                             decrypt n' run in RAM\n");
    printf("  binf install cm                                        install context menu\n");
    printf("  binf uninstall cm                                      uninstall context menu\n");
    printf("  binf path                                              add to system PATH\n\n");
    printf("OPTIONS:\n");
    printf("  -p <password>   use custom password (AES-256-GCM + Argon2id)\n");
    printf("                  if omitted, uses default password\n");
    printf("  -silent         hide all output except from executed program\n\n");
    printf("EXAMPLES:\n");
    printf("  binf test.exe test.bxe\n");
    printf("  binf test.exe test.bxe -p SuperSecretPasswordOrSomething\n");
    printf("  binf test.bxe -silent\n");
}
void get_password_secure(char *buffer, size_t max_len)
{
    size_t i = 0;
    int ch;
    while (i < max_len - 1)
    {
        ch = _getch();
        if (ch == '\r' || ch == '\n')
        {
            break;
        }
        else if (ch == '\b' && i > 0)
        {
            i--;
            continue;
        }
        else if (ch == '\b')
        {
            continue;
        }
        buffer[i++] = ch;
    }
    buffer[i] = '\0';
}

// end