#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

int altura(Node *n) {
    return n ? n->altura : 0;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

Node *novo_no(Registro r) {
    Node *n = malloc(sizeof(Node));
    n->info = r;
    n->altura = 1;
    n->esq = n->dir = NULL;
    return n;
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

int fator_balanceamento(Node *n) {
    return altura(n->esq) - altura(n->dir);
}

Node* inserir(Node *raiz, Registro r) {
    if (!raiz) return novo_no(r);

    if (strcmp(r.data, raiz->info.data) < 0)
        raiz->esq = inserir(raiz->esq, r);
    else if (strcmp(r.data, raiz->info.data) > 0)
        raiz->dir = inserir(raiz->dir, r);
    else
        return raiz; // dados repetidos

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
    if (raiz) {
        em_ordem(raiz->esq);
        printf("%s | DR=%.2f | DC=%.2f | GD=%.2f | GT=%.2f | IMP=%.2f\n",
               raiz->info.data,
               raiz->info.demanda_residual,
               raiz->info.demanda_contratada,
               raiz->info.geracao_despachavel,
               raiz->info.geracao_termica,
               raiz->info.importacoes);
        em_ordem(raiz->dir);
    }
}