#include "md_str.h"

#include "md_common.h"
#include "md_file.h"
#include "md_allocator.h"
#include "md_array.inl"
#include "md_log.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

str_t str_from_cstr(const char* cstr) {
    str_t str = {cstr, strlen(cstr)};
    return str;
}

bool skip_line(str_t* in_out_str) {
    ASSERT(in_out_str);
    const char* c = (const char*)memchr(in_out_str->ptr, '\n', in_out_str->len);
    if (c) {
        in_out_str->len = in_out_str->len - (c - in_out_str->ptr);
        in_out_str->ptr = c + 1;
        return in_out_str;
    }
    return false;
}

bool peek_line(str_t* out_line, const str_t* in_str) {
    ASSERT(out_line);
    ASSERT(in_str);
    const char* beg = in_str->ptr;
    const char* end = (const char*)memchr(in_str->ptr, '\n', in_str->len);
    if (end) {
        out_line->ptr = beg;
        out_line->len = end - beg;
        return true;
    }
    return false;
}

bool extract_line(str_t* out_line, str_t* in_out_str) {
    ASSERT(out_line);
    ASSERT(in_out_str);
    if (in_out_str->len == 0) return false;

    const char* beg = in_out_str->ptr;
    const char* end = in_out_str->ptr + in_out_str->len;
    const char* c = (const char*)memchr(in_out_str->ptr, '\n', in_out_str->len);
    if (c) {
        end = c + 1;
    }

    out_line->ptr = beg;
    out_line->len = end - beg;

    in_out_str->len = in_out_str->ptr + in_out_str->len - end;
    in_out_str->ptr = end;
    
    return true;
}

int64_t find_char(str_t str, int c) {
    // @TODO: Implement support for UTF encodings here.
    // Identify the effective width used in c and search for that.
    for (int64_t i = 0; i < (int64_t)str.len; ++i) {
        if (str.ptr[i] == c) return i;
    }
    return -1;
}

int64_t rfind_char(str_t str, int c) {
    for (int64_t i = str.len - 1; i >= 0; --i) {
        if (str.ptr[i] == c) return i;
    }
    return -1;
}

str_t str_find_str(str_t haystack, str_t needle) {
    const char* h_beg = haystack.ptr;
    const char* h_end = haystack.ptr + haystack.len;
    str_t result = {0};

    if (haystack.len == 0) goto done;
    if (needle.len == 0) goto done;
    if (needle.len > haystack.len) {
        md_print(MD_LOG_TYPE_ERROR, "Trying to find 'needle' which is larger than supplied 'haystack'");
        goto done;
    }

    int64_t i = 0;
    const char* n_beg = 0;
    for (const char* c = h_beg; c != h_end; ++c) {
        if (*c == needle.ptr[i]) {
            if (i == 0) n_beg = c;
            ++i;
        } else {
            i = 0;
        }

        if (i == needle.len) {
            result.ptr = n_beg;
            result.len = needle.len;
            goto done;
        }
    }

done:
    return result;
}

double parse_float(str_t str) {
    ASSERT(str.ptr);
    static const double pow10[16] = {
        1e+0,  1e+1,  1e+2,  1e+3,
        1e+4,  1e+5,  1e+6,  1e+7,
        1e+8,  1e+9,  1e+10, 1e+11,
        1e+12, 1e+13, 1e+14, 1e+15
    };

    const char* c = str.ptr;
    const char* end = str.ptr + str.len;

    double val = 0;
    double sign = 1;
    if (*c == '-') {
        ++c;
        sign = -1;
    }
    while (c != end && is_digit(*c)) {
        val = val * 10 + ((int)(*c) - '0');
        ++c;
    }
    if (*c != '.' || c == end) return sign * val;

    ++c; // skip '.'
    const uint32_t count = (uint32_t)(end - c);
    while (c < end && is_digit(*c)) {
        val = val * 10 + ((int)(*c) - '0');
        ++c;
    }

    return sign * val / pow10[count];
}

int64_t parse_int(str_t str) {
    int64_t val = 0;
    const char* c = str.ptr;
    const char* end = str.ptr + str.len;
    int64_t sign = 1;
    if (*c == '-') {
        ++c;
        sign = -1;
    }
    while (c != end && is_digit(*c)) {
        val = val * 10 + ((int)(*c) - '0');
        ++c;
    }
    return sign * val;
}

