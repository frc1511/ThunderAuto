#!/bin/bash

if [ -z "$1" ]; then
  echo "Usage: $0 <arch> <exe>"
  exit 1
fi

ARCH=$1

if [ "$ARCH" != "x86_64" ] && [ "$ARCH" != "i686" ]; then
  echo "Invalid arch: $ARCH, must be x86_64 or i686"
  exit 1
fi

if [ -z "$2" ]; then
  echo "Usage: $0 <arch> <exe>"
  exit 1
fi

WINEPATH=/opt/homebrew/opt/mingw-w64/toolchain-$ARCH/$ARCH-w64-mingw32/bin/ \
  wine64 $2
