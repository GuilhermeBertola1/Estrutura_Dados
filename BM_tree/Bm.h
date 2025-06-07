#ifndef BPLUS_H
#define BPLUS_H

#include <stdbool.h>
#include <stdlib.h>

#define M 32
#define MAX_STR_BM 24

typedef struct {
    char data[MAX_STR_BM];
    float demanda_residual;
    float demanda_contratada;
    float geracao_despachavel;
    float geracao_termica;
    float importacoes;
    float geracao_renovavel_total;
    float carga_reduzida_manual;
    float capacidade_instalada;
    float perdas_geracao_total;
} Bdados;

// Lista dinâmica de Bdados para armazenar múltiplos dados por chave
typedef struct {
    Bdados *itens;
    int tamanho;
    int capacidade;
} ListaDados;

typedef struct BPlusNode {
    bool folha;
    int n;  // número de chaves armazenadas
    char chaves[M][MAX_STR_BM];

    union {
        struct {
            void *filhos[M + 1];
        };
        struct {
            ListaDados dados[M];  // para cada chave, uma lista de dados
            struct BPlusNode *prox;
        };
    };
} BPlusNode;

// Funções para manipular lista de dados
void lista_inicializar(ListaDados *lista);
void lista_adicionar(ListaDados *lista, Bdados valor);
void lista_liberar(ListaDados *lista);

BPlusNode *criar_no(bool folha);
void inserir_bplus(BPlusNode **raiz, Bdados valor);
void imprimir_bplus(BPlusNode *raiz);
long long datetime_para_inteiro_LSM(const char *datetime);
void buscar_intervalo_bplus_json(BPlusNode *raiz, const char *inicio_str, const char *fim_str, char **saida);

#endif