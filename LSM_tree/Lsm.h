#ifndef LSM_TREE
#define LSM_TREE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MEMTABLE 1000

#define EPSILON_lsm 1e-9

extern int memtable_size;
extern int sstable_counter;

typedef struct {
    char data[24];
    double demanda_residual;
    double demanda_contratada;
    double geracao_despachavel;
    double geracao_termica;
    double importacoes;
    double geracao_renovavel_total;
    double carga_reduzida_manual;
    double capacidade_instalada;
    double perdas_geracao_total;
}Dados;

typedef struct {
    Dados *dados;   // array dinâmico
    size_t tamanho;    // quantidade usada
    size_t capacidade; // capacidade alocada
} VetorDados;

typedef struct {
    double media;
    double variancia;
    double desvio_padrao;
} EstatisticasSimpleslsm;

typedef struct {
    EstatisticasSimpleslsm demanda_residual;
    EstatisticasSimpleslsm demanda_contratada;
    EstatisticasSimpleslsm geracao_despachavel;
    EstatisticasSimpleslsm geracao_renovavel_total;
    EstatisticasSimpleslsm carga_reduzida_manual;
    EstatisticasSimpleslsm capacidade_instalada;
    EstatisticasSimpleslsm perdas_geracao_total;
    EstatisticasSimpleslsm geracao_termica;
    EstatisticasSimpleslsm importacoes;
} EstatisticasCamposlsm;

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
} Medianaslsm;

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
} Modaslsm;

extern Dados memtable[MAX_MEMTABLE];

char* get_lsm_directory();
long long datetime_para_inteiro_LSM(const char *datetime);
int comparar_dados(const void *a, const void *b);
void merge_sstables(const char *file1, const char *file2, const char *output);
void flush_memtable();
int inserir_dado(const Dados *dado);
void vetor_inicializar_lsm (VetorDados *v, size_t capacidade_inicial);
void vetor_adicionar_lsm (VetorDados *v, Dados reg);
void vetor_liberar_lsm (VetorDados *v);
void buscar_intervalo_lsm(const char *inicio_str, const char *fim_str, char **saida, VetorDados *vetor);

int eh_arquivo_sstable(const char *nome);
void printar_arquivo(const char *caminho_completo, const char *nome);
void printar_dados_todos_arquivos(const char *pasta);

EstatisticasCamposlsm calcular_estatisticas_lsm(VetorDados *v);
int comparador_double_lsm(const void *a, const void *b);
static double pegar_mediana_lsm(double *arr, size_t n);
static int doubles_iguais_lsm(double a, double b);
static double pegar_moda_lsm(double *arr, size_t n);
Medianaslsm calcular_mediana_lsm(VetorDados *v) ;
Modaslsm calcular_moda_lsm(VetorDados *v);
static int print_val_or_null(char *buffer, size_t size, double val);
char* estatisticas_para_json_conteudo_lsm(EstatisticasCamposlsm est, Medianaslsm med, Modaslsm moda);

#endif