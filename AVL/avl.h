#ifndef AVL_H
#define AVL_H

#define MAX_NODES 10000
#define EPSILON 1e-9

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
} Registro;

typedef struct {
    Registro *dados;   // array dinâmico
    size_t tamanho;    // quantidade usada
    size_t capacidade; // capacidade alocada
} VetorRegistros;

typedef struct {
    double media;
    double variancia;
    double desvio_padrao;
} EstatisticasSimples;

typedef struct {
    EstatisticasSimples demanda_residual;
    EstatisticasSimples demanda_contratada;
    EstatisticasSimples geracao_despachavel;
    EstatisticasSimples geracao_renovavel_total;
    EstatisticasSimples carga_reduzida_manual;
    EstatisticasSimples capacidade_instalada;
    EstatisticasSimples perdas_geracao_total;
    EstatisticasSimples geracao_termica;
    EstatisticasSimples importacoes;
} EstatisticasCampos;

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
} Medianas;

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
} Modas;

typedef struct ListaRegistro {
    Registro info;
    struct ListaRegistro *prox;
} ListaRegistro;

typedef struct Node {
    char data[24];
    ListaRegistro *registros;
    int altura;
    struct Node *esq, *dir;
} Node;

extern Node* vetor_nos[MAX_NODES];
extern int contador;

Node* inserir(Node *raiz, Registro r);
void imprimir_AVL(Node *raiz);

long long datetime_para_inteiro(const char *datetime);
void vetor_inicializar(VetorRegistros *v, size_t capacidade_inicial);
void vetor_adicionar(VetorRegistros *v, Registro reg);
void vetor_liberar(VetorRegistros *v);
void buscar_intervalo(Node *raiz, const char *data_inicio, const char *data_fim, char **saida, VetorRegistros *vetor);
void liberar_avl(Node *raiz);

EstatisticasCampos calcular_estatisticas(VetorRegistros *v);
Medianas calcular_mediana(VetorRegistros *v);
Modas calcular_moda(VetorRegistros *v);
char* estatisticas_para_json_conteudo(EstatisticasCampos est, Medianas med, Modas moda);

#endif