#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

void imprimir_AVL(Node *raiz) {
    if (raiz) {
        imprimir_AVL(raiz->esq);

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

        imprimir_AVL(raiz->dir);
    }
}

long long datetime_para_inteiro(const char *datetime) {
    int ano, mes, dia, hora, min, seg;
    char ampm[3] = "";

    // Verifica se tem AM/PM no final
    if (strstr(datetime, "AM") || strstr(datetime, "PM") || strstr(datetime, "am") || strstr(datetime, "pm")) {
        sscanf(datetime, "%d-%d-%d %d:%d:%d %2s", &ano, &mes, &dia, &hora, &min, &seg, ampm);

        if ((strcmp(ampm, "PM") == 0 || strcmp(ampm, "pm") == 0) && hora != 12)
            hora += 12;
        else if ((strcmp(ampm, "AM") == 0 || strcmp(ampm, "am") == 0) && hora == 12)
            hora = 0;
    } else {
        sscanf(datetime, "%d-%d-%d %d:%d:%d", &ano, &mes, &dia, &hora, &min, &seg);
    }

    return (long long)ano * 10000000000LL +
           (long long)mes * 100000000 +
           (long long)dia * 1000000 +
           (long long)hora * 10000 +
           (long long)min * 100 +
           (long long)seg;
}

// Função recursiva para coletar registros entre data_inicio e data_fim, montando JSON no buffer
static void buscar_intervalo_rec(Node *no, long long inicio, long long fim,
                                 char **resultado, size_t *tamanho, size_t *usado, int *primeiro) {
    if (!no) return;

    long long data_no = datetime_para_inteiro(no->data);

    if (data_no > inicio)
        buscar_intervalo_rec(no->esq, inicio, fim, resultado, tamanho, usado, primeiro);

    if (data_no >= inicio && data_no <= fim) {
        ListaRegistro *lr = no->registros;
        while (lr) {
            char item[2048];
            int n = snprintf(item, sizeof(item),
                "%s{\n"
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
                (*primeiro) ? "" : ",\n",
                lr->info.data,
                lr->info.demanda_residual,
                lr->info.demanda_contratada,
                lr->info.geracao_despachavel,
                lr->info.geracao_termica,
                lr->info.importacoes,
                lr->info.geracao_renovavel_total,
                lr->info.carga_reduzida_manual,
                lr->info.capacidade_instalada,
                lr->info.perdas_geracao_total
            );

            if (*usado + n + 1 >= *tamanho) {
                *tamanho *= 2;
                *resultado = realloc(*resultado, *tamanho);
                if (!*resultado) {
                    perror("realloc");
                    exit(1);
                }
            }

            memcpy(*resultado + *usado, item, n);
            *usado += n;
            (*resultado)[*usado] = '\0';
            *primeiro = 0;

            lr = lr->prox;
        }
    }

    if (data_no < fim)
        buscar_intervalo_rec(no->dir, inicio, fim, resultado, tamanho, usado, primeiro);
}

void buscar_intervalo(Node *raiz, const char *data_inicio, const char *data_fim, char **saida) {
    if (!raiz) {
        *saida = strdup("[]");
        return;
    }

    long long inicio = datetime_para_inteiro(data_inicio);
    long long fim = datetime_para_inteiro(data_fim);
    int primeiro = 1;

    size_t capacidade = 8192;
    size_t usado = 0;
    char *tmp_buffer = malloc(capacidade);
    if (!tmp_buffer) {
        perror("malloc");
        exit(1);
    }

    usado = snprintf(tmp_buffer, capacidade, "[");
    buscar_intervalo_rec(raiz, inicio, fim, &tmp_buffer, &capacidade, &usado, &primeiro);

    if (usado + 2 >= capacidade) {
        capacidade += 2;
        tmp_buffer = realloc(tmp_buffer, capacidade);
        if (!tmp_buffer) {
            perror("realloc");
            exit(1);
        }
    }

    tmp_buffer[usado++] = ']';
    tmp_buffer[usado] = '\0';

    *saida = tmp_buffer; // Retorna ponteiro para o buffer alocado
}

void liberar_avl(Node *raiz) {
    if (raiz == NULL) return;
    liberar_avl(raiz->esq);
    liberar_avl(raiz->dir);
    free(raiz);
}