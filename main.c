#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 1024
#define MAX_KEY_SIZE 256


void xorCrypt(uint8_t *data, size_t data_len, const uint8_t *key, size_t key_len) {
    for (size_t i = 0; i < data_len; i++) {
        data[i] ^= key[i % key_len];
    }
}


void dataToHex(const uint8_t *input, size_t input_len, char *output) {
    for (size_t i = 0; i < input_len; i++) {
        sprintf(output + i * 2, "%02X", input[i]);
    }
    output[input_len * 2] = '\0';
}


size_t hexToData(const char *input, uint8_t *output) {
    size_t len = strlen(input);
    size_t out_len = 0;
    for (size_t i = 0; i < len; i += 2) {
        sscanf(input + i, "%2hhx", &output[out_len++]);
    }
    return out_len;
}


void safeInput(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        } else {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }
}

int main() {
    uint8_t data[MAX_INPUT_SIZE];
    uint8_t key[MAX_KEY_SIZE];
    char hexOutput[MAX_INPUT_SIZE * 2 + 1];
    char input[MAX_INPUT_SIZE * 2 + 1];
    size_t data_len, key_len;

    printf("Choose the mode:\n1. Encrypting strings\n2. Decripting HEX\n3. Number encryption\n");
    int choice;
    scanf("%d", &choice);
    getchar(); 

    switch (choice) {
        case 1:
            printf("enter string for encryption: ");
            safeInput(input, sizeof(input));
            data_len = strlen(input);
            memcpy(data, input, data_len);

            printf("enter the key for the encryption: ");
            safeInput((char*)key, sizeof(key));
            key_len = strlen((char*)key);

            xorCrypt(data, data_len, key, key_len);
            dataToHex(data, data_len, hexOutput);
            printf("encrypted string in HEX: %s\n", hexOutput);
            break;

        case 2:
            printf("enter HEX for decryption: ");
            safeInput(input, sizeof(input));
            data_len = hexToData(input, data);

            printf("enter the key for the decryption: ");
            safeInput((char*)key, sizeof(key));
            key_len = strlen((char*)key);

            xorCrypt(data, data_len, key, key_len);
            data[data_len] = '\0';
            printf("decrypted strig: %s\n", data);
            break;

        case 3:
            printf("encryption for the number: ");
            uint32_t num, numKey;
            scanf("%u", &num);
            printf("enter the the key th encrypt the number: ");
            scanf("%u", &numKey);

            uint32_t encryptedNum = num ^ numKey;
            printf("encrypted number in HEX: %08X\n", encryptedNum);
            printf("decrypted number: %u\n", encryptedNum ^ numKey);
            break;

        default:
            printf("error wrong chose\n");
            return 1;
    }

    return 0;
}