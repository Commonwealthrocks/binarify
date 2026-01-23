// core64.h
// last updated: 18/01/2026 <d/m/y>
#ifndef CORE64_H
#define CORE64_H
#include "../c_header_files/common.h"
void derive_key(const char *password, unsigned char *salt, unsigned char *key);
int encrypt_and_binaryify(const char *input, const char *output, const char *password);
int decrypt_and_run(const char *input, const char *password);
#endif

// end
