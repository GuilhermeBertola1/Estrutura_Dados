FROM debian:bookworm-slim

# Instala dependências necessárias para compilar C e libzmq
RUN apt update && apt install -y build-essential libzmq3-dev

WORKDIR /app

# Copia somente os arquivos C e pastas necessárias para compilar o programa
COPY main.c ./
COPY AVL/ ./AVL
COPY Cuckoo_Hashing/ ./Cuckoo_Hashing
COPY Fenwick_Tree/ ./Fenwick_Tree
COPY Hash_table/ ./Hash_table
COPY Lista_Encadeada/ ./Lista_Encadeada
COPY Segment_Tree/ ./Segment_Tree
COPY Trie/ ./Trie

# Compila o programa C
RUN gcc main.c \
    AVL/avl.c \
    Cuckoo_Hashing/CcH.c \
    Hash_table/Hash.c \
    Lista_Encadeada/List.c \
    -o programa \
    -lzmq -lm
