#!/bin/bash
set -eu -o pipefail

BIN_DIR="./bin"
BIN_NAME="hbo-certificate-manager"
BIN="$BIN_DIR/$BIN_NAME"

BUILD_DIR="./build-sh"

CC="clang"
CFLAGS="-Wall -Wextra -Wpedantic -std=c17 -Werror=vla -I ./src -I $BUILD_DIR"
CFLAGS_DEBUG="-g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer"
CFLAGS_RELEASE_WITH_DEBUG_INFO="-g -O3 -march=native"
CFLAGS_RELEASE="-O3 -march=native"

PKG_CONFIG_DEPS="gtk4 libadwaita-1"
PKG_CONFIG_CFLAGS=$(pkg-config --cflags $PKG_CONFIG_DEPS)
PKG_CONFIG_LIBS=$(pkg-config --libs $PKG_CONFIG_DEPS)
CFLAGS="$CFLAGS $PKG_CONFIG_CFLAGS"

echo "Escolha o modo de compilação:"
select BUILD_MODE in Debug Release RelWithDebInfo; do break; done
case "$BUILD_MODE" in
    "Debug") CFLAGS="$CFLAGS $CFLAGS_DEBUG";;
    "Release") CFLAGS="$CFLAGS $CFLAGS_RELEASE";;
    "RelWithDebInfo") CFLAGS="$CFLAGS $CFLAGS_RELEASE_WITH_DEBUG_INFO";;
esac

echo "build: cleaning build directory"
rm -rf "$BIN_DIR" "$BUILD_DIR"
mkdir -p "$BIN_DIR" "$BUILD_DIR"

SOURCES=`find ./src -name '*.c'`
OBJECTS=()

# GResource compilation
echo "build: precompiling resources"
glib-compile-resources --generate-source \
    --target="$BUILD_DIR/hbo-certificate-manager.gresource.c" \
    "hbo-certificate-manager.gresource.xml"

# Compiling sources
for SRC in $SOURCES; do
    OUTPUT_OBJ="$BUILD_DIR/${SRC%.*}.o"
    mkdir -p "$(dirname $OUTPUT_OBJ)"

    echo "build: compiling \"$SRC\""
    $CC -c $CFLAGS -o "$OUTPUT_OBJ" $SRC

    OBJECTS+=("$OUTPUT_OBJ")
done

echo "build: compiling resource"
$CC -c $CFLAGS -o "$BUILD_DIR/hbo-certificate-manager.gresource.o" "$BUILD_DIR/hbo-certificate-manager.gresource.c"

echo "build: linking final executable"
$CC $CFLAGS -o $BIN "$BUILD_DIR/hbo-certificate-manager.gresource.o" ${OBJECTS[@]} $PKG_CONFIG_LIBS