str_t load_textfile(str_t filename, struct md_allocator_i* alloc) {
    ASSERT(alloc);
    md_file_o* file = md_file_open(filename, MD_FILE_READ | MD_FILE_BINARY);
    str_t result = {0,0};
    if (file) {
        int64_t file_size = md_file_size(file);
        char* mem = md_alloc(alloc, file_size + 1);
        if (mem) {
            int64_t read_size = md_file_read(file, mem, file_size);
            if (read_size == file_size) {
                mem[file_size] = '\0'; // Zero terminate as a nice guy
                result.ptr = mem;
                result.len = file_size;
            }
            else {
                md_print(MD_LOG_TYPE_ERROR, "Failed to read full textfile");
            }
        } else {
            md_printf(MD_LOG_TYPE_ERROR, "Could not allocate memory for file %d", 10);
        }
        md_file_close(file);
    }
    return result;
}

str_t alloc_str(uint64_t len, struct md_allocator_i* alloc) {
    ASSERT(alloc);
    char* mem = md_alloc(alloc, len + 1);
    memset(mem, 0, len + 1);
    str_t str = {mem, len};
    return str;
}

void free_str(str_t str, struct md_allocator_i* alloc) {
    ASSERT(alloc);
    md_free(alloc, (void*)str.ptr, str.len + 1);
}

str_t copy_str(const str_t str, struct md_allocator_i* alloc) {
    ASSERT(alloc);
    str_t result = {0,0};
    if (str.ptr && str.len > 0) {
        char* data = md_alloc(alloc, str.len + 1);
        data[str.len] = '\0';
        memcpy(data, str.ptr, str.len);
        result.ptr = data;
        result.len = str.len;
    }
    return result;
}

str_t alloc_printf(struct md_allocator_i* alloc, const char* format, ...) {
    va_list args;

    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);

    char* buf = md_alloc(alloc, len + 1);

    va_start(args, format);
    vsnprintf(buf, len + 1, format, args);
    va_end(args);

    return (str_t) {buf, len};
}

// c:/folder/file.ext -> ext
str_t extract_ext(str_t path) {
    int64_t pos = rfind_char(path, '.');

    str_t res = {0,0};
    if (pos > -1) {
        pos += 1; // skip '.'
        res.ptr = path.ptr + pos;
        res.len = path.len - pos;
    }
    return res;
}

// c:/folder/file.ext -> file.ext
str_t extract_file(str_t path) {
    int64_t pos = rfind_char(path, '/');
    if (pos == -1) {
        pos = rfind_char(path, '\\');
    }

    str_t res = {0,0};
    if (pos != -1) {
        pos += 1; // skip slash or backslash
        res.ptr = path.ptr + pos;
        res.len = path.len - pos;
    }
    return res;
}

// c:/folder/file.ext -> c:/folder/file.
str_t extract_path_without_ext(str_t path) {
    const int64_t pos = rfind_char(path, '.');

    str_t res = {0,0};
    if (pos) {
        res.ptr = path.ptr;
        res.len = pos;
    }
    return res;
}

// c:/folder/file.ext -> c:/folder/
str_t extract_path_without_file(str_t path) {
    int64_t pos = rfind_char(path, '/');
    if (pos == -1) {
        pos = rfind_char(path, '\\');
    }

    str_t res = {0};
    if (pos != -1) {
        res.ptr = path.ptr;
        res.len = pos+1;    // include '/' or '\'
    }
    return res;
}

bool extract_next_token(str_t* tok, str_t* str) {
    ASSERT(tok);
    ASSERT(str);
    if (!str->ptr || str->len == 0) return false;

    const char* end = str->ptr + str->len;
    const char* c = str->ptr;
    while (c < end && is_whitespace(*c)) ++c;
    if (c >= end) return false;

    const char* tok_beg = c;
    while (c < end && !is_whitespace(*c)) ++c;
     
    tok->ptr = tok_beg;
    tok->len = c - tok_beg;

    str->ptr = c < end ? c + 1 : end;
    str->len = end - str->ptr;

    return true;
}

bool extract_next_token_delim(str_t* tok, str_t* str, char delim) {
    ASSERT(tok);
    ASSERT(str);
    if (!str->ptr || str->len == 0) return false;

    const char* beg = str->ptr;
    const char* end = str->ptr + str->len;
    const char* c = str->ptr;
    while (c != end && *c != delim) {
        ++c;
    }
    tok->ptr = beg;
    tok->len = c - beg;

    str->ptr = c != end ? c + 1 : end;
    str->len = end - str->ptr;

    return true;
}

