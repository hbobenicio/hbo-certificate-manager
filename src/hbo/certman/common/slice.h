#pragma once

#include <stddef.h>

struct slice_char {
    char* ptr;
    size_t len;
};

struct slice_const_char {
    const char* ptr;
    size_t len;
};

void slice_const_copy(struct slice_const_char slice, char* dest, size_t dest_size);
