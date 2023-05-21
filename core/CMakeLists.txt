add_library(hbo-certificate-manager-core STATIC
    "./src/hbo/certman/core/commons/slice.c"
)

set_target_properties(hbo-certificate-manager-core PROPERTIES C_STANDARD 17)

target_include_directories(hbo-certificate-manager-core
    PRIVATE "./src" "./include"
    INTERFACE "./include"
    PUBLIC "./include"
)
