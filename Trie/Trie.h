#ifndef TRIE_H
#define TRIE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR_TRIE 64
#define MAX_CHAVES 256

#define EPSILON_Trie 1e-9

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

typedef struct {
    Entrada2 *dados;   // array dinâmico
    size_t tamanho;    // quantidade usada
    size_t capacidade; // capacidade alocada
} VetorEntrada2;

typedef struct {
    double media;
    double variancia;
    double desvio_padrao;
} EstatisticasSimplesTrie;

typedef struct {
    EstatisticasSimplesTrie demanda_residual;
    EstatisticasSimplesTrie demanda_contratada;
    EstatisticasSimplesTrie geracao_despachavel;
    EstatisticasSimplesTrie geracao_renovavel_total;
    EstatisticasSimplesTrie carga_reduzida_manual;
    EstatisticasSimplesTrie capacidade_instalada;
    EstatisticasSimplesTrie perdas_geracao_total;
    EstatisticasSimplesTrie geracao_termica;
    EstatisticasSimplesTrie importacoes;
} EstatisticasCamposTrie;

typedef struct {
    double demanda_residual;
    double demanda_contratada;
    double geracao_despachavel;
    double geracao_renovavel_total;
    double carga_reduzida_manual;
    double capacidade_instalada;
    double perdas_geracao_total;
    double geracao_termica;
    double importacoes;
} MedianasTrie;

typedef struct {
    double demanda_residual;
    double demanda_contratada;
    double geracao_despachavel;
    double geracao_renovavel_total;
    double carga_reduzida_manual;
    double capacidade_instalada;
    double perdas_geracao_total;
    double geracao_termica;
    double importacoes;
} ModasTrie;

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
void vetor_inicializar_Trie(VetorEntrada2 *v, size_t capacidade_inicial);
void vetor_adicionar_Trie(VetorEntrada2 *v, Entrada2 reg);
void vetor_liberar_Trie(VetorEntrada2 *v);
void buscar_intervalo_trie_rec(TrieNode *no, char *prefixo, int nivel, long long inicio, long long fim, char **json, size_t *usado, size_t *capacidade, int *primeiro, int *contagem, VetorEntrada2 *vetor);
void buscar_intervalo_trie(Trie *trie, const char *inicio_str, const char *fim_str, char **saida, VetorEntrada2 *vetor);


EstatisticasCamposTrie calcular_estatisticas_Trie(VetorEntrada2 *v);
int comparador_double_Trie(const void *a, const void *b);
static double pegar_mediana_Trie(double *arr, size_t n);
static int doubles_iguais_Trie(double a, double b);
static double pegar_moda_Trie(double *arr, size_t n);
MedianasTrie calcular_mediana_Trie(VetorEntrada2 *v);
ModasTrie calcular_moda_Trie(VetorEntrada2 *v) ;
static int print_val_or_null(char *buffer, size_t size, double val);
char* estatisticas_para_json_conteudo_Trie(EstatisticasCamposTrie est, MedianasTrie med, ModasTrie moda);

#endif