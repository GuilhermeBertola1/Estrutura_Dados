#ifndef COMPLIST_H
#define COMPLIST_H

#define BLOCO_TAM 256
#define MAX_STR_CMP 25

#define EPSILON_listc 1e-9

typedef struct {
    char data[MAX_STR_CMP];
    float demanda_residual;
    float demanda_contratada;
    float geracao_despachavel;
    float geracao_termica;
    float importacoes;
    float geracao_renovavel_total;
    float carga_reduzida_manual;
    float capacidade_instalada;
    float perdas_geracao_total;
} CompList;

typedef struct {
    CompList *dados;
    size_t qtd;
    size_t capacidade;
} VetorCompList;

typedef struct {
    double media;
    double variancia;
    double desvio_padrao;
} EstatisticasSimplesListC;

typedef struct {
    EstatisticasSimplesListC demanda_residual;
    EstatisticasSimplesListC demanda_contratada;
    EstatisticasSimplesListC geracao_despachavel;
    EstatisticasSimplesListC geracao_renovavel_total;
    EstatisticasSimplesListC carga_reduzida_manual;
    EstatisticasSimplesListC capacidade_instalada;
    EstatisticasSimplesListC perdas_geracao_total;
    EstatisticasSimplesListC geracao_termica;
    EstatisticasSimplesListC importacoes;
} EstatisticasCamposListC;

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
} MedianasListC;

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
} ModasListC;

typedef struct Bloco {
    CompList registros[BLOCO_TAM];
    int qtd;
    struct Bloco *prox;
} Bloco;

// Funções públicas
Bloco* criar_bloco();
void inserir_CMP(Bloco** lista, CompList dado);
void imprimir_lista_CMP(Bloco* lista);
void liberar_lista_CMP(Bloco* lista);
long long datetime_para_inteiro_CMP(const char *datetime);
void buscar_intervalo_lista_CMP(Bloco *lista, const char *inicio_str, const char *fim_str, char **saida, VetorCompList *vetor);

void vetor_inicializarListC (VetorCompList *v, size_t capacidade_inicial);
void vetor_adicionarListC (VetorCompList*v, CompList reg);
void vetor_liberarListC (VetorCompList *v);

EstatisticasCamposListC calcular_estatisticasListC (VetorCompList *v);
int comparador_double_listc(const void *a, const void *b);
static double pegar_mediana_listc(double *arr, size_t n);
static int doubles_iguais_listc(double a, double b);
static double pegar_moda_listc(double *arr, size_t n);
MedianasListC calcular_mediana_listc(VetorCompList *v);
ModasListC calcular_moda_listc(VetorCompList *v);
static int print_val_or_null(char *buffer, size_t size, double val);
char* estatisticas_para_json_conteudo_listc(EstatisticasCamposListC est, MedianasListC med, ModasListC moda);


#endif