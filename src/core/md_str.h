#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "md_common.h"  // ASSERT, MIN, MAX

#ifdef __cplusplus
#define DEF_VAL(x) = x
#else
#define DEF_VAL(x)
#endif

struct md_allocator_i;
struct md_file_o;

typedef struct str_t {
    const char* ptr;
    int64_t     len;

#ifdef __cplusplus
    constexpr char operator[](int64_t idx) noexcept       { return ptr[idx]; }
    constexpr char operator[](int64_t idx) const noexcept { return ptr[idx]; }

    constexpr const char* beg() noexcept { return ptr; }
    constexpr const char* end() noexcept { return ptr + len; }
    constexpr operator bool() noexcept { return ptr != nullptr; }
#endif
} str_t;

#ifdef __cplusplus
#define MAKE_STR(cstr) {(cstr""), (sizeof(cstr)-1)}
#else
#define MAKE_STR(cstr) (str_t){(cstr ""), (sizeof(cstr)-1)}
#endif

#ifdef __cplusplus
extern "C" {
#endif

str_t str_from_cstr(const char* cstr);

// Only for ASCII character set
static inline int  char_to_digit(int c) { return c - '0'; }
static inline int  to_lower(int c)      { return ('A' <= c && c <= 'Z') ? c+32 : c; }
static inline int  to_upper(int c)      { return ('a' <= c && c <= 'z') ? c-32 : c; }
static inline bool is_digit(int c)      { return '0' <= c && c <= '9'; }
static inline bool is_alpha(int c)      { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }
static inline bool is_whitespace(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
static inline bool is_symbol(int c) {
    return (32 < c && c < 48) || (57 < c && c < 65) || (90 < c && c < 97) || (122 < c && c < 127);
}

static inline bool str_empty(str_t str) {
    return str.ptr == 0 || str.len == 0;
}

static inline str_t trim_whitespace(str_t str) {
    const char* beg = str.ptr;
    const char* end = str.ptr + str.len;
    while (beg < end && is_whitespace(*beg)) ++beg;
    while (beg < end && (is_whitespace(end[-1]) || end[-1] == '\0')) --end;
    str.ptr = beg;
    str.len = end - beg;
    return str;
}

static inline bool compare_str(const str_t str_a, const str_t str_b) {
    if (!str_a.ptr || !str_b.ptr) return false;
    if (str_a.len != str_b.len) return false;
    for (int64_t i = 0; i < str_a.len; ++i) {
        if (str_a.ptr[i] != str_b.ptr[i]) return false;
    }
    return true;
}

static inline bool compare_str_n(const str_t str_a, const str_t str_b, int64_t n) {
    if (!str_a.ptr || !str_b.ptr) return false;
    if ((str_a.len < n || str_b.len < n) && str_a.len != str_b.len) return false;

    // str_a & str_b have equal len.
    n = n < str_a.len ? n : str_a.len;
    for (int64_t i = 0; i < n; ++i) {
        if (str_a.ptr[i] != str_b.ptr[i]) return false;
    }
    return true;
}

static inline bool compare_str_ignore_case(const str_t str_a, const str_t str_b) {
    if (!str_a.ptr || !str_b.ptr) return false;
    if (str_a.len != str_b.len) return false;
    for (int64_t i = 0; i < str_a.len; ++i) {
        if (to_lower(str_a.ptr[i]) != to_lower(str_b.ptr[i])) return false;
    }
    return true;
}

static inline bool compare_str_cstr(str_t str, const char* cstr) {
    if (!str.ptr || !str.len || !cstr) return false;
    for (int64_t i = 0; i < str.len; ++i) {
        if (cstr[i] == '\0' || str.ptr[i] != cstr[i]) return false;
    }
    return cstr[str.len] == '\0';
}

// Compare str and cstr only up to n characters
static inline bool compare_str_cstr_n(str_t str, const char* cstr, int64_t n) {
    if (n < 0) return false;
    if (!str.ptr || !str.len || !cstr) return false;
    n = n < str.len ? n : str.len;
    for (int64_t i = 0; i < n; ++i) {
        if (cstr[i] == '\0' || str.ptr[i] != cstr[i]) return false;
    }
    return true;
}

static inline bool compare_str_cstr_ignore_case(str_t str, const char* cstr) {
    if (!str.ptr || !str.len || !cstr) return false;
    for (int64_t i = 0; i < str.len; ++i) {
        if (cstr[i] == '\0' || to_lower(str.ptr[i]) != to_lower(cstr[i])) return false;
    }
    return cstr[str.len] == '\0';
}

static inline int64_t str_count_equal_chars(str_t a, str_t b) {
    if (!a.ptr || a.len <= 0 || !b.ptr || b.len <= 0) return 0;
    int64_t i = 0;
    for (; i < MIN(a.len, b.len); ++i) {
        if (a.ptr[i] != b.ptr[i]) break;
    }
    return i;
}

static inline int64_t str_count_char_occur(str_t str, char character) {
    if (!str.ptr || str.len <= 0) return 0;
    int64_t count = 0;
    for (int64_t i = 0; i < str.len; ++i) {
        if (str.ptr[i] == character) count += 1;
    }
    return count;
}

static inline str_t substr(str_t str, int64_t offset, int64_t length DEF_VAL(-1)) {
    if (offset < 0 || offset > str.len) {
        str_t res = {0, 0};
        return res;
    }
    if (offset + length > str.len || length < 0) length = str.len - offset;
    str.ptr = str.ptr + offset;
    str.len = length;
    return str;
}

bool skip_line(str_t* in_out_str);
bool peek_line(str_t* out_line, const str_t* in_str);
bool extract_line(str_t* out_line, str_t* in_out_str);

int64_t find_char(str_t str, int c);
int64_t rfind_char(str_t str, int c);

str_t str_find_str(str_t str, str_t str_to_find);

// Make sure you ave trimmed all whitespace before using these!
// They are lean and mean
double parse_float(str_t str);
int64_t parse_int(str_t str);

// Will allocate one extra character for zero termination
str_t alloc_str(uint64_t len, struct md_allocator_i* alloc);
void  free_str(str_t str, struct md_allocator_i* alloc);
str_t copy_str(str_t str, struct md_allocator_i* alloc);
str_t load_textfile(str_t path, struct md_allocator_i* alloc);
str_t alloc_printf(struct md_allocator_i* alloc, const char* format, ...);

// c:/folder/file.ext -> ext
str_t extract_ext(str_t path);

// c:/folder/file.ext -> file.ext
str_t extract_file(str_t path);

// c:/folder/file.ext -> c:/folder/file
str_t extract_path_without_ext(str_t path);

// c:/folder/file.ext -> c:/folder/
str_t extract_path_without_file(str_t path);

// Converts Windows backslashes '\\' to forward slashes '/'
static inline void convert_backslashes(char* str, int64_t len) {
    for (char* c = str; c != str + len; ++c) {
        if (*c == '\\') *c = '/';
    }
}

static inline void convert_to_lower(char* str, int64_t len) {
    for (char* c = str; c != str + len; ++c) {
        *c = (char)to_lower(*c);
    }
}

static inline void convert_to_upper(char* str, int64_t len) {
    for (char* c = str; c != str + len; ++c) {
        *c = (char)to_upper(*c);
    }
}

// Extracts token with whitespace as delimiter
bool extract_next_token(str_t* tok, str_t* str);
// Extracts token with specific delimiter
bool extract_next_token_delim(str_t* tok, str_t* str, char delim);

typedef struct buffered_reader_t {
    struct md_file_o* file;
    char* buffer;
    int64_t offset;
} buffered_reader_t;

bool buffered_reader_init(buffered_reader_t* reader, int64_t buf_size, str_t filename, struct md_allocator_i* alloc);
void buffered_reader_free(buffered_reader_t* reader, struct md_allocator_i* alloc);

// Get single line
bool buffered_reader_get_line(str_t* line, buffered_reader_t* reader);

// Get a big chunk as many complete lines as can we can fit into the reader buffer
bool buffered_reader_get_chunk(str_t* chunk, buffered_reader_t* reader);

bool buffered_reader_peek_line(str_t* line, buffered_reader_t* reader);

// Gets the total offset into the file (current chunk + offset)
int64_t buffered_reader_get_offset(const buffered_reader_t* reader);

#ifdef __cplusplus
}
#endif
