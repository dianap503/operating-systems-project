# Bash Orchestrator Project

This project is designed to standardize project initialization, incremental compilation, execution management, and automated testing using a single Bash script entry point.

---

## Project Structure & Architecture

The project is structured around a central orchestrator script located at `tools/fileops.sh`, which exposes a coherent Command Line Interface (CLI) supporting five core subcommands: `init`, `build`, `run`, `clean`, and `test`.

```text
.
├── tools/
│   └── fileops.sh         # Main orchestration script (single entry point)
├── tests/
│   ├── test_init.sh       # Test script validating project initialization
│   └── test_build_recursive.sh # Test script validating recursive build and C source discovery
├── src/                   # Default source directory for C files
├── include/               # Header files directory
├── bin/                   # Compiled binary executables output directory
├── tmp/                   # Temporary build artifacts (e.g., tmp/obj/)
├── logs/                  # Automated execution logs
└── reports/               # Test execution reports (e.g., reports/T2_tests.txt)
```

---

## Core Components & Implementation Details

### 1. The Orchestrator Script (`tools/fileops.sh`)

The script serves as the single entry point for all project lifecycle operations. It includes robust automated logging via `trap`, capturing execution start/end timestamps, duration, executed subcommand, and final exit codes into the `logs/` directory.

*   **`init`**: Creates the standard project directory layout (`bin/`, `src/`, `include/`, `data/`, `logs/`, `reports/`, `tmp/obj/`, `tests/`, `doc/`, `tools/`) and checks for the availability of the `gcc` compiler, exiting cleanly with a diagnostic message if unavailable [cite: 1, 2].
*   **`build`**: Performs **incremental compilation** of C sources found recursively within a source directory (defaulting to `src/` or overridden via `--src <dir>`) [cite: 1, 2]. 
    *   **Explicit Recursion**: Implements custom recursive functions (`find_c_files` and `find_main_files`) to traverse subdirectories and discover `.c` files [cite: 1, 2].
    *   **Incremental Logic**: Object files (`.o`) are generated in `tmp/obj/` only if they do not exist or if the source file `.c` has a newer modification timestamp (`-nt`) [cite: 1, 2].
    *   **Binary Generation**: Files matching `main_<name>.c` are compiled and linked with available auxiliary object files to produce executables named `bin/<name>`. Auxiliary modules are compiled strictly to object files [cite: 1, 2].
*   **`run`**: Executes a compiled binary from `bin/` with passed arguments, parsing arguments cleanly past the `--` delimiter [cite: 1, 2].
*   **`clean`**: Removes build artifacts within `tmp/obj/` and `bin/` [cite: 1, 2].
*   **`test`**: Automatically executes all `.sh` test scripts found in the `tests/` directory using implicit recursion (`find tests/ -type f -name "*.sh"`), logs individual test outcomes (`PASS`/`FAIL`) into `reports/T2_tests.txt`, and exits with a non-zero code if any test fails [cite: 1, 2].

---

### 2. Automated Test Suite

Two comprehensive, non-interactive test scripts validate the core functionality on standard Linux systems:

*   **`tests/test_init.sh`**: Invokes `./tools/fileops.sh init` and verifies that all ten standard project directories (`bin/`, `src/`, `include/`, `data/`, `logs/`, `reports/`, `tmp/`, `tests/`, `doc/`, `tools/`) are correctly established [cite: 1, 4].
*   **`tests/test_build_recursive.sh`**: Dynamically generates a multi-level temporary C project structure (`tmp/test_scenario_src/` with `app/`, `lib/`, and `include/` subdirectories containing `main_demo.c`, `util.c`, and `util.h`), runs the recursive build script via `--src tmp/test_scenario_src`, executes the resulting binary (`bin/demo`), and asserts that output matches expected computation values (e.g., verifying `util_add(2, 3) == 5`) [cite: 1, 3].

---

## Usage Guide

### Initializing the Project
```bash
./tools/fileops.sh init
```

### Building the Project (Default or Custom Source Directory)
```bash
# Build from default src/ directory
./tools/fileops.sh build

# Build from a custom source directory
./tools/fileops.sh build --src path/to/sources
```

### Running an Executable
```bash
./tools/fileops.sh run -- demo_executable [arguments...]
```

### Running the Test Suite
```bash
./tools/fileops.sh test
```

### Cleaning Build Artifacts
```bash
./tools/fileops.sh clean
```
