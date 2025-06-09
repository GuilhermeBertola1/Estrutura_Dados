#include "Hash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define INITIAL_SIZE 1024
#define MAX_LOAD_FACTOR 0.7
int qtd_elementos = 0;

#define LIVRE 0
#define OCUPADO 1
#define REMOVIDO 2

int TABLE_SIZE = INITIAL_SIZE;
Entrada *hashLinear = NULL;

unsigned long hashDJB2(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

int hashSimples(const char *chave, int tamanho) {
    return hashDJB2(chave) % tamanho;
}

void inicializarHash() {
    if (hashLinear) {
        free(hashLinear);
        hashLinear = NULL;
    }
    hashLinear = malloc(TABLE_SIZE * sizeof(Entrada));
    if (!hashLinear) {
        perror("malloc");
        exit(1);
    }
    for (int i = 0; i < TABLE_SIZE; i++) hashLinear[i].ocupado = LIVRE;
    qtd_elementos = 0;
}

int inserirHash_linear_sem_rehash(const Entrada *e) {
    int pos = hashSimples(e->chave, TABLE_SIZE);
    for (int i = 0; i < TABLE_SIZE; i++) {
        int idx = (pos + i) % TABLE_SIZE;
        if (hashLinear[idx].ocupado == LIVRE || hashLinear[idx].ocupado == REMOVIDO) {
            hashLinear[idx] = *e;
            hashLinear[idx].ocupado = OCUPADO;
            qtd_elementos++;
            return 1;
        }
    }
    return 0;
}

void rehash_linear() {
    int old_size = TABLE_SIZE;
    Entrada *old_table = hashLinear;

    TABLE_SIZE *= 2;
    hashLinear = malloc(TABLE_SIZE * sizeof(Entrada));
    for (int i = 0; i < TABLE_SIZE; i++) hashLinear[i].ocupado = LIVRE;

    qtd_elementos = 0; // reset

    for (int i = 0; i < old_size; i++) {
        if (old_table[i].ocupado == OCUPADO) {
            inserirHash_linear_sem_rehash(&old_table[i]);
        }
    }

    free(old_table);
    //printf("Rehash realizado. Novo tamanho: %d\n", TABLE_SIZE);
}


int inserirHash_linear(const Entrada *e) {
    // Se inserir este elemento ultrapassar carga máxima, faz rehash
    if ((double)(qtd_elementos + 1) / TABLE_SIZE > MAX_LOAD_FACTOR) {
        rehash_linear();
    }

    int pos = hashSimples(e->chave, TABLE_SIZE);
    for (int i = 0; i < TABLE_SIZE; i++) {
        int idx = (pos + i) % TABLE_SIZE;
        if (hashLinear[idx].ocupado == LIVRE || hashLinear[idx].ocupado == REMOVIDO) {
            hashLinear[idx] = *e;
            hashLinear[idx].ocupado = OCUPADO;
            qtd_elementos++;
            return 1;
        }
    }

    // Se chegou aqui, tabela cheia (muito improvável depois do rehash)
    return 0;
}

int removerHash_linear(const char *chave) {
    int pos = hashSimples(chave, TABLE_SIZE);
    for (int i = 0; i < TABLE_SIZE; i++) {
        int idx = (pos + i) % TABLE_SIZE;
        if (hashLinear[idx].ocupado == OCUPADO && strcmp(hashLinear[idx].chave, chave) == 0) {
            hashLinear[idx].ocupado = REMOVIDO;
            qtd_elementos--;
            return 1;
        }
    }
    return 0;
}

long long datetime_para_inteiro_linear(const char *datetime) {
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

void buscar_intervalo_linear(const char *inicio_str, const char *fim_str, char **saida) {
    long long inicio = datetime_para_inteiro_linear(inicio_str);
    long long fim = datetime_para_inteiro_linear(fim_str);

    size_t capacidade = 8192;
    size_t usado = 0;
    char *buffer = malloc(capacidade);
    if (!buffer) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    usado = snprintf(buffer, capacidade, "[");
    int primeiro = 1;

    for (int i = 0; i < TABLE_SIZE; i++) {
        if (hashLinear[i].ocupado == OCUPADO) {
            long long dt = datetime_para_inteiro_linear(hashLinear[i].data);
            if (dt >= inicio && dt <= fim) {
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
                    hashLinear[i].data,
                    hashLinear[i].demanda_residual,
                    hashLinear[i].demanda_contratada,
                    hashLinear[i].geracao_despachavel,
                    hashLinear[i].geracao_termica,
                    hashLinear[i].importacoes,
                    hashLinear[i].geracao_renovavel_total,
                    hashLinear[i].carga_reduzida_manual,
                    hashLinear[i].capacidade_instalada,
                    hashLinear[i].perdas_geracao_total
                );

                if (usado + n + 1 >= capacidade) {
                    capacidade *= 2;
                    buffer = realloc(buffer, capacidade);
                    if (!buffer) {
                        perror("realloc");
                        exit(EXIT_FAILURE);
                    }
                }

                memcpy(buffer + usado, item, n);
                usado += n;
                buffer[usado] = '\0';
                primeiro = 0;
            }
        }
    }

    if (usado + 2 >= capacidade) {
        capacidade += 2;
        buffer = realloc(buffer, capacidade);
        if (!buffer) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
    }

    buffer[usado++] = ']';
    buffer[usado] = '\0';

    *saida = buffer;
}

void estatisticasHash() {
    int count = 0;
    double soma = 0, min = 1e9, max = -1e9;
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (hashLinear[i].ocupado == OCUPADO) {
            double v = hashLinear[i].demanda_residual;
            soma += v;
            if (v < min) min = v;
            if (v > max) max = v;
            count++;
        }
    }
    if (count)
        printf("Média: %.2f | Mín: %.2f | Máx: %.2f\n", soma / count, min, max);
    else
        printf("Nenhum dado disponível.\n");
}

void liberarHashLinear() {
    if (hashLinear) {
        free(hashLinear);
        hashLinear = NULL;
        TABLE_SIZE = 0;
        qtd_elementos = 0;
    }
}