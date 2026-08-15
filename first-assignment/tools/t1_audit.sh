#!/bin/bash

START_TIME=$(date +$s)
DATE_TIME=$(date +'%Y-%m-%d %H:%M:%S')

mkdir -p  reports/fs reports/pipeline reports/proc reports/process

# THE 12 MANDATORY COMMANDS
# A. Filesystem commands
# A1
ls -all > reports/fs/ls_long.txt

# A2
find . -name *.sh > reports/fs/A2_find_sh.txt

# A3
du -sh * > reports/fs/A3_du_level1.txt

# B. Process commands
# B1
ps aux | sort -k4 -nr | head -n 10 > reports/process/B1_top_mem.txt

# B2
pstree -p > reports/process/B2_pstree.txt

# B3
sleep 60 &
PID=$!
ps aux | grep "[s]leep 60" > reports/process/B3_pgrep_sleep.txt
ps -p $PID > reports/process/B3_pgrep_sleep.txt
kill $PID

# C. Commands based on /proc
# C1
lscpu | grep "Model name" > reports/proc/C1_cpu_model.txt

# C2
grep -E "MemTotal|MemAvailable" /proc/meminfo > reports/proc/C2_mem_total_avail.txt

# C3
awk '{print $1}' /proc/uptime > reports/proc/C3_uptime.txt

# D. Pipeline commands
# D1
du -sh * | sort -hr | head -n 5 > reports/pipeline/D1_top5_large_files.txt

# D2
ps -eo pid,cmd --sort=%mem | head -n 6 > reports/pipeline/D2_top5_proc_mem_pid_name.txt

# D3
grep "^/$" doc/T1_commands.md | sort | wc -l > reports/pipeline/D3_count_commands.txt

END_TIME=$(date +%s)
END_DATE=$(date +'%Y-%m-%d %H:%M:%S')
RUNTIME=$((END_TIME - START_TIME))

cat << EOF > reports/T1_summary.txt
T1 Audit Summary
Start Timestamp: $START_DATE
End Timestamp: $END_DATE
Total runtime: ${RUNTIME} seconds

Created Paths & Reports:
-reports/fs/ls_long.txt
-reports/fs/A2_find_sh.txt
-reports/fs/A3_du_level1.txt
-reports/process/B1_top_mem.txt
-reports/process/B2_pstree.txt
-reports/process/B3_pgrep_sleep.txt
-reports/proc/C1_cpu_model.txt
-reports/proc/C2_mem_total_avail.txt
-reports/proc/C3_uptime.txt
-reports/pipeline/D1_top5_large_files.txt
-reports/pipeline/D2_top_mem_processes.txt
-reports/pipeline/D3_count_commands.txt
EOF
