#pragma once

#include <stddef.h>

/**
 * Returns the first binary occurrence found in the PATH
 */
int find_bin_in_path(const char* bin_name, char* out_bin_path, size_t out_bin_path_size);
