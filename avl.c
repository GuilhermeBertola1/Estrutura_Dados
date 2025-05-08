#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

int max(int a, int b) {
    return (a > b) ? a : b;
}

int altura(Node *n) {
    return n ? n->altura : 0;
}

Node *novo_no(Registro r) {
    Node *n = (Node*) malloc(sizeof(Node));
    n->info = r;
    n->altura = 1;
    n->esq = n->dir = NULL;
    return n;
}

int fator_balanceamento(Node *n) {
    return altura(n->esq) - altura(n->dir);
}

Node *rotacao_direita(Node *y) {
    Node *x = y->esq;
    Node *T2 = x->dir;

    x->dir = y;
    y->esq = T2;

    y->altura = max(altura(y->esq), altura(y->dir)) + 1;
    x->altura = max(altura(x->esq), altura(x->dir)) + 1;

    return x;
}

Node *rotacao_esquerda(Node *x) {
    Node *y = x->dir;
    Node *T2 = y->esq;

    y->esq = x;
    x->dir = T2;

    x->altura = max(altura(x->esq), altura(x->dir)) + 1;
    y->altura = max(altura(y->esq), altura(y->dir)) + 1;

    return y;
}

Node* inserir(Node *raiz, Registro r) {
    if (raiz == NULL)
        return novo_no(r);

    if (strcmp(r.data, raiz->info.data) < 0)
        raiz->esq = inserir(raiz->esq, r);
    else if (strcmp(r.data, raiz->info.data) > 0)
        raiz->dir = inserir(raiz->dir, r);
    else
        return raiz;

    raiz->altura = 1 + max(altura(raiz->esq), altura(raiz->dir));

    int fb = fator_balanceamento(raiz);

    if (fb > 1 && strcmp(r.data, raiz->esq->info.data) < 0)
        return rotacao_direita(raiz);

    if (fb < -1 && strcmp(r.data, raiz->dir->info.data) > 0)
        return rotacao_esquerda(raiz);

    if (fb > 1 && strcmp(r.data, raiz->esq->info.data) > 0) {
        raiz->esq = rotacao_esquerda(raiz->esq);
        return rotacao_direita(raiz);
    }

    if (fb < -1 && strcmp(r.data, raiz->dir->info.data) < 0) {
        raiz->dir = rotacao_direita(raiz->dir);
        return rotacao_esquerda(raiz);
    }

    return raiz;
}

void em_ordem(Node *raiz) {
    if (raiz != NULL) {
        em_ordem(raiz->esq);
        printf("%s: %.2f kWh, %d clientes\n", raiz->info.data, raiz->info.consumo, raiz->info.clientes);
        em_ordem(raiz->dir);
    }
}
