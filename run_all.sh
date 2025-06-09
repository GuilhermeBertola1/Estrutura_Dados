#!/bin/sh
echo "========== Benchmark AVL =========="
./AVL_benchmark
/usr/bin/time -v ./AVL_benchmark 2>&1
perf stat ./AVL_benchmark 2>&1
valgrind --tool=massif ./AVL_benchmark 2>&1
echo "-----------------------------------"

echo "========== Benchmark BM =========="
./BM_benchmark
/usr/bin/time -v ./BM_benchmark 2>&1
perf stat ./BM_benchmark 2>&1
valgrind --tool=massif ./BM_benchmark 2>&1
echo "-----------------------------------"

echo "========== Benchmark Cuckoo Hash =========="
./CCH_benchmark
/usr/bin/time -v ./CCH_benchmark 2>&1
perf stat ./CCH_benchmark 2>&1
valgrind --tool=massif ./CCH_benchmark 2>&1
echo "-----------------------------------"

echo "========== Benchmark Compact List =========="
./CMP_benchmark
/usr/bin/time -v ./CMP_benchmark 2>&1
perf stat ./CMP_benchmark 2>&1
valgrind --tool=massif ./CMP_benchmark 2>&1
echo "-----------------------------------"

echo "========== Benchmark Hash Table =========="
./HT_benchmark
/usr/bin/time -v ./HT_benchmark 2>&1
perf stat ./HT_benchmark 2>&1
valgrind --tool=massif ./HT_benchmark 2>&1
echo "-----------------------------------"

echo "========== Benchmark LSM Tree =========="
./LSM_benchmark
/usr/bin/time -v ./LSM_benchmark 2>&1
perf stat ./LSM_benchmark 2>&1
valgrind --tool=massif ./LSM_benchmark 2>&1
echo "-----------------------------------"

echo "========== Benchmark Lista Encadeada =========="
./LT_benchmark
/usr/bin/time -v ./LT_benchmark 2>&1
perf stat ./LT_benchmark 2>&1
valgrind --tool=massif ./LT_benchmark 2>&1
echo "-----------------------------------"

echo "========== Benchmark Trie =========="
./TRIE_benchmark
/usr/bin/time -v ./TRIE_benchmark 2>&1
perf stat ./TRIE_benchmark 2>&1
valgrind --tool=massif ./TRIE_benchmark 2>&1
echo "-----------------------------------"
