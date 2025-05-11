// avl.h
#ifndef AVL_H
#define AVL_H

typedef struct {
    char data[20];
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
    char data[20];
    ListaRegistro *registros;
    int altura;
    struct Node *esq, *dir;
} Node;

Node* inserir(Node *raiz, Registro r);
void em_ordem(Node *raiz);

#endif