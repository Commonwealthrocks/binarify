// core64.c
// last updated: 19/01/2026 <d/m/y>
#include "../c_header_files/core64.h"
#include "../c_header_files/_compiler_based.h"
#include "../c_header_files/outs.h"
#include "../c_header_files/ram_mng.h"
#include "../c_header_files/vfs_mng.h"
void derive_key(const char *password, unsigned char *salt, unsigned char *key)
{
    if (!password || strlen(password) == 0)
    {
        if (!g_silent)
            printf("[ERROR] Password is null or empty.\n");
        exit(1);
    }
    int result = argon2id_hash_raw(
        ARGON2_TIME_COST, ARGON2_MEMORY_COST, ARGON2_PARALLELISM,
        password, strlen(password),
        salt, SALT_SIZE,
        key, AES_KEY_SIZE);
    if (result != ARGON2_OK)
    {
        if (!g_silent)
            printf("[ERROR] Argon2id failed: %s\n", argon2_error_message(result));
        exit(1);
    }
}
int encrypt_and_binaryify(const char *input, const char *output, const char *password)
{
    FILE *in = fopen(input, "rb");
    if (!in)
    {
        if (!g_silent)
            printf("[ERROR] Cannot open '%s'\n", input);
        return 1;
    }
    fseek(in, 0, SEEK_END);
    long file_size = ftell(in);
    fseek(in, 0, SEEK_SET);
    unsigned char *plaintext = malloc(file_size);
    fread(plaintext, 1, file_size, in);
    fclose(in);
    unsigned char salt[SALT_SIZE];
    unsigned char iv[IV_SIZE];
    RAND_bytes(salt, SALT_SIZE);
    RAND_bytes(iv, IV_SIZE);
    unsigned char key[AES_KEY_SIZE];
    derive_key(password, salt, key);
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv);
    unsigned char *ciphertext = malloc(file_size + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    int len, ciphertext_len;
    EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, file_size);
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;
    unsigned char tag[AES_TAG_SIZE];
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, AES_TAG_SIZE, tag);
    EVP_CIPHER_CTX_free(ctx);
    secure_zero(key, AES_KEY_SIZE);
    FILE *out = fopen(output, "w");
    if (!out)
    {
        if (!g_silent)
            printf("[ERROR] Cannot create '%s'\n", output);
        secure_zero(plaintext, file_size);
        free(plaintext);
        free(ciphertext);
        return 1;
    }
    if (!g_silent)
    {
        printf("Encrypting with AES-256-GCM + Argon2id...\n");
        printf("  - Time cost: %d iterations\n", ARGON2_TIME_COST);
        printf("  - Memory cost: %d KB (%d MB)\n", ARGON2_MEMORY_COST, ARGON2_MEMORY_COST / 1024);
        printf("  - Parallelism: %d threads\n\n", ARGON2_PARALLELISM);
        printf("Binaryifying '%s' -> '%s'...\n", input, output);
    }
    unsigned char size_bytes[4];
    size_bytes[0] = (file_size >> 24) & 0xFF;
    size_bytes[1] = (file_size >> 16) & 0xFF;
    size_bytes[2] = (file_size >> 8) & 0xFF;
    size_bytes[3] = file_size & 0xFF;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 7; j >= 0; j--)
        {
            fputc((size_bytes[i] & (1 << j)) ? '1' : '0', out);
        }
    }
    for (int i = 0; i < SALT_SIZE; i++)
    {
        for (int j = 7; j >= 0; j--)
        {
            fputc((salt[i] & (1 << j)) ? '1' : '0', out);
        }
    }
    for (int i = 0; i < IV_SIZE; i++)
    {
        for (int j = 7; j >= 0; j--)
        {
            fputc((iv[i] & (1 << j)) ? '1' : '0', out);
        }
    }
    for (int i = 0; i < AES_TAG_SIZE; i++)
    {
        for (int j = 7; j >= 0; j--)
        {
            fputc((tag[i] & (1 << j)) ? '1' : '0', out);
        }
    }
    for (int i = 0; i < ciphertext_len; i++)
    {
        for (int j = 7; j >= 0; j--)
        {
            fputc((ciphertext[i] & (1 << j)) ? '1' : '0', out);
        }
        if (!g_silent && (i + 1) % 1000 == 0)
        {
            printf("\r[PROGRESS] %d / %d bytes...", i + 1, ciphertext_len);
            fflush(stdout);
        }
    }
    if (!g_silent)
    {
        printf("\rFinished encrypting and converting %ld bytes.\n", file_size);
        long total_binary_size = (4 + SALT_SIZE + IV_SIZE + AES_TAG_SIZE + ciphertext_len) * 8;
        printf("Original size: %ld bytes\n", file_size);
        printf("Encrypted size: %d bytes\n", ciphertext_len);
        printf("Output .bxe size: %ld bytes (8x inflation lmao)\n", total_binary_size);
    }
    fclose(out);
    secure_zero(plaintext, file_size);
    secure_zero(ciphertext, ciphertext_len);
    free(plaintext);
    free(ciphertext);
    return 0;
}
int decrypt_and_run(const char *input, const char *password)
{
    FILE *in = fopen(input, "r");
    if (!in)
    {
        if (!g_silent)
            printf("[ERROR] Cannot open '%s'\n", input);
        return 1;
    }
    if (!g_silent)
        printf("Decrypting '%s'...\n", input);
    unsigned char size_bytes[4];
    for (int i = 0; i < 4; i++)
    {
        unsigned char byte = 0;
        for (int j = 7; j >= 0; j--)
        {
            int c = fgetc(in);
            if (c == '1')
                byte |= (1 << j);
        }
        size_bytes[i] = byte;
    }
    long original_size = (size_bytes[0] << 24) | (size_bytes[1] << 16) | (size_bytes[2] << 8) | size_bytes[3];

    if (!g_silent)
        printf("Original file size: %ld bytes\n", original_size);
    unsigned char salt[SALT_SIZE];
    unsigned char iv[IV_SIZE];
    unsigned char tag[AES_TAG_SIZE];
    for (int i = 0; i < SALT_SIZE; i++)
    {
        unsigned char byte = 0;
        for (int j = 7; j >= 0; j--)
        {
            int c = fgetc(in);
            if (c == '1')
                byte |= (1 << j);
        }
        salt[i] = byte;
    }
    for (int i = 0; i < IV_SIZE; i++)
    {
        unsigned char byte = 0;
        for (int j = 7; j >= 0; j--)
        {
            int c = fgetc(in);
            if (c == '1')
                byte |= (1 << j);
        }
        iv[i] = byte;
    }
    for (int i = 0; i < AES_TAG_SIZE; i++)
    {
        unsigned char byte = 0;
        for (int j = 7; j >= 0; j--)
        {
            int c = fgetc(in);
            if (c == '1')
                byte |= (1 << j);
        }
        tag[i] = byte;
    }
    fseek(in, 0, SEEK_END);
    long total_bits = ftell(in);
    long data_bits = total_bits - ((4 + SALT_SIZE + IV_SIZE + AES_TAG_SIZE) * 8);
    long data_size = data_bits / 8;
    fseek(in, (4 + SALT_SIZE + IV_SIZE + AES_TAG_SIZE) * 8, SEEK_SET);
    unsigned char *ciphertext = malloc(data_size);
    for (long i = 0; i < data_size; i++)
    {
        unsigned char byte = 0;
        for (int j = 7; j >= 0; j--)
        {
            int c = fgetc(in);
            if (c == '1')
                byte |= (1 << j);
        }
        ciphertext[i] = byte;
        if (!g_silent && (i + 1) % 1000 == 0)
        {
            printf("\rReading: %ld / %ld bytes...", i + 1, data_size);
            fflush(stdout);
        }
    }
    if (!g_silent)
        printf("\rRead: %ld bytes                    \n", data_size);
    fclose(in);
    unsigned char key[AES_KEY_SIZE];
    derive_key(password, salt, key);
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv);
    unsigned char *plaintext = malloc(data_size);
    int len, plaintext_len;
    EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, data_size);
    plaintext_len = len;
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, AES_TAG_SIZE, tag);
    int ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    EVP_CIPHER_CTX_free(ctx);
    secure_zero(key, AES_KEY_SIZE);
    if (ret <= 0)
    {
        if (!g_silent)
            printf("\n[ERROR] Decryption failed; wrong password or corrupted / invalid file.\n");
        secure_zero(ciphertext, data_size);
        secure_zero(plaintext, data_size);
        free(ciphertext);
        free(plaintext);
        return 1;
    }
    plaintext_len += len;
    if (!g_silent)
        printf("[SUCCESS] Decryption successful (%d bytes)\n", plaintext_len);
    secure_zero(ciphertext, data_size);
    free(ciphertext);
    set_fake_cmdline("payload.exe");
    int result = execute_from_memory(plaintext, plaintext_len);
    secure_zero(plaintext, plaintext_len);
    free(plaintext);
    return result;
}

// end