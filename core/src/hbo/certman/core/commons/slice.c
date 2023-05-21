#include <hbo/certman/core/commons/slice.h>

#include <string.h>

#include <hbo/certman/core/commons/math.h>

void slice_const_copy(struct slice_const_char slice, char* dest, size_t dest_size)
{
    memcpy(dest, slice.ptr, MIN(dest_size, slice.len));
}
