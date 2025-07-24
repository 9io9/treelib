#ifndef _LJW_ERROR_H_

#define _LJW_ERROR_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const char* message;
    unsigned int line;
    const char* file;
    const char* function;
}Error;

#define ERROR(msg) (Error) {.message = msg, .line = __LINE__, .file = __FILE__, .function = __func__}

typedef union {
    uint8_t to_u8;
    uint16_t to_u16;
    uint32_t to_u32;
    uint64_t to_u64;
    int8_t to_i8;
    int16_t to_i16;
    int32_t to_i32;
    int64_t to_i64;
    void* to_ptr;
}Any;

typedef struct {
    bool has_value;
    union {
        Error error;
        Any data;
    }result;
}Result;

#define RESULT_SUC(dt, d) (Result) {.has_value = true, .result = {.data = {.to_##dt = d}}}
#define RESULT_FAIL(msg) (Result) {.has_value = false, .result = {.error = ERROR(msg)}}

#endif