#!/bin/sh
echo "========== Benchmark AVL =========="
./AVL_benchmark
/usr/bin/time -v ./AVL_benchmark 2>&1
valgrind --tool=massif --massif-out-file=/dev/stdout ./AVL_benchmark 2>&1 | \
  grep mem_heap_B= | sed 's/mem_heap_B=//' | sort -nr | head -1 | \
  xargs -I{} echo "Pico maximo memoria heap: {} bytes"
echo "-----------------------------------"

echo "========== Benchmark BM =========="
./BM_benchmark
/usr/bin/time -v ./BM_benchmark 2>&1
valgrind --tool=massif --massif-out-file=/dev/stdout ./BM_benchmark  2>&1 | \
  grep mem_heap_B= | sed 's/mem_heap_B=//' | sort -nr | head -1 | \
  xargs -I{} echo "Pico maximo memoria heap: {} bytes"
echo "-----------------------------------"

echo "========== Benchmark Cuckoo Hash =========="
./CCH_benchmark
/usr/bin/time -v ./CCH_benchmark 2>&1
valgrind --tool=massif --massif-out-file=/dev/stdout ./CCH_benchmark  2>&1 | \
  grep mem_heap_B= | sed 's/mem_heap_B=//' | sort -nr | head -1 | \
  xargs -I{} echo "Pico maximo memoria heap: {} bytes"
echo "-----------------------------------"

echo "========== Benchmark Compact List =========="
./CMP_benchmark
/usr/bin/time -v ./CMP_benchmark 2>&1
valgrind --tool=massif --massif-out-file=/dev/stdout ./CMP_benchmark  2>&1 | \
  grep mem_heap_B= | sed 's/mem_heap_B=//' | sort -nr | head -1 | \
  xargs -I{} echo "Pico maximo memoria heap: {} bytes"
echo "-----------------------------------"

echo "========== Benchmark Hash Table =========="
./HT_benchmark
/usr/bin/time -v ./HT_benchmark 2>&1
valgrind --tool=massif --massif-out-file=/dev/stdout ./HT_benchmark  2>&1 | \
  grep mem_heap_B= | sed 's/mem_heap_B=//' | sort -nr | head -1 | \
  xargs -I{} echo "Pico maximo memoria heap: {} bytes"
echo "-----------------------------------"

echo "========== Benchmark LSM Tree =========="
./LSM_benchmark
/usr/bin/time -v ./LSM_benchmark 2>&1
valgrind --tool=massif --massif-out-file=/dev/stdout ./LSM_benchmark  2>&1 | \
  grep mem_heap_B= | sed 's/mem_heap_B=//' | sort -nr | head -1 | \
  xargs -I{} echo "Pico maximo memoria heap: {} bytes"
echo "-----------------------------------"

echo "========== Benchmark Lista Encadeada =========="
./LT_benchmark
/usr/bin/time -v ./LT_benchmark 2>&1
valgrind --tool=massif --massif-out-file=/dev/stdout ./LT_benchmark  2>&1 | \
  grep mem_heap_B= | sed 's/mem_heap_B=//' | sort -nr | head -1 | \
  xargs -I{} echo "Pico maximo memoria heap: {} bytes"
echo "-----------------------------------"

echo "========== Benchmark Trie =========="
./TRIE_benchmark
/usr/bin/time -v ./TRIE_benchmark 2>&1
valgrind --tool=massif --massif-out-file=/dev/stdout ./TRIE_benchmark  2>&1 | \
  grep mem_heap_B= | sed 's/mem_heap_B=//' | sort -nr | head -1 | \
  xargs -I{} echo "Pico maximo memoria heap: {} bytes"
echo "-----------------------------------"
