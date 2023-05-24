#!/bin/sh
printf "\033c"
libraries="-lm"
flags="-Wall -Wno-missing-braces"
# -O3 -Winline"
gcc main.c $libraries $flags -o main.out