bool buffered_reader_init(buffered_reader_t* reader, int64_t buf_size, str_t filename, struct md_allocator_i* alloc) {
    if (!reader) {
        md_print(MD_LOG_TYPE_ERROR, "Reader was null");
        return false;
    }

    if (buf_size <= 0) {
        md_print(MD_LOG_TYPE_ERROR, "Invalid buffer size");
        return false;
    }

    if (!alloc) {
        md_print(MD_LOG_TYPE_ERROR, "Allocator was null");
        return false;
    }

    if (str_empty(filename)) {
        md_print(MD_LOG_TYPE_ERROR, "Filename was empty");
        return false;
    }

    md_file_o* file = md_file_open(filename, MD_FILE_READ | MD_FILE_BINARY);
    if (!file) {
        md_print(MD_LOG_TYPE_ERROR, "Could not open file");
        return false;
    }
    
    if (reader->buffer) {
        md_print(MD_LOG_TYPE_DEBUG, "Reader is not zero, potential memory leak");
    }
    memset(reader, 0, sizeof(buffered_reader_t));

    reader->file = file;
    md_array_ensure(reader->buffer, buf_size, alloc);

    return true;
}


void buffered_reader_free(buffered_reader_t* reader, struct md_allocator_i* alloc) {
    ASSERT(reader);
    ASSERT(alloc);

    md_array_free(reader->buffer, alloc);
    md_file_close(reader->file);
}

static inline bool buffered_reader_fill_buffer(buffered_reader_t* reader) {
    ASSERT(reader);

    // Copy eventual remainder to beginning
    const int64_t len = reader->offset;
    const int64_t cap = md_array_capacity(reader->buffer);
    const int64_t remainder = len > 0 ? cap - len : 0;
    const int64_t target_bytes = cap - remainder;

    if (remainder > 0) {
        // Copy remainder to beginning of buffer
        memcpy(reader->buffer, reader->buffer + cap - remainder, remainder);
    }

    const int64_t read_bytes = md_file_read(reader->file, reader->buffer + remainder, target_bytes);
    if (read_bytes == 0) {
        return false;
    }

    const int64_t new_size = remainder + read_bytes;
    md_array_shrink(reader->buffer, new_size);
    reader->offset = 0;

    return new_size > 0;
}

static inline str_t buffered_reader_str(const buffered_reader_t* reader) {
    return (str_t){reader->buffer + reader->offset, md_array_size(reader->buffer) - reader->offset};
}

// Get single line
bool buffered_reader_get_line(str_t* line, buffered_reader_t* reader) {
    ASSERT(line);
    ASSERT(reader);

    int64_t new_line = find_char(buffered_reader_str(reader), '\n');
    if (new_line == -1) {
        // Could not find new line, 
        if (!buffered_reader_fill_buffer(reader)) return false;
    }
    
    new_line = find_char(buffered_reader_str(reader), '\n');
    if (new_line == -1) new_line = md_array_size(reader->buffer);
    line->ptr = reader->buffer + reader->offset;
    line->len = new_line - reader->offset;

    reader->offset = new_line;

    return true;
}

bool buffered_reader_peek_chunk(str_t* chunk, buffered_reader_t* reader) {
    ASSERT(chunk);
    ASSERT(reader);

    int64_t new_line = rfind_char(buffered_reader_str(reader), '\n');
    if (new_line == -1) {
        // Could not find new line, 
        if (!buffered_reader_fill_buffer(reader)) return false;
        new_line = rfind_char(buffered_reader_str(reader), '\n');
        if (new_line == -1) new_line = md_array_size(reader->buffer);
    }

    chunk->ptr = reader->buffer + reader->offset;
    chunk->len = new_line - reader->offset;

    return true;
}

// Get chunk
bool buffered_reader_get_chunk(str_t* chunk, buffered_reader_t* reader) {
    ASSERT(chunk);
    ASSERT(reader);

    int64_t new_line = rfind_char(buffered_reader_str(reader), '\n');
    if (new_line == -1) {
        // Could not find new line, 
        if (!buffered_reader_fill_buffer(reader)) return false;
        new_line = rfind_char(buffered_reader_str(reader), '\n');
        if (new_line == -1) new_line = md_array_size(reader->buffer);
    }

    chunk->ptr = reader->buffer + reader->offset;
    chunk->len = new_line - reader->offset;

    reader->offset = new_line + 1;

    return true;
}

int64_t buffered_reader_get_offset(const buffered_reader_t* reader) {
    ASSERT(reader);
    const int64_t file_offset = md_file_tell(reader->file) - md_array_size(reader->buffer);
    return file_offset + reader->offset;
}
