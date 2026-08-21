# Project Deep Dive: Concurrent File Indexer, Process Snapshot, and Binary Diff Engine

## Executive Summary
This project is a systems-level utility suite built entirely in **C** on Linux, designed to capture point-in-time state snapshots of both the filesystem and running system processes under high-concurrency SPMD (Single Program, Multiple Data) execution. The architecture emphasizes low-level binary serialization, robust inter-process synchronization using advisory record locking, and custom differential analysis algorithms to report changes between snapshots.

---

## Architecture & Core Components

The system is divided into three primary modules, each engineered to address specific operating system and concurrency requirements:

### 1. The File System Indexer (`fileops_indexer`)
* **Directory Traversal:** Recursively navigates directory trees using POSIX directory streams (`opendir`, `readdir`) and retrieves deep file metadata via `lstat`.
* **Binary Serialization:** Encodes variable-length absolute paths, file sizes, custom DJB2-derived file hashes, modification times (`mtime`), device IDs (`dev_t`), and inode numbers (`ino_t`) directly into a custom, high-performance binary database format (`.db`).
* **Concurrency Support:** Engineered to handle multi-process, concurrent execution where multiple workers can safely analyze and index overlapping subtrees simultaneously.

### 2. The Process Snapshot (`proc_snapshot`)
* **Kernel Introspection:** Iterates through the Linux `/proc` pseudo-filesystem to extract real-time, low-level process metrics.
* **Extracted Attributes:** Captures Process IDs (`PID`), Parent Process IDs (`PPID`), execution states, command names (`comm`), full command lines (`cmdline`), Resident Set Size (`RSS`) memory usage, and cumulative CPU execution time.

### 3. The Differential Engine (`db_diff`)
* **Snapshot Analysis:** Parses two distinct binary snapshots (file indexes or process records), reconstructs their data structures in dynamic heap memory, and executes set-based comparison logic.
* **Report Generation:** Produces clean, structured text reports highlighting items that have **appeared**, **disappeared**, or undergone **significant modifications** (such as file updates or major RSS memory fluctuations).

---

## Core Engineering Highlights

### A. Non-Blocking Concurrency & Advisory Locking
* **Mechanism:** Implements robust advisory file locking using POSIX `fcntl` with exclusive write locks (`F_WRLCK`).
* **Resilience:** Integrates a controlled retry-backoff loop handling lock contention (`EACCES` and `EAGAIN`) with brief sleep intervals, ensuring data integrity and safe multi-process coordination without causing deadlocks.

### B. Binary Layout Portability & Strict Serialization
* **Memory Alignment:** Utilizes explicit structure packaging (`__attribute__((packed))`) across shared database headers and record definitions. This guarantees bit-for-bit parity of data structures written to and read from disk, ensuring complete cross-architecture stability.
* **Cursor Synchronization:** Manages precise byte offsets, variable-length string boundaries, and exact type mappings (`off_t`, `time_t`, `dev_t`, `ino_t`) during sequential binary reads and writes.

---

## Technical Stack & Competencies Demonstrated

* **Language:** C (C99/C11 standards), low-level memory management, pointer arithmetic, manual heap allocation (`malloc`/`free`).
* **Linux Systems Programming:** POSIX file descriptors (`open`, `read`, `write`, `lseek`), directory streams, metadata queries (`lstat`), advisory locking (`fcntl`).
* **Concurrency & Synchronization:** SPMD execution flow, race condition mitigation, retry-backoff algorithms.
* **Data Structures & Algorithms:** Custom binary file formats, hashing algorithms (DJB2), set difference algorithms for relational comparison.
* **Tooling & Workflow:** Bash scripting for automated test orchestration, Makefiles/build scripts, Git version control.
