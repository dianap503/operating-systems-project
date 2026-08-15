---
name: T1_commands_documentation
description: Documentation for the 12 audit commands used in the T1 task.
---

# T1 Audit Commands Documentation

This document describes the 12 commands used for the T1 system audit, organized by category.

## A. Filesystem Commands

1. **List long format**
   - **Purpose**: Lists all files in the current directory with detailed information.
   - **Command**: `ls -all`
   - **Output File**: `reports/fs/ls_long.txt`
   - **Method**: Shell redirection (`>`).

2. **Find shell scripts**
   - **Purpose**: Locates all `.sh` files within the `./tema1` directory.
   - **Command**: `find ./tema1 -name *.sh`
   - **Output File**: `reports/fs/A2_find_sh.txt`
   - **Method**: Shell redirection (`>`).

3. **Check disk usage**
   - **Purpose**: Displays the size of files and directories in the current level.
 - **Command**: `du -sh *`
   - **Output File**: `reports/fs/A3_du_level1.txt`
   - **Method**: Shell redirection (`>`).

## B. Process-related Commands

1. **Top memory processes**
   - **Purpose**: Displays processes sorted by memory usage, showing the top 10.
   - **Command**: `ps aux | sort -k4 -nr | head -n 10`
   - **Output File**: `reports/process/B1_top_mem.txt`
   - **Method**: Shell redirection (`>`).

2. **Process tree**
   - **Purpose**: Displays the hierarchy of processes.
   - **Command**: `pstree -p`
   - **Output File**: `reports/process/B2_pstree.txt`
   - **Method**: Shell redirection (`>`).

3. **Test process monitoring**
   - **Purpose**: Starts a background process, identifies it, and records details.- **Command**: `sleep 60 &; PID=$!; ps aux | grep "[s]leep 60"; ps -p $PID; kill $PID`
   - **Output File**: `reports/process/B3_pgrep_sleep.txt`
   - **Method**: Shell redirection (`>`).

## C. Commands based on /proc

1. **Extract CPU model**
   - **Purpose**: Gets the model name of the CPU from `/proc/cpuinfo`.
   - **Command**: `lscpu | grep "Model name"`
   - **Output File**: `reports/proc/C1_cpu_model.txt`
   - **Method**: Shell redirection (`>`).

2. **Memory info**
   - **Purpose**: Extracts total and available memory information.
   - **Command**: `grep -E "MemTotal|MemAvailable" /proc/meminfo`
   - **Output File**: `reports/proc/C2_mem_total_avail.txt`
   - **Method**: Shell redirection (`>`).

3. **Uptime check**
   - **Purpose**: Gets the system uptime in seconds from `/proc/uptime`.- **Command**: `awk '{print $1}' /proc/uptime`
   - **Output File**: `reports/proc/C3_uptime.txt`
   - **Method**: Shell redirection (`>`).

## D. Pipeline Commands

1. **Top 5 large files/dirs**
   - **Purpose**: Finds the top 5 largest files or directories in the current folder.
   - **Command**: `du -sh * | sort -hr | head -n 5`
   - **Output File**: `reports/pipeline/D1_top5_large_files.txt`
   - **Method**: Shell redirection (`>`).

2. **Top memory processes (alternative)**
   - **Purpose**: Shows the top 5 processes by memory usage using a specific format.
   - **Command**: `ps -eo pid,cmd --sort=-%mem | head -n 6`
   - **Output File**: `reports/pipeline/D2_top_mem_processes.txt`
   - **Method**: Shell redirection (`>`).

3. **Count commands**
   - **Purpose**: Counts the number of unique command lines in the documentation file.
   - **Command**: `grep "^" doc/T1_commands.md | sort | wc -l`
   - **Output File**: `reports/pipeline/D3_count_commands.txt`
   - **Method**: Shell redirection (`>`).
