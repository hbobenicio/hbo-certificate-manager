#pragma once

#include <regex.h>

void regex_compile_or_panic(regex_t* regex, const char* pattern, int flags);
