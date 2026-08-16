#!/bin/bash
SC_SRC="tmp/scenariu_test_src"
rm -rf "$SC_SRC"
mkdir -p "$SC_SRC/app" "$SC_SRC/lib/include"
cat <<EOF > "$SC_SRC/lib/include/util.h"
int util_add(int a, int b);
EOF
cat <<EOF > "$SC_SRC/lib/util.c"
#include "include/util.h"
int util_add(int a, int b) { return a + b; }
EOF
cat <<EOF > "$SC_SRC/app/main_demo.c"
#include <stdio.h>
#include "../lib/include/util.h"
int main() { printf("%d", util_add(2, 3)); return 0; }
EOF
export CFLAGS="-I$SC_SRC/lib -std=c11"
./tools/fileops.sh build --src "$SC_SRC" > /dev/null
if [[ ! -x "bin/demo" ]]; then
    exit 1
fi
OUTPUT=$(./bin/demo)
if [[ "$OUTPUT" == "5" ]]; then
    exit 0
else
    exit 1
fi
