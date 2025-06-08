FROM debian:bookworm-slim

# Instala dependências necessárias para compilar C e libzmq
RUN apt update && apt install -y build-essential libzmq3-dev

WORKDIR /app

# Copia somente os arquivos C e pastas necessárias para compilar o programa
COPY main.c ./
COPY AVL/ ./AVL
COPY Cuckoo_Hashing/ ./Cuckoo_Hashing
COPY BM_tree/ ./BM_tree
COPY Hash_table/ ./Hash_table
COPY Lista_Encadeada/ ./Lista_Encadeada
COPY LSM_tree/ ./LSM_tree
COPY Trie/ ./Trie
COPY Comp_list ./Comp_list

# Compila o programa C
RUN gcc main.c \
    AVL/avl.c \
    Cuckoo_Hashing/CcH.c \
    Hash_table/Hash.c \
    Lista_Encadeada/List.c \
    LSM_tree/Lsm.c \
    BM_tree/Bm.c \
    Trie/Trie.c \
    Comp_list/CmpList.c \
    -o programa \
    -lzmq -lm
