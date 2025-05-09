#ifndef AVL_H
#define AVL_H

typedef struct {
    char data[20];
    double demanda_residual;
    double demanda_contratada;
    double geracao_despachavel;
    double geracao_termica;
    double importacoes;
} Registro;

typedef struct Node {
    Registro info;
    int altura;
    struct Node *esq, *dir;
} Node;

Node* inserir(Node *raiz, Registro r);
void em_ordem(Node *raiz);

#endif