cmake_minimum_required(VERSION 3.24)
project(hbo-certificate-manager LANGUAGES C)

option(HBO_CERTMAN_SANITIZATION "Enables/Disables sanitization" ON)

# Using pkg-config to find dependencies
find_package(PkgConfig REQUIRED)

# Dependencies:
#
# On Ubuntu 22.04:
# - libgtk-4-dev
# - libadwaita-1-dev
# - libssl-dev
pkg_check_modules(GTK4 REQUIRED "gtk4")
pkg_check_modules(ADW1 REQUIRED "libadwaita-1")
pkg_check_modules(OPENSSL REQUIRED "openssl")

# https://stackoverflow.com/questions/63697778/how-to-use-glib-compile-resources-with-cmake
#TODO corrigir dependencia desse comando (alterar o xml não provoca recompilacao do recurso)
find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

# set(CMAKE_VERBOSE_MAKEFILE ON)
set(HBO_CERTMAN_GRESOURCE_C   "${CMAKE_BINARY_DIR}/hbo-certificate-manager.gresource.c")
set(HBO_CERTMAN_GRESOURCE_XML "${CMAKE_SOURCE_DIR}/hbo-certificate-manager.gresource.xml")

add_custom_command(
    OUTPUT ${HBO_CERTMAN_GRESOURCE_C}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${GLIB_COMPILE_RESOURCES}
    ARGS
        --generate-source
        --target=${HBO_CERTMAN_GRESOURCE_C}
        ${HBO_CERTMAN_GRESOURCE_XML}
    VERBATIM
    MAIN_DEPENDENCY ${HBO_CERTMAN_GRESOURCE_XML}
    # DEPENDS
    #     for.glade
    #     bar.glade
)
add_custom_target("${PROJECT_NAME}-gresource"
    DEPENDS ${HBO_CERTMAN_GRESOURCE_C}
)

# Commons flags for all build profiles
set(HBO_CERTMAN_C_FLAGS " -Wall -Wextra -Wpedantic -Werror=vla")
string(APPEND CMAKE_C_FLAGS ${HBO_CERTMAN_C_FLAGS})
string(APPEND CMAKE_C_FLAGS_DEBUG " -O0")

if(HBO_CERTMAN_SANITIZATION)
    string(APPEND CMAKE_C_FLAGS_DEBUG " -fsanitize=address,undefined -fno-omit-frame-pointer")
endif()

# add_compile_options(-Wall -Wextra -Wpedantic -Werror=vla)

add_subdirectory(core)

add_executable(${PROJECT_NAME}
    "${CMAKE_SOURCE_DIR}/src/hbo/certman/utils/fs.c"
    "${CMAKE_SOURCE_DIR}/src/hbo/certman/utils/regular-expr.c"
    "${CMAKE_SOURCE_DIR}/src/hbo/certman/tls/client.c"
    "${CMAKE_SOURCE_DIR}/src/hbo/certman/main-window.c"
    "${CMAKE_SOURCE_DIR}/src/hbo/certman/app.c"
    "${CMAKE_SOURCE_DIR}/src/hbo/certman/main.c"
    "${HBO_CERTMAN_GRESOURCE_C}"
)
# add_dependencies(${PROJECT_NAME} "hbo-certificate-manager-core")
set_source_files_properties("${HBO_CERTMAN_GRESOURCE_C}" PROPERTIES GENERATED TRUE)
set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD 17
)
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    # "$<TARGET_PROPERTY:hbo-certificate-manager-core,INTERFACE_INCLUDE_DIRECTORIES>"
    "${GTK4_INCLUDE_DIRS}"
    "${ADW1_INCLUDE_DIRS}"
    "${OPENSSL_INCLUDE_DIRS}"
)
target_link_directories(${PROJECT_NAME} PRIVATE
    "${GTK4_LIBRARY_DIRS}"
    "${ADW1_LIBRARY_DIRS}"
    "${OPENSSL_LIBRARY_DIRS}"
)
target_link_libraries(${PROJECT_NAME}
    "hbo-certificate-manager-core"
    "${GTK4_LIBRARIES}"
    "${ADW1_LIBRARIES}"
    "${OPENSSL_LIBRARIES}"
)
add_dependencies(${PROJECT_NAME} "${PROJECT_NAME}-gresource")
