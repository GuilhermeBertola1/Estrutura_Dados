#ifndef CUCKOO_HASH_H
#define CUCKOO_HASH_H

#include <stddef.h>  // para size_t

#define MAX_STR 2048
#define EPSILON_cc 1e-9

typedef struct {
    char data[MAX_STR];
    float demanda_residual;
    float demanda_contratada;
    float geracao_despachavel;
    float geracao_termica;
    float importacoes;
    float geracao_renovavel_total;
    float carga_reduzida_manual;
    float capacidade_instalada;
    float perdas_geracao_total;
} Registro1;

typedef struct {
    Registro1 *dados;
    size_t tamanho;
    size_t capacidade;
} VetorRegistro1;

typedef struct {
    double media;
    double variancia;
    double desvio_padrao;
} EstatisticasSimplesCC;

typedef struct {
    EstatisticasSimplesCC demanda_residual;
    EstatisticasSimplesCC demanda_contratada;
    EstatisticasSimplesCC geracao_despachavel;
    EstatisticasSimplesCC geracao_renovavel_total;
    EstatisticasSimplesCC carga_reduzida_manual;
    EstatisticasSimplesCC capacidade_instalada;
    EstatisticasSimplesCC perdas_geracao_total;
    EstatisticasSimplesCC geracao_termica;
    EstatisticasSimplesCC importacoes;
} EstatisticasCamposCC;

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
} MedianasCC;

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
} ModasCC;

typedef struct {
    char chave[MAX_STR];
    Registro1 valor;
    int ocupado;
} CuckooEntry;

extern CuckooEntry *tabela1;
extern CuckooEntry *tabela2;
extern size_t tamanho_atual;
extern size_t quantidade_itens;

unsigned int hash1(const char *str);
unsigned int hash2(const char *str);
void inicializarCuckoo(size_t tamanho_inicial);
void liberarCuckoo(void);
int existe_na_tabela(const char *chave);
void rehash(size_t novo_tamanho);
int inserirCuckoo(const Registro1 *r);
long long datetime_para_inteiro_cuck(const char *datetime);
void vetor_inicializar_cc(VetorRegistro1 *v, size_t capacidade_inicial);
void vetor_adicionar_cc(VetorRegistro1 *v, Registro1 reg);
void vetor_liberar_cc(VetorRegistro1 *v);
void buscar_intervalo_cuckoo(const char *data_inicio, const char *data_fim, char **saida, VetorRegistro1 *vetor);
void exibirCuckoo();
EstatisticasCamposCC calcular_estatisticas_cc (VetorRegistro1 *v);
int comparador_double_cc(const void *a, const void *b);
static double pegar_mediana_cc(double *arr, size_t n);
static int doubles_iguais_cc(double a, double b);
static double pegar_moda_cc(double *arr, size_t n);
MedianasCC calcular_mediana_cc(VetorRegistro1 *v);
ModasCC calcular_moda_cc(VetorRegistro1 *v);
static int print_val_or_null(char *buffer, size_t size, double val);
char* estatisticas_para_json_conteudo_cc(EstatisticasCamposCC est, MedianasCC med, ModasCC moda);

#endif // CUCKOO_HASH_H