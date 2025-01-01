#!/bin/bash

if [ -z "$1" ]; then
  echo "Usage: $0 <arch> <build_dir> [cmake args]"
  exit 1
fi

ARCH=$1

if [ "$ARCH" != "x86_64" ] && [ "$ARCH" != "i686" ]; then
  echo "Invalid arch: $ARCH, must be x86_64 or i686"
  exit 1
fi


if [ -z "$2" ]; then
  echo "Usage: $0 <arch> <build_dir> [cmake args]"
  exit 1
fi

BUILD_DIR=$2

shift 2

cmake . -B$BUILD_DIR \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_SYSTEM_VERSION=10.0 \
  -DCMAKE_SYSTEM_PROCESSOR=$ARCH \
  -DCMAKE_C_COMPILER=/opt/homebrew/bin/$ARCH-w64-mingw32-gcc \
  -DCMAKE_CXX_COMPILER=/opt/homebrew/bin/$ARCH-w64-mingw32-g++ \
  -DTH_WINDRES_EXECUTABLE=/opt/homebrew/bin/$ARCH-w64-mingw32-windres \
  $@

