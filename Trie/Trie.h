#ifndef TRIE_H
#define TRIE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR_TRIE 64
#define MAX_CHAVES 256

typedef struct Entrada2 {
    char data[MAX_STR_TRIE];
    double demanda_residual;
    double demanda_contratada;
    double geracao_despachavel;
    double geracao_termica;
    double importacoes;
    double geracao_renovavel_total;
    double carga_reduzida_manual;
    double capacidade_instalada;
    double perdas_geracao_total;
    int ocupado;
} Entrada2;

typedef struct TrieNode {
    struct TrieNode *filhos[MAX_CHAVES];
    Entrada2 *dado;
    int eh_folha;
} TrieNode;

typedef struct {
    TrieNode *raiz;
} Trie;

Trie *criar_trie();
void inserir_trie_temporal(Trie *trie, Entrada2 valor);
void imprimir_trie(Trie *trie);
void destruir_trie(Trie *trie);
long long datetime_para_inteiro_TRIE(const char *datetime);
void buscar_intervalo_trie(Trie *trie, const char *inicio_str, const char *fim_str, char **saida);

#endif