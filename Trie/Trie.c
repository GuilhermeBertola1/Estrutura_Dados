#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Trie.h"

TrieNode* criar_no_trie() {
    TrieNode *no = malloc(sizeof(TrieNode));
    for (int i = 0; i < MAX_CHAVES; i++)
        no->filhos[i] = NULL;
    no->dado = NULL;
    no->eh_folha = 0;
    return no;
}

Trie* criar_trie() {
    Trie *t = malloc(sizeof(Trie));
    t->raiz = criar_no_trie();
    return t;
}

void inserir_trie_temporal(Trie *trie, Entrada2 valor) {
    const char *chave = valor.data;
    TrieNode *atual = trie->raiz;

    for (int i = 0; chave[i] != '\0'; i++) {
        unsigned char c = (unsigned char)chave[i];
        if (atual->filhos[c] == NULL) {
            atual->filhos[c] = criar_no_trie();
        }
        atual = atual->filhos[c];
    }

    if (atual->dado != NULL) {
        free(atual->dado);
    }

    Entrada2 *novo = malloc(sizeof(Entrada2));
    *novo = valor;
    atual->dado = novo;
    atual->eh_folha = 1;
}

void imprimir_trie_rec(TrieNode *no, char *prefixo, int nivel) {
    if (no == NULL) return;

    if (no->eh_folha && no->dado) {
        printf("Data: %s\n", no->dado->data);
        printf("  Demanda Residual: %.2f\n", no->dado->demanda_residual);
        printf("  Demanda Contratada: %.2f\n", no->dado->demanda_contratada);
        printf("  G. Despachavel: %.2f\n", no->dado->geracao_despachavel);
        printf("  G. Termica: %.2f\n", no->dado->geracao_termica);
        printf("  Importacoes: %.2f\n", no->dado->importacoes);
        printf("  G. Renovavel Total: %.2f\n", no->dado->geracao_renovavel_total);
        printf("  Carga Reduzida Manual: %.2f\n", no->dado->carga_reduzida_manual);
        printf("  Capacidade Instalada: %.2f\n", no->dado->capacidade_instalada);
        printf("  Perdas Geracao Total: %.2f\n", no->dado->perdas_geracao_total);
        printf("  Ocupado: %d\n\n", no->dado->ocupado);
    }

    for (int i = 0; i < MAX_CHAVES; i++) {
        if (no->filhos[i]) {
            prefixo[nivel] = (char)i;
            prefixo[nivel + 1] = '\0';
            imprimir_trie_rec(no->filhos[i], prefixo, nivel + 1);
        }
    }
}

void imprimir_trie(Trie *trie) {
    char prefixo[256] = {0};
    imprimir_trie_rec(trie->raiz, prefixo, 0);
}

void destruir_trie_rec(TrieNode *no) {
    if (!no) return;
    for (int i = 0; i < MAX_CHAVES; i++) {
        if (no->filhos[i]) destruir_trie_rec(no->filhos[i]);
    }
    if (no->dado) free(no->dado);
    free(no);
}

void destruir_trie(Trie *trie) {
    destruir_trie_rec(trie->raiz);
    free(trie);
}

long long datetime_para_inteiro_TRIE(const char *datetime) {
    int ano, mes, dia, hora, min, seg;
    sscanf(datetime, "%d-%d-%d %d:%d:%d", &ano, &mes, &dia, &hora, &min, &seg);
    return (long long)ano * 10000000000LL +
           (long long)mes * 100000000 +
           (long long)dia * 1000000 +
           (long long)hora * 10000 +
           (long long)min * 100 +
           (long long)seg;
}

void buscar_intervalo_trie_rec(TrieNode *no, char *prefixo, int nivel, long long inicio, long long fim, char **json, size_t *usado, size_t *capacidade, int *primeiro, int *contagem) {
    if (!no) return;

    if (no->eh_folha && no->dado) {
        long long data_valor = datetime_para_inteiro_TRIE(no->dado->data);
        if (data_valor >= inicio && data_valor <= fim) {
            (*contagem)++;
            char item[1024];
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
                *primeiro ? "" : ",\n",
                no->dado->data,
                no->dado->demanda_residual,
                no->dado->demanda_contratada,
                no->dado->geracao_despachavel,
                no->dado->geracao_termica,
                no->dado->importacoes,
                no->dado->geracao_renovavel_total,
                no->dado->carga_reduzida_manual,
                no->dado->capacidade_instalada,
                no->dado->perdas_geracao_total
            );

            while (*usado + n + 1 >= *capacidade) {
                *capacidade *= 2;
                char *temp = realloc(*json, *capacidade);
                if (!temp) {
                    free(*json);
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
                *json = temp;
            }

            memcpy(*json + *usado, item, n);
            *usado += n;
            *primeiro = 0;
        }
    }

    for (int i = 0; i < MAX_CHAVES; i++) {
        if (no->filhos[i]) {
            prefixo[nivel] = (char)i;
            prefixo[nivel + 1] = '\0';
            buscar_intervalo_trie_rec(no->filhos[i], prefixo, nivel + 1, inicio, fim, json, usado, capacidade, primeiro, contagem);
        }
    }
}

void buscar_intervalo_trie(Trie *trie, const char *inicio_str, const char *fim_str, char **saida) {
    long long inicio = datetime_para_inteiro_TRIE(inicio_str);
    long long fim = datetime_para_inteiro_TRIE(fim_str);

    //printf("Intervalo: %lld - %lld\n", inicio, fim);

    size_t capacidade = 8192;
    size_t usado = 0;
    int primeiro = 1;
    int contagem = 0;

    char *json = malloc(capacidade);
    if (!json) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    usado += snprintf(json + usado, capacidade - usado, "[");

    char prefixo[256];
    buscar_intervalo_trie_rec(trie->raiz, prefixo, 0, inicio, fim, &json, &usado, &capacidade, &primeiro, &contagem);

    usado += snprintf(json + usado, capacidade - usado, "\n]");
    json[usado] = '\0';

    //printf("Itens encontrados: %d\n", contagem);
    *saida = json;
}