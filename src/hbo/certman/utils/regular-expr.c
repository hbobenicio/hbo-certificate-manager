#include "regular-expr.h"

#include <stdlib.h>

#include <glib.h>

void regex_compile_or_panic(regex_t* regex, const char* pattern, int flags)
{
    int rc = regcomp(regex, pattern, flags);
    if (rc != 0) {
        
        // When we call `regerror` with 0, 0 as last parameters it returns the error msg size
        // (already considering the '\0')
        const size_t error_msg_size = regerror(rc, regex, 0, 0);
        
        char* error_msg = calloc(error_msg_size, sizeof(char));
        if (error_msg == NULL) {
            g_error("regex: compilation failed: no more memory for getting the error message. pattern=\"%s\"\n", pattern);
            exit(1);
        }

        // We call `regerror` once again, now with the error_msg buffer to be set
        regerror(rc, regex, error_msg, error_msg_size);

        g_error("regex: compilation failed: %s. pattern=\"%s\"\n", error_msg, pattern);

        free(error_msg);
        exit(1);
    }
}
