#ifndef AVL_H
#define AVL_H

#define MAX_NODES 10000

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
void buscar_intervalo(Node *raiz, const char *data_inicio, const char *data_fim, char **saida);
void liberar_avl(Node *raiz);

#endif