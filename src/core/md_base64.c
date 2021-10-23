#include "md_base64.h"

#include "md_log.h"
#include <stdint.h>

#ifndef NULL
#define NULL 0
#endif

// Base implementation from here: https://www.mycplus.com/source-code/c-source-code/base64-encode-decode/
// Modified interface to avoid malloc
// Modified decode table to avoid malloc and also check for invalid characters

static char encoding_table[64] = { 
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'};

static char decoding_table[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1, -1,  0, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, 62, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1,  0, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, 63,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1 };

static inline char decode_character(char c) {
    return decoding_table[c & 127];
}

static int mod_table[] = {0, 2, 1};

// Gives the length in bytes of the encoded string which is required to be the minimum buffer size supplied as 'output' into base64_encode
int md_base64_encode_size_in_bytes(int input_length) {
    return 4 * ((input_length + 2) / 3);
}

// 
int md_base64_encode(char* output, const void* input, int input_length) {
    if (output == NULL) {
        md_print(MD_LOG_TYPE_ERROR, "Supplied output buffer was NULL");
        return 0;
    }

    const char* in_bytes = (const char*)input;

    for (int i =0, j =0; i < input_length;) {
        uint32_t a = i < input_length ? in_bytes[i++] : 0;
        uint32_t b = i < input_length ? in_bytes[i++] : 0;
        uint32_t c = i < input_length ? in_bytes[i++] : 0;

        uint32_t triple = (a << 16) + (b << 8) + c;

        output[j++] = encoding_table[(triple >> 18) &0x3F];
        output[j++] = encoding_table[(triple >> 12) &0x3F];
        output[j++] = encoding_table[(triple >>  6) &0x3F];
        output[j++] = encoding_table[(triple >>  0) &0x3F];
    }

    int output_length = md_base64_encode_size_in_bytes(input_length);

    // Fill up with empty
    for (int i = 0; i < mod_table[input_length % 3]; i++)
        output[output_length - 1 - i] = '=';

    return output_length;
}

// Gives the length in bytes of the encoded string which is required to be the minimum buffer size supplied as 'output' into base64_decode
int md_base64_decode_size_in_bytes(int input_length) {
    return input_length / 4 * 3;
}

int md_base64_decode(void* output, const char *input, int input_length) {
    if (output == NULL) {
        md_print(MD_LOG_TYPE_ERROR, "Supplied output buffer was NULL");
        return 0;
    }

    if (input_length % 4 != 0) {
        md_print(MD_LOG_TYPE_ERROR, "Supplied input length was not a multiple of 4");
        return 0;
    }

    int output_length = md_base64_decode_size_in_bytes(input_length);
    if (input[input_length - 1] == '=' || input[input_length - 1] == '.') output_length--;
    if (input[input_length - 2] == '=' || input[input_length - 2] == '.') output_length--;

    char* out_bytes = (char*)output;

    for (int i = 0, j = 0; i < input_length;) {
        char a = decode_character(input[i++]);
        char b = decode_character(input[i++]);
        char c = decode_character(input[i++]);
        char d = decode_character(input[i++]);
        if (a == -1 || b == -1 || c == -1 || d == -1) {
            md_print(MD_LOG_TYPE_ERROR, "Encountered invalid character in sequence.");
            return 0;
        }
        uint32_t triple = ((uint32_t)a << 18) + ((uint32_t)b << 12) + ((uint32_t)c << 6) + (uint32_t)d;
        if (j < output_length) out_bytes[j++] = (triple >> 16) & 0xFF;
        if (j < output_length) out_bytes[j++] = (triple >>  8) & 0xFF;
        if (j < output_length) out_bytes[j++] = (triple >>  0) & 0xFF;
    }

    return output_length;
}