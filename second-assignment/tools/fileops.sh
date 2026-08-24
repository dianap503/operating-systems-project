#!/bin/bash

START_TIME_MS=$(date +%s)
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
LOG_DIR="logs"
mkdir -p "$LOG_DIR"
LOG_FILE="$LOG_DIR/fileops_$TIMESTAMP.log"

log_execution() {
    END_TIME_MS=$(date +%s)
    DURATION=$((END_TIME_MS - START_TIME_MS))
    EXIT_CODE=$1
    {
        echo "Executed subcommand: $SUBCOMMAND"
        echo "Timestamp start: $START_TIME_MS"
        echo "Timestamp end: $END_TIME_MS"
        echo "Total duration: ${DURATION}s"
        echo "Exit code: $EXIT_CODE"
    } > "$LOG_FILE"
}

do_init() {
    echo "Executare init..."
    local dirs=(bin src include data logs reports tmp tests doc tools)
    for dir in "${dirs[@]}"; do
        mkdir -p "$dir"
    done
    
    if ! command -v gcc &> /dev/null; then
        echo "ERROR: gcc was not found!"
        return 1
    fi
    echo "Structured created and gcc verified."
    return 0
}

do_build() {
    local SRC_DIR="src"
    if [[ "$1" == "--src" ]]; then
        SRC_DIR="$2"
    fi

    mkdir -p tmp/obj bin

    compile_recursive() {
        for item in "$1"/*; do
            if [[ -d "$item" ]]; then
                compile_recursive "$item"
            elif [[ "$item" == *.c ]]; then
                local filename=$(basename "$item")
                local obj="tmp/obj/${filename%.c}.o"
                
                if [[ ! -f "$obj" || "$item" -nt "$obj" ]]; then
                    echo "Incremental compilation: $item"
                    gcc $CFLAGS -c "$item" -o "$obj" || return 1
                fi
            fi
        done
    }
    compile_recursive "$SRC_DIR"

    find "$SRC_DIR" -name "main_*.c" | while read -r main_file; do
        local base=$(basename "$main_file" .c)
        local exe_name=${base#main_}
        echo "Link-edit executable: bin/$exe_name"
        gcc $CFLAGS tmp/obj/*.o -o "bin/$exe_name"
    done
    return 0
}

do_run() {
    while [[ "$1" != "--" && $# -gt 0 ]]; do shift; done
    shift 
    
    local exe="bin/$1"
    shift 
    
    if [[ -x "$exe" ]]; then
        ./"$exe" "$@"
    else
        echo "ERROR: The executable $exe does not exit or is not executable."
        return 1
    fi
}

do_clean() {
    echo "Cleaning build artifacts..."
    rm -rf tmp/obj/* bin/*
}

do_test() {
    local report="reports/T2_tests.txt"
    mkdir -p reports
    > "$report"
    local global_fail=0

    find tests -name "*.sh" | while read -r test_script; do
        [[ "$test_script" == *"$0"* ]] && continue
        
        chmod +x "$test_script"
        echo "Run test: $test_script"
        if ./"$test_script"; then
            echo "$(basename "$test_script") PASS" >> "$report"
        else
            echo "$(basename "$test_script") FAIL" >> "$report"
            global_fail=1
        fi
    done
    
    [[ $global_fail -eq 1 ]] && return 1
    return 0
}

SUBCOMMAND=$1
shift

case "$SUBCOMMAND" in
    init)  do_init "$@"; RES=$? ;;
    build) do_build "$@"; RES=$? ;;
    run)   do_run "$@"; RES=$? ;;
    clean) do_clean "$@"; RES=$? ;;
    test)  do_test "$@"; RES=$? ;;
    *)     echo "Invalid command!"; RES=1 ;;
esac

log_execution $RES
exit $RES
