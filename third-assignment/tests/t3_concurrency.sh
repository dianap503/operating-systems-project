#!/bin/bash

# First we create the folders, and then we clean what was left from the test before

mkdir -p data reports test_dir
rm -f data/*.db reports/*.txt

# Then, we create a file in the test directory

echo "Hello World" > test_dir/hello.txt

# Concurrent execution of 3-5 instances of fileops_indexer, following the SPMD model 

echo "Testing the indexer (Fileops)."

./tools/fileops.sh run -- fileops_indexer --root test_dir --db data/index.db &
./tools/fileops.sh run -- fileops_indexer --root test_dir --db data/index.db &
./tools/fileops.sh run -- fileops_indexer --root test_dir --db data/index.db &
wait

# Saving the old snapshot

cp data/index.db data/old_index.db

# Modifying something on the disk

echo "Test" > test_dir.new_file.txt

# Testing the indexer again to generate a new snapshot

./tools/fileops.sh run -- fileops_indexer --root test_dir --db data/index.db
cp data/index.db data/new_index.db

# Testing db_diff

./tools/fileops.sh run -- db_diff --old data/old_index.db --new data/new_index.db --out reports/T3_filediff.txt

# Concurrent execution of 3-5 instances of proc_snapshot, following the SPMD model

echo "Testing the processes (Proc)."

./tools/fileops.sh run -- proc_snapshot --db data/proc.db &
./tools/fileops.sh run -- proc_snapshot --db data/proc.db &
./tools/fileops.sh run -- proc_snapshot --db data/proc.db &
wait

cp data/proc.db data/old_proc.db

sleep 2

./tools/fileops.sh run -- proc_snapshot --db data/proc.db
cp data/proc.db data/new_proc.db

./tools/fileops.sh run -- db_diff --old data/old_proc.db --new data/new_proc.db --out reports/T3_procdiff.txt

echo "Test completed! Check the reports/ directory for the reports."
