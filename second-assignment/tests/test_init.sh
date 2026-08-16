#!/bin/bash
./tools/fileops.sh init > /dev/null
dirs=(bin src include data logs reports tmp tests doc tools)
for d in "${dirs[@]}"; do
    if [[ ! -d "$d" ]]; then
        exit 1
    fi
done
exit 0
