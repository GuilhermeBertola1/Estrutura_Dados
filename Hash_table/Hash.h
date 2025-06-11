#ifndef HASH_LINEAR_H
#define HASH_LINEAR_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define EPSILON_HT 1e-9

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

typedef struct {
    Entrada *dados;   // array dinâmico
    size_t tamanho;    // quantidade usada
    size_t capacidade; // capacidade alocada
} VetorEntrada;

typedef struct {
    double media;
    double variancia;
    double desvio_padrao;
} EstatisticasSimplesHT;

typedef struct {
    EstatisticasSimplesHT demanda_residual;
    EstatisticasSimplesHT demanda_contratada;
    EstatisticasSimplesHT geracao_despachavel;
    EstatisticasSimplesHT geracao_renovavel_total;
    EstatisticasSimplesHT carga_reduzida_manual;
    EstatisticasSimplesHT capacidade_instalada;
    EstatisticasSimplesHT perdas_geracao_total;
    EstatisticasSimplesHT geracao_termica;
    EstatisticasSimplesHT importacoes;
} EstatisticasCamposHT;

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
} MedianasHT;

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
} ModasHT;

void vetor_inicializar_HT(VetorEntrada *v, size_t capacidade_inicial);
void vetor_adicionar_HT(VetorEntrada *v, Entrada reg);
void vetor_liberar_HT(VetorEntrada *v);

unsigned long hashDJB2(const char *str);
int hashSimples(const char *chave, int tamanho);
void inicializarHash();
int inserirHash_linear_sem_rehash(const Entrada *e);
void rehash_linear();
int inserirHash_linear(const Entrada *e);
int removerHash_linear(const char *chave);
long long datetime_para_inteiro_linear(const char *data_str);
void buscar_intervalo_HT(const char *inicio_str, const char *fim_str, char **saida, VetorEntrada *vetor);
void estatisticasHash();
void liberarHashLinear();

EstatisticasCamposHT calcular_estatisticas_HT (VetorEntrada *v);
int comparador_double_HT(const void *a, const void *b);
static double pegar_mediana_HT(double *arr, size_t n);
static int doubles_iguais_HT(double a, double b);
static double pegar_moda_HT(double *arr, size_t n);
MedianasHT calcular_mediana_HT(VetorEntrada *v);
ModasHT calcular_moda_HT(VetorEntrada *v);
static int print_val_or_null(char *buffer, size_t size, double val);
char* estatisticas_para_json_conteudo_HT(EstatisticasCamposHT est, MedianasHT med, ModasHT moda);

#endif