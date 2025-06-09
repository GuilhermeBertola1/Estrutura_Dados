CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g \
    -IAVL -IBenchmark -IBM_tree -IComp_list -ICuckoo_Hashing -IHash_table -ILista_Encadeada -ILSM_tree -ITrie

BIN_DIR = bin

ifeq ($(OS),Windows_NT)
    MKDIR = mkdir
    RM = rmdir /S /Q
else
    MKDIR = mkdir -p
    RM = rm -rf
endif

# Benchmarks a serem compilados
BENCHMARKS = \
    Benchmark_AVL \
    Benchmark_HT \
    Benchmark_CCH \
    Benchmark_LSM \
    Benchmark_BM \
    Benchmark_CMP \
    Benchmark_LT \
    Benchmark_TRIE

# Dependências específicas de cada benchmark
Benchmark_AVL_DEPS = AVL/avl.c
Benchmark_HT_DEPS = Hash_table/Hash.c
Benchmark_CCH_DEPS = Cuckoo_Hashing/CcH.c
Benchmark_LSM_DEPS = LSM_tree/Lsm.c
Benchmark_BM_DEPS = BM_tree/Bm.c
Benchmark_CMP_DEPS = Comp_list/CmpList.c
Benchmark_LT_DEPS = Lista_Encadeada/List.c
Benchmark_TRIE_DEPS = Trie/Trie.c

all: $(BIN_DIR) $(addprefix $(BIN_DIR)/,$(BENCHMARKS))

$(BIN_DIR):
	$(MKDIR) $(BIN_DIR)

# Regra de compilação gerada dinamicamente
define COMPILE_RULE
$(BIN_DIR)/$(1): Benchmark/$(1).c $$($(1)_DEPS)
	$$(CC) $$(CFLAGS) Benchmark/$(1).c $$($(1)_DEPS) -o $$@
endef

# Aplica a regra para cada benchmark
$(foreach bench,$(BENCHMARKS),$(eval $(call COMPILE_RULE,$(bench))))

clean:
	$(RM) $(BIN_DIR)

.PHONY: all clean