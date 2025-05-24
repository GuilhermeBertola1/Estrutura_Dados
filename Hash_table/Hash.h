#ifndef HASH_LINEAR_H
#define HASH_LINEAR_H

#include <stddef.h>

#define MAX_STR2 1024
extern int TABLE_SIZE;

typedef struct {
    char chave[MAX_STR2];
    char data[MAX_STR2];
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
} Entrada;

int hashSimples(const char *chave, int tamanho);
unsigned long hashDJB2(const char *str);
void inicializarHash();
int inserirHash_linear_sem_rehash(const Entrada *e);
void rehash_linear();
int inserirHash_linear(const Entrada *e);
int removerHash_linear(const char *chave);
char *buscar_intervalo_linear(const char *inicio, const char *fim);
void estatisticasHash();
long long datetime_para_inteiro_linear(const char *data_str);
void converter_24h_para_12h_linear(const char *entrada, char *saida, size_t tamanho);

#endif