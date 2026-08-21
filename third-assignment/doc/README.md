# Project Report: Implementation of Concurrent Database Operations and Snapshot Analysis

## 1. Introduction
The objective of this project was to implement a robust system for indexing files and snapshotting system processes using an SPMD (Single Program, Multiple Data) concurrency model. The final goal was to develop a utility (`db_diff`) to compare two database snapshots and generate detailed reports regarding file modifications or process state changes.

## 2. Technical Challenges and Solutions

### A. Concurrent Database Synchronization
* **Challenge:** During the development of `fileops_indexer` and `proc_snapshot`, we encountered resource contention when multiple instances attempted to write to the same database file simultaneously, resulting in the error: `Resource temporarily unavailable`.
* **Solution:** We implemented mandatory file locking using `fcntl` with `F_WRLCK`. By wrapping the write operations in a `while` loop that periodically retries if the lock is held (incorporating a `sleep` mechanism), we ensured data integrity and serialized database access without causing deadlocks.

### B. Binary Data Integrity and Alignment
* **Challenge:** A major issue was identified when comparing binary database headers between the indexer and the diff utility. We experienced `Segmentation fault` errors and incorrect database reads (e.g., `record_count` showing 0).
* **Solution:** We identified that structure padding (added by the compiler for memory alignment) caused different memory layouts between the writer and the reader. 
  * **Fix:** We applied the `__attribute__((packed))` directive to the `DatabaseHeader` and other data structures in both `fileops_indexer.c` and `main_db_diff.c`. This ensured that the structures are written and read bit-for-bit identically, regardless of the system's memory alignment rules.

### C. Data Persistence and Consistency
* **Challenge:** Even after successful execution, the `db_diff` tool would sometimes return empty reports or fail to read the record counts correctly.
* **Solution:**
  1. **Header Updates:** We verified that `record_count` was correctly incremented and flushed to the disk at the end of each indexing operation using `fseek` to return to the file start and `fflush` to force the write.
  2. **String Handling:** We resolved issues with variable-length path strings by explicitly allocating memory for the null-terminator (`malloc(path_len + 1)`) and manually assigning `abs_path[path_len] = '\0'`.
  3. **Field Synchronization:** We synchronized the read/write logic for metadata fields (`dev_t`, `ino_t`) in `db_diff.c` to match the exact order of operations in `fileops_indexer.c`, preventing cursor misalignment in the binary files.

### D. Integration and Automation
* **Challenge:** The provided infrastructure script (`fileops.sh`) would incorrectly interpret paths, returning `./bin/: Is a directory` when calling `run`.
* **Solution:** We discovered that the `do_run` function expected a double-dash (`--`) separator to correctly parse arguments. By updating the test script (`tests/t3_concurrency.sh`) to use the syntax `./tools/fileops.sh run -- <executable>`, we successfully automated the testing pipeline.

## 3. Conclusion
Through these iterations, we developed a functional system capable of safely handling concurrent database access and performing deep comparisons of snapshots. The project provided valuable insights into C system programming, binary file I/O, and the nuances of process synchronization in Linux environments.
