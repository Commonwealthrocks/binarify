#ifndef CORE32_H
#define CORE32_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <argon2.h>
#include "common.h"

void derive_key_32(const char *password, unsigned char *salt, unsigned char *key);
int encrypt_and_binaryify_32(const char *input, const char *output, const char *password);
int decrypt_and_run_32(const char *input, const char *password);

#endif
