#ifndef BPLUS_H
#define BPLUS_H

#include <stdbool.h>
#include <stdlib.h>

#define M 32
#define MAX_STR_BM 24
#define EPSILON_BM 1e-9

typedef struct {
    char data[MAX_STR_BM];
    double demanda_residual;
    double demanda_contratada;
    double geracao_despachavel;
    double geracao_termica;
    double importacoes;
    double geracao_renovavel_total;
    double carga_reduzida_manual;
    double capacidade_instalada;
    double perdas_geracao_total;
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

typedef struct {
    Bdados *dados;   // array dinâmico
    size_t tamanho;    // quantidade usada
    size_t capacidade; // capacidade alocada
} VetorBdados;

typedef struct {
    double media;
    double variancia;
    double desvio_padrao;
} EstatisticasSimples_BM;

typedef struct {
    EstatisticasSimples_BM demanda_residual;
    EstatisticasSimples_BM demanda_contratada;
    EstatisticasSimples_BM geracao_despachavel;
    EstatisticasSimples_BM geracao_renovavel_total;
    EstatisticasSimples_BM carga_reduzida_manual;
    EstatisticasSimples_BM capacidade_instalada;
    EstatisticasSimples_BM perdas_geracao_total;
    EstatisticasSimples_BM geracao_termica;
    EstatisticasSimples_BM importacoes;
} EstatisticasCampos_BM;

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
} Medianas_BM;

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
} Modas_BM;

// Funções para manipular lista de dados
void lista_inicializar(ListaDados *lista);
void lista_adicionar(ListaDados *lista, Bdados valor);
void lista_liberar(ListaDados *lista);

BPlusNode *criar_no(bool folha);
void inserir_bplus(BPlusNode **raiz, Bdados valor);
void imprimir_bplus(BPlusNode *raiz);
long long datetime_para_inteiro_LSM(const char *datetime);
void buscar_intervalo_bplus_json(BPlusNode *raiz, const char *inicio_str, const char *fim_str, char **saida, VetorBdados *vetor);
void liberar_bplus(BPlusNode *raiz);

void vetor_inicializar_BM(VetorBdados *v, size_t capacidade_inicial);
void vetor_adicionar_BM(VetorBdados *v, Bdados reg);
void vetor_liberar_BM(VetorBdados *v);

EstatisticasCampos_BM calcular_estatisticas_BM(VetorBdados *v);
Medianas_BM calcular_mediana_BM(VetorBdados *v);
Modas_BM calcular_moda_BM(VetorBdados *v);
char* estatisticas_para_json_conteudo_BM(EstatisticasCampos_BM est, Medianas_BM med, Modas_BM moda);

#endif