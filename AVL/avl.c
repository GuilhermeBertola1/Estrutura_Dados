#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

Node* vetor_nos[MAX_NODES];
int contador = 0;

int altura(Node *n) {
    return n ? n->altura : 0;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

ListaRegistro* novo_registro(Registro r) {
    ListaRegistro *lr = malloc(sizeof(ListaRegistro));
    lr->info = r;
    lr->prox = NULL;
    return lr;
}

Node *novo_no(Registro r) {
    Node *n = malloc(sizeof(Node));
    strncpy(n->data, r.data, sizeof(n->data));
    n->registros = novo_registro(r);
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

    int cmp = strcmp(r.data, raiz->data);

    if (cmp < 0)
        raiz->esq = inserir(raiz->esq, r);
    else if (cmp > 0)
        raiz->dir = inserir(raiz->dir, r);
    else {
        ListaRegistro *novo = novo_registro(r);
        novo->prox = raiz->registros;
        raiz->registros = novo;
        return raiz;
    }

    raiz->altura = 1 + max(altura(raiz->esq), altura(raiz->dir));

    int fb = fator_balanceamento(raiz);

    if (fb > 1 && strcmp(r.data, raiz->esq->data) < 0)
        return rotacao_direita(raiz);
    if (fb < -1 && strcmp(r.data, raiz->dir->data) > 0)
        return rotacao_esquerda(raiz);
    if (fb > 1 && strcmp(r.data, raiz->esq->data) > 0) {
        raiz->esq = rotacao_esquerda(raiz->esq);
        return rotacao_direita(raiz);
    }
    if (fb < -1 && strcmp(r.data, raiz->dir->data) < 0) {
        raiz->dir = rotacao_direita(raiz->dir);
        return rotacao_esquerda(raiz);
    }
    return raiz;
}

void em_ordem(Node *raiz) {
    if (raiz) {
        em_ordem(raiz->esq);

        printf("\n%s:\n", raiz->data);
        ListaRegistro *lr = raiz->registros;
        while (lr) {
            printf("  DR=%.2f | DC=%.2f | GD=%.2f | GT=%.2f | IMP=%.2f | RE=%.2f | MLR=%.2f | CAP=%.2f | UO=%.2f\n",
                   lr->info.demanda_residual,
                   lr->info.demanda_contratada,
                   lr->info.geracao_despachavel,
                   lr->info.geracao_termica,
                   lr->info.importacoes,
                   lr->info.geracao_renovavel_total,
                   lr->info.carga_reduzida_manual,
                   lr->info.capacidade_instalada,
                   lr->info.perdas_geracao_total);
            lr = lr->prox;
        }

        em_ordem(raiz->dir);
    }
}

void preencher_vetor_nos(Node *raiz) {
    if (!raiz) return;
    preencher_vetor_nos(raiz->esq);
    if (contador < MAX_NODES) {
        vetor_nos[contador++] = raiz;
    }
    preencher_vetor_nos(raiz->dir);
}

void registro_to_json_completo(Node *no, char *buffer, size_t size) {
    if (!no || !no->registros) {
        snprintf(buffer, size, "{\n  \"erro\": \"nó ou registro vazio\"\n}");
        return;
    }

    Registro *r = &(no->registros->info);
    snprintf(buffer, size,
        "{\n"
        "  \"data\": \"%s\",\n"
        "  \"demanda_residual\": %.2f,\n"
        "  \"demanda_contratada\": %.2f,\n"
        "  \"geracao_despachavel\": %.2f,\n"
        "  \"geracao_termica\": %.2f,\n"
        "  \"importacoes\": %.2f,\n"
        "  \"geracao_renovavel_total\": %.2f,\n"
        "  \"carga_reduzida_manual\": %.2f,\n"
        "  \"capacidade_instalada\": %.2f,\n"
        "  \"perdas_geracao_total\": %.2f\n"
        "}",
        no->data,
        r->demanda_residual,
        r->demanda_contratada,
        r->geracao_despachavel,
        r->geracao_termica,
        r->importacoes,
        r->geracao_renovavel_total,
        r->carga_reduzida_manual,
        r->capacidade_instalada,
        r->perdas_geracao_total
    );
}