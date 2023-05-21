#include "fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <hbo/certman/core/commons/slice.h>

#ifndef HBO_CERTMAN_UTILS_FS_MAX_DIR_PATH_SIZE
#define HBO_CERTMAN_UTILS_FS_MAX_DIR_PATH_SIZE 512
#endif

int find_bin_in_path(const char* bin_name, char* out_bin_path, size_t out_bin_path_size)
{
    const char* path = getenv("PATH");
    if (path == NULL) {
        g_error("PATH environment variable is not set");
        return 1;
    }
    
    const char* c = path;
    while (*c != '\0') {
        if (*c == ':') {
            c++;
            continue;
        }
        struct slice_const_char directory = {
            .ptr = c,
            .len = 0,
        };
        while (*c != '\0' && *c != ':') {
            directory.len++;
            c++;
        }

        char dir_path[HBO_CERTMAN_UTILS_FS_MAX_DIR_PATH_SIZE + 1] = {0};
        slice_const_copy(directory, dir_path, HBO_CERTMAN_UTILS_FS_MAX_DIR_PATH_SIZE);

        DIR* dir = opendir(dir_path);
        if (dir == NULL) {
            int error_code = errno;
            g_error("opendir failed: %s. dir_path=\"%s\"", strerror(error_code), dir_path);
            continue;
        }

        // To distinguish end of stream from an error, set errno to zero before calling readdir()
        // and then check the value of errno if NULL is returned
        errno = 0;
        for (struct dirent* dir_entry = readdir(dir); dir_entry != NULL; dir_entry = readdir(dir)) {
            if (strcmp(dir_entry->d_name, bin_name) == 0) {
                
                int rc = snprintf(out_bin_path, out_bin_path_size, "%s/%s", dir_path, dir_entry->d_name);
                if (rc < 0 || (size_t) rc >= out_bin_path_size) {
                    g_error("bin path is too big for output buffer. bin_path=\"%s/%s\" buffer_size=%zu", dir_path, dir_entry->d_name, out_bin_path_size);
                    closedir(dir);
                    return 1;
                }

                closedir(dir);
                return 0;
            }
        }
        if (errno != 0) {
            g_error("readdir failed: %s. dir_path=%s", strerror(errno), dir_path);
            closedir(dir);
            continue;
        }
        
        closedir(dir);
    }
    
    return 1;
}
