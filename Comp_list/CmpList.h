#ifndef COMPLIST_H
#define COMPLIST_H

#define BLOCO_TAM 256
#define MAX_STR_CMP 25

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
void buscar_intervalo_lista_CMP(Bloco *lista, const char *inicio_str, const char *fim_str, char **saida);

#endif