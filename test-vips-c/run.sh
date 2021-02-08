#!/bin/sh

set -x

CFLAGS=`pkg-config --cflags vips`
LIBS=`pkg-config --libs vips`
gcc $CFLAGS $LIBS -o test-vips main.c
