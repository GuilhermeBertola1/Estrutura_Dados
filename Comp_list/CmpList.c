#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CmpList.h"

Bloco* criar_bloco() {
    Bloco* novo = malloc(sizeof(Bloco));
    if (!novo) {
        fprintf(stderr, "Erro ao alocar bloco\n");
        exit(1);
    }
    novo->qtd = 0;
    novo->prox = NULL;
    return novo;
}

void inserir_CMP(Bloco** lista, CompList dado) {
    if (*lista == NULL) {
        *lista = criar_bloco();
    }

    Bloco* atual = *lista;
    while (atual->qtd == BLOCO_TAM) {
        if (atual->prox == NULL) {
            atual->prox = criar_bloco();
        }
        atual = atual->prox;
    }

    atual->registros[atual->qtd++] = dado;
}

void imprimir_lista_CMP(Bloco* lista) {
    Bloco* atual = lista;
    while (atual != NULL) {
        for (int i = 0; i < atual->qtd; i++) {
            CompList *a = &atual->registros[i];
            printf(
                "Data: %s\n"
                "  Demanda Residual:       %.2f\n"
                "  Demanda Contratada:     %.2f\n"
                "  Geracao Despachavel:    %.2f\n"
                "  Geracao Termica:        %.2f\n"
                "  Importacoes:            %.2f\n"
                "  Geracao Renovavel:      %.2f\n"
                "  Carga Reduzida Manual:  %.2f\n"
                "  Capacidade Instalada:   %.2f\n"
                "  Perdas Geracao Total:   %.2f\n",
                a->data,
                a->demanda_residual,
                a->demanda_contratada,
                a->geracao_despachavel,
                a->geracao_termica,
                a->importacoes,
                a->geracao_renovavel_total,
                a->carga_reduzida_manual,
                a->capacidade_instalada,
                a->perdas_geracao_total
            );
            printf("-------------------------------------\n");
        }
        atual = atual->prox;
    }
}

void liberar_lista(Bloco* lista) {
    Bloco* atual = lista;
    while (atual != NULL) {
        Bloco* temp = atual;
        atual = atual->prox;
        free(temp);
    }
}

long long datetime_para_inteiro_CMP(const char *datetime) {
    int ano, mes, dia, hora, min, seg;
    sscanf(datetime, "%d-%d-%d %d:%d:%d", &ano, &mes, &dia, &hora, &min, &seg);
    return (long long)ano * 10000000000LL +
           (long long)mes * 100000000 +
           (long long)dia * 1000000 +
           (long long)hora * 10000 +
           (long long)min * 100 +
           (long long)seg;
}

void buscar_intervalo_lista_CMP(Bloco *lista, const char *inicio_str, const char *fim_str, char **saida){
    long long inicio = datetime_para_inteiro_CMP(inicio_str);
    long long fim = datetime_para_inteiro_CMP(fim_str);

    size_t capacidade = 8192;
    size_t usado = 0;
    int primeiro = 1;

    char *json = malloc(capacidade);
    if (!json) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    usado += snprintf(json + usado, capacidade - usado, "[");

    Bloco *atual = lista;
    while (atual != NULL) {
        for (int i = 0; i < atual->qtd; i++) {
            CompList *d = &atual->registros[i];
            long long data_valor = datetime_para_inteiro_CMP(d->data);

            if (data_valor >= inicio && data_valor <= fim) {
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
                    primeiro ? "" : ",\n",
                    d->data,
                    d->demanda_residual,
                    d->demanda_contratada,
                    d->geracao_despachavel,
                    d->geracao_termica,
                    d->importacoes,
                    d->geracao_renovavel_total,
                    d->carga_reduzida_manual,
                    d->capacidade_instalada,
                    d->perdas_geracao_total
                );

                while (usado + n + 1 >= capacidade) {
                    capacidade *= 2;
                    char *temp = realloc(json, capacidade);
                    if (!temp) {
                        free(json);
                        perror("realloc");
                        exit(EXIT_FAILURE);
                    }
                    json = temp;
                }

                memcpy(json + usado, item, n);
                usado += n;
                primeiro = 0;
            }
        }
        atual = atual->prox;
    }

    usado += snprintf(json + usado, capacidade - usado, "\n]");
    json[usado] = '\0';

    *saida = json;
}