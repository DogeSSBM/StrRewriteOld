#!/bin/sh
clear
flags="-std=c11 -Wall -Wextra -Wpedantic -Werror -g"
# flags="-std=c11 -Wall -Wextra -Wpedantic -Werror -O3"
cc main.c $flags -o main.out
