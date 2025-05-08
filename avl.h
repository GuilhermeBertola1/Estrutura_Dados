#ifndef AVL_H
#define AVL_H

typedef struct {
    char data[20];
    double consumo;
    double clientes;
    double receita_total;
    double preco_kwh;
    double uso_medio;
} Registro;

typedef struct Node {
    Registro info;
    int altura;
    struct Node *esq, *dir;
} Node;

Node* inserir(Node *raiz, Registro r);
void em_ordem(Node *raiz);
int altura(Node *n);

#endif
