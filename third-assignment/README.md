# Assignment T3: File Indexer and Process Snapshots in Versioned Binary Databases with Synchronization

This repository contains the implementation for **Assignment T3: File Indexing and Process Snapshots in Versioned Binary Databases, with Synchronization**. It extends the T2 Bash Orchestrator infrastructure with advanced C utilities for file tree indexing, `/proc` process snapshotting, atomic binary database versioning, concurrent SPMD synchronization via `fcntl` record locking, and binary diff reporting.

---

## Project Structure & Architecture

Building upon the T2 infrastructure (`tools/fileops.sh`), the project introduces new core C source modules, automated concurrency test scripts, database format specifications, and comparison report outputs:

```text
.
├── tools/
│   └── fileops.sh         # Main T2 orchestration script entry point
├── src/
│   ├── fileops_indexer.c  # Recursive directory indexer writing to versioned binary index.db
│   ├── proc_snapshot.c    # /proc process snapshot collector writing to versioned binary proc.db
│   └── db_diff.c          # Binary database comparison utility generating text reports
├── tests/
│   ├── test_init.sh       # Validates project initialization
│   ├── test_build_recursive.sh # Validates recursive C build and compilation
│   └── t3_concurrency.sh  # Validates SPMD concurrent execution, locking, and diff reports
├── include/               # Header files directory
├── data/                  # Versioned binary databases (index.db, proc.db, etc.)
├── logs/                  # Automated execution logs
├── reports/
│   ├── T2_tests.txt       # T2 test execution outcomes
│   ├── T3_filediff.txt    # File database diff report
│   └── T3_procdiff.txt    # Process database diff report
└── doc/
    └── FORMAT_DB.md       # Detailed binary database format specifications
```

---

## Core Components & Implementation Details

### 1. Integration with T2 Infrastructure
All operations are executed exclusively through the single orchestrator entry point:
*   `./tools/fileops.sh init` — Initialized project directory structure.
*   `./tools/fileops.sh build` — Compiles C sources incrementally, generating executables under `bin/` (`bin/fileops_indexer`, `bin/proc_snapshot`, `bin/db_diff`).
*   `./tools/fileops.sh run <program> [args...]` — Executes compiled binaries under the orchestrator.
*   `./tools/fileops.sh test` — Automatically runs all test scripts under `tests/` (including `t3_concurrency.sh`).

---

### 2. File Indexer (`bin/fileops_indexer`)
The `fileops_indexer` utility recursively traverses a specified directory tree and writes entries into a versioned binary database (`data/index.db` by default or overridden via `--db <path>`).
*   **Command Line Interface**: `./tools/fileops.sh run fileops_indexer --root <dir> [--db <path>]`.
*   **Stored Metadata**: Absolute paths (relative to `/`), file type (regular file, directory, symlink, fifo, socket, block/character device), file size (for regular files; 0 for others), modification timestamp (`mtime`), device ID (`st_dev`), inode number (`st_ino`), and a deterministic content hash computed using the **DJB2 hash algorithm**.
*   **Traversal Rules**: Symlinks are recorded strictly as symlinks without dereferencing or recursive descent through target directories.

---

### 3. Process Snapshotter (`bin/proc_snapshot`)
The `proc_snapshot` utility captures system process states from `/proc` and stores them into a versioned binary database (`data/proc.db` by default).
*   **Command Line Interface**: `./tools/fileops.sh run proc_snapshot [--db <path>]`.
*   **Captured Information**: PID, PPID, process state, command name (`comm`), command line arguments (`cmdline`), Resident Set Size (`RSS` memory from `/status`), and combined user/system CPU time.
*   **Snapshot Bounding**: Snapshots are strictly bound to the initial set of PIDs observed at startup; newly spawned processes are ignored, and disappeared processes are safely handled.

---

### 4. Versioned Binary Format (`doc/FORMAT_DB.md`)
Both databases (`index.db` and `proc.db`) feature a robust binary header structure followed by records:
*   **Header Fields**: 
    *   `magic`: 4-byte identifier (`IDXI` for index databases, `PRCI` for process databases).
    *   `format_version`: Format interpretation version (`uint32_t`).
    *   `snapshot_id`: Unique timestamp-based session identifier (`uint64_t`).
    *   `snapshot_state`: Lifecycle state (`0` for `OPEN`, `1` for `SEALED`).
    *   `active_writers`: Counter tracking concurrent instances attached to the snapshot (`int32_t`).
    *   `record_count`: Total valid records stored (`uint32_t`).

---

### 5. Concurrent Synchronization (SPMD Model)
Synchronization is implemented directly on the binary database files using mandatory advisory locking via POSIX `fcntl(2)` exclusive write locks (`F_WRLCK`), avoiding separate lock files:
*   **Initialization**: The first instance creates the DB file, sets the header metadata (`OPEN`, `active_writers = 1`), and initializes records.
*   **Attachment**: Subsequent concurrent instances (3–5 SPMD workers) lock the header, increment `active_writers`, and append contributions.
*   **Sealing**: When an instance completes its contribution, it decrements `active_writers` under a header lock; the final instance reducing the counter to `0` automatically updates `snapshot_state` to `SEALED`.

---

### 6. Database Diff Utility (`bin/db_diff`)
Compares two binary snapshots of the same database type (`IDXI` or `PRCI`) and format version, generating structured comparison reports:
*   **Command Line Interface**: `./tools/fileops.sh run db_diff --old <db1> --new <db2> --out <path>`.
*   **File Diff (`reports/T3_filediff.txt`)**: Detects entries that `APPEARED`, `DISAPPEARED`, or were `MODIFIED` (comparing path, type, size, `mtime`, and content hash).
*   **Process Diff (`reports/T3_procdiff.txt`)**: Detects processes that `APPEARED`, `DISAPPEARED`, or changed significantly (such as an RSS memory fluctuation exceeding 1024 KB).

---

## Usage Guide

### 1. Initializing and Building
```bash
./tools/fileops.sh init
./tools/fileops.sh build
```

### 2. Running Indexer and Process Snapshot
```bash
./tools/fileops.sh run fileops_indexer --root src --db data/index.db
./tools/fileops.sh run proc_snapshot --db data/proc.db
```

### 3. Running Database Diff
```bash
./tools/fileops.sh run db_diff --old data/old_index.db --new data/new_index.db --out reports/T3_filediff.txt
```

### 4. Running the Concurrency Test Suite
```bash
./tools/fileops.sh test
```
