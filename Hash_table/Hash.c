#include "Hash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

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

//Vetor que armaena os dados brutos
void vetor_inicializar_HT(VetorEntrada *v, size_t capacidade_inicial) {
    v->dados = malloc(sizeof(Entrada) * capacidade_inicial);
    if (!v->dados) {
        perror("malloc vetor");
        exit(1);
    }
    v->tamanho = 0;
    v->capacidade = capacidade_inicial;
}

void vetor_adicionar_HT(VetorEntrada *v, Entrada reg) {
    if (v->tamanho == v->capacidade) {
        v->capacidade *= 2;
        v->dados = realloc(v->dados, sizeof(Entrada) * v->capacidade);
        if (!v->dados) {
            perror("realloc vetor");
            exit(1);
        }
    }
    v->dados[v->tamanho++] = reg;
}

void vetor_liberar_HT(VetorEntrada *v) {
    free(v->dados);
    v->dados = NULL;
    v->tamanho = 0;
    v->capacidade = 0;
}

void buscar_intervalo_HT(const char *inicio_str, const char *fim_str, char **saida, VetorEntrada *vetor) {
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

                 if (vetor != NULL) {
                    vetor_adicionar_HT(vetor, hashLinear[i]);
                }               

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

//---------------------- Funções estatísticas ----------------------

EstatisticasCamposHT calcular_estatisticas_HT(VetorEntrada *v) {
    EstatisticasCamposHT stats = {0};
    size_t n = v->tamanho;
    if (n == 0) return stats;

    // Para cálculo de variância, acumulamos soma e soma dos quadrados
    double sum_demanda_residual = 0, sumsq_demanda_residual = 0;
    double sum_demanda_contratada = 0, sumsq_demanda_contratada = 0;
    double sum_geracao_despachavel = 0, sumsq_geracao_despachavel = 0;
    double sum_geracao_renovavel_total = 0, sumsq_geracao_renovavel_total = 0;
    double sum_carga_reduzida_manual = 0, sumsq_carga_reduzida_manual = 0;
    double sum_capacidade_instalada = 0, sumsq_capacidade_instalada = 0;
    double sum_perdas_geracao_total = 0, sumsq_perdas_geracao_total = 0;
    double sum_geracao_termica = 0, sumsq_geracao_termica = 0;
    double sum_importacoes = 0, sumsq_importacoes = 0;

    for (size_t i = 0; i < n; i++) {
        Entrada *r2 = &v->dados[i];

        sum_demanda_residual += r2->demanda_residual;
        sumsq_demanda_residual += r2->demanda_residual * r2->demanda_residual;

        sum_demanda_contratada += r2->demanda_contratada;
        sumsq_demanda_contratada += r2->demanda_contratada * r2->demanda_contratada;

        sum_geracao_despachavel += r2->geracao_despachavel;
        sumsq_geracao_despachavel += r2->geracao_despachavel * r2->geracao_despachavel;

        sum_geracao_renovavel_total += r2->geracao_renovavel_total;
        sumsq_geracao_renovavel_total += r2->geracao_renovavel_total * r2->geracao_renovavel_total;

        sum_carga_reduzida_manual += r2->carga_reduzida_manual;
        sumsq_carga_reduzida_manual += r2->carga_reduzida_manual * r2->carga_reduzida_manual;

        sum_capacidade_instalada += r2->capacidade_instalada;
        sumsq_capacidade_instalada += r2->capacidade_instalada * r2->capacidade_instalada;

        sum_perdas_geracao_total += r2->perdas_geracao_total;
        sumsq_perdas_geracao_total += r2->perdas_geracao_total * r2->perdas_geracao_total;

        sum_geracao_termica += r2->geracao_termica;
        sumsq_geracao_termica += r2->geracao_termica * r2->geracao_termica;

        sum_importacoes += r2->importacoes;
        sumsq_importacoes += r2->importacoes * r2->importacoes;
    }

    // Cálculo média
    stats.demanda_residual.media = sum_demanda_residual / n;
    stats.demanda_contratada.media = sum_demanda_contratada / n;
    stats.geracao_despachavel.media = sum_geracao_despachavel / n;
    stats.geracao_renovavel_total.media = sum_geracao_renovavel_total / n;
    stats.carga_reduzida_manual.media = sum_carga_reduzida_manual / n;
    stats.capacidade_instalada.media = sum_capacidade_instalada / n;
    stats.perdas_geracao_total.media = sum_perdas_geracao_total / n;
    stats.geracao_termica.media = sum_geracao_termica / n;
    stats.importacoes.media = sum_importacoes / n;

    // Cálculo variância (amostral: divisão por n-1, evita bias)
    if (n > 1) {
        stats.demanda_residual.variancia = (sumsq_demanda_residual - n * stats.demanda_residual.media * stats.demanda_residual.media) / (n - 1);
        stats.demanda_contratada.variancia = (sumsq_demanda_contratada - n * stats.demanda_contratada.media * stats.demanda_contratada.media) / (n - 1);
        stats.geracao_despachavel.variancia = (sumsq_geracao_despachavel - n * stats.geracao_despachavel.media * stats.geracao_despachavel.media) / (n - 1);
        stats.geracao_renovavel_total.variancia = (sumsq_geracao_renovavel_total - n * stats.geracao_renovavel_total.media * stats.geracao_renovavel_total.media) / (n - 1);
        stats.carga_reduzida_manual.variancia = (sumsq_carga_reduzida_manual - n * stats.carga_reduzida_manual.media * stats.carga_reduzida_manual.media) / (n - 1);
        stats.capacidade_instalada.variancia = (sumsq_capacidade_instalada - n * stats.capacidade_instalada.media * stats.capacidade_instalada.media) / (n - 1);
        stats.perdas_geracao_total.variancia = (sumsq_perdas_geracao_total - n * stats.perdas_geracao_total.media * stats.perdas_geracao_total.media) / (n - 1);
        stats.geracao_termica.variancia = (sumsq_geracao_termica - n * stats.geracao_termica.media * stats.geracao_termica.media) / (n - 1);
        stats.importacoes.variancia = (sumsq_importacoes - n * stats.importacoes.media * stats.importacoes.media) / (n - 1);

        // Desvio padrão é raiz quadrada da variância
        stats.demanda_residual.desvio_padrao = sqrt(stats.demanda_residual.variancia);
        stats.demanda_contratada.desvio_padrao = sqrt(stats.demanda_contratada.variancia);
        stats.geracao_despachavel.desvio_padrao = sqrt(stats.geracao_despachavel.variancia);
        stats.geracao_renovavel_total.desvio_padrao = sqrt(stats.geracao_renovavel_total.variancia);
        stats.carga_reduzida_manual.desvio_padrao = sqrt(stats.carga_reduzida_manual.variancia);
        stats.capacidade_instalada.desvio_padrao = sqrt(stats.capacidade_instalada.variancia);
        stats.perdas_geracao_total.desvio_padrao = sqrt(stats.perdas_geracao_total.variancia);
        stats.geracao_termica.desvio_padrao = sqrt(stats.geracao_termica.variancia);
        stats.importacoes.desvio_padrao = sqrt(stats.importacoes.variancia);
    } else {
        // Para n=1, variância e desvio padrão são zero
        stats.demanda_residual.variancia = 0;
        stats.demanda_contratada.variancia = 0;
        stats.geracao_despachavel.variancia = 0;
        stats.geracao_renovavel_total.variancia = 0;
        stats.carga_reduzida_manual.variancia = 0;
        stats.capacidade_instalada.variancia = 0;
        stats.perdas_geracao_total.variancia = 0;
        stats.geracao_termica.variancia = 0;
        stats.importacoes.variancia = 0;

        stats.demanda_residual.desvio_padrao = 0;
        stats.demanda_contratada.desvio_padrao = 0;
        stats.geracao_despachavel.desvio_padrao = 0;
        stats.geracao_renovavel_total.desvio_padrao = 0;
        stats.carga_reduzida_manual.desvio_padrao = 0;
        stats.capacidade_instalada.desvio_padrao = 0;
        stats.perdas_geracao_total.desvio_padrao = 0;
        stats.geracao_termica.desvio_padrao = 0;
        stats.importacoes.desvio_padrao = 0;
    }

    return stats;
}

int comparador_double_HT(const void *a, const void *b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

// Função para pegar mediana de array ordenado
static double pegar_mediana_HT(double *arr, size_t n) {
    if (n == 0) return 0.0;
    if (n % 2 == 1)
        return arr[n / 2];
    else
        return (arr[(n/2) - 1] + arr[n/2]) / 2.0;
}

// Comparação "aproximada" de doubles
static int doubles_iguais_HT(double a, double b) {
    return fabs(a - b) < EPSILON_HT;
}

// Função para pegar moda de array ordenado
static double pegar_moda_HT(double *arr, size_t n) {
    if (n == 0) return 0.0;

    double moda = arr[0];
    size_t maior_contagem = 1;
    size_t contagem_atual = 1;

    for (size_t i = 1; i < n; i++) {
        if (doubles_iguais_HT(arr[i], arr[i-1])) {
            contagem_atual++;
        } else {
            if (contagem_atual > maior_contagem) {
                maior_contagem = contagem_atual;
                moda = arr[i-1];
            }
            contagem_atual = 1;
        }
    }

    if (contagem_atual > maior_contagem) {
        moda = arr[n-1];
    }

    return moda;
}

MedianasHT calcular_mediana_HT(VetorEntrada *v) {
    MedianasHT medianas = {0};
    size_t n = v->tamanho;
    if (n == 0) return medianas;

    double *arr_demanda_residual = malloc(n * sizeof(double));
    double *arr_demanda_contratada = malloc(n * sizeof(double));
    double *arr_geracao_despachavel = malloc(n * sizeof(double));
    double *arr_geracao_renovavel_total = malloc(n * sizeof(double));
    double *arr_carga_reduzida_manual = malloc(n * sizeof(double));
    double *arr_capacidade_instalada = malloc(n * sizeof(double));
    double *arr_perdas_geracao_total = malloc(n * sizeof(double));
    double *arr_geracao_termica = malloc(n * sizeof(double));
    double *arr_importacoes = malloc(n * sizeof(double));

    for (size_t i = 0; i < n; i++) {
        Entrada *r2 = &v->dados[i];
        arr_demanda_residual[i] = r2->demanda_residual;
        arr_demanda_contratada[i] = r2->demanda_contratada;
        arr_geracao_despachavel[i] = r2->geracao_despachavel;
        arr_geracao_renovavel_total[i] = r2->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = r2->carga_reduzida_manual;
        arr_capacidade_instalada[i] = r2->capacidade_instalada;
        arr_perdas_geracao_total[i] = r2->perdas_geracao_total;
        arr_geracao_termica[i] = r2->geracao_termica;
        arr_importacoes[i] = r2->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double_HT);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double_HT);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double_HT);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double_HT);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double_HT);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double_HT);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double_HT);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double_HT);
    qsort(arr_importacoes, n, sizeof(double), comparador_double_HT);

    medianas.demanda_residual = pegar_mediana_HT(arr_demanda_residual, n);
    medianas.demanda_contratada = pegar_mediana_HT(arr_demanda_contratada, n);
    medianas.geracao_despachavel = pegar_mediana_HT(arr_geracao_despachavel, n);
    medianas.geracao_renovavel_total = pegar_mediana_HT(arr_geracao_renovavel_total, n);
    medianas.carga_reduzida_manual = pegar_mediana_HT(arr_carga_reduzida_manual, n);
    medianas.capacidade_instalada = pegar_mediana_HT(arr_capacidade_instalada, n);
    medianas.perdas_geracao_total = pegar_mediana_HT(arr_perdas_geracao_total, n);
    medianas.geracao_termica = pegar_mediana_HT(arr_geracao_termica, n);
    medianas.importacoes = pegar_mediana_HT(arr_importacoes, n);

    free(arr_demanda_residual);
    free(arr_demanda_contratada);
    free(arr_geracao_despachavel);
    free(arr_geracao_renovavel_total);
    free(arr_carga_reduzida_manual);
    free(arr_capacidade_instalada);
    free(arr_perdas_geracao_total);
    free(arr_geracao_termica);
    free(arr_importacoes);

    return medianas;
}

ModasHT calcular_moda_HT(VetorEntrada *v) {
    ModasHT modas = {0};
    size_t n = v->tamanho;
    if (n == 0) return modas;

    double *arr_demanda_residual = malloc(n * sizeof(double));
    double *arr_demanda_contratada = malloc(n * sizeof(double));
    double *arr_geracao_despachavel = malloc(n * sizeof(double));
    double *arr_geracao_renovavel_total = malloc(n * sizeof(double));
    double *arr_carga_reduzida_manual = malloc(n * sizeof(double));
    double *arr_capacidade_instalada = malloc(n * sizeof(double));
    double *arr_perdas_geracao_total = malloc(n * sizeof(double));
    double *arr_geracao_termica = malloc(n * sizeof(double));
    double *arr_importacoes = malloc(n * sizeof(double));

    for (size_t i = 0; i < n; i++) {
        Entrada *r2 = &v->dados[i];
        arr_demanda_residual[i] = r2->demanda_residual;
        arr_demanda_contratada[i] = r2->demanda_contratada;
        arr_geracao_despachavel[i] = r2->geracao_despachavel;
        arr_geracao_renovavel_total[i] = r2->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = r2->carga_reduzida_manual;
        arr_capacidade_instalada[i] = r2->capacidade_instalada;
        arr_perdas_geracao_total[i] = r2->perdas_geracao_total;
        arr_geracao_termica[i] = r2->geracao_termica;
        arr_importacoes[i] = r2->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double_HT);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double_HT);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double_HT);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double_HT);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double_HT);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double_HT);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double_HT);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double_HT);
    qsort(arr_importacoes, n, sizeof(double), comparador_double_HT);

    modas.demanda_residual = pegar_moda_HT(arr_demanda_residual, n);
    modas.demanda_contratada = pegar_moda_HT(arr_demanda_contratada, n);
    modas.geracao_despachavel = pegar_moda_HT(arr_geracao_despachavel, n);
    modas.geracao_renovavel_total = pegar_moda_HT(arr_geracao_renovavel_total, n);
    modas.carga_reduzida_manual = pegar_moda_HT(arr_carga_reduzida_manual, n);
    modas.capacidade_instalada = pegar_moda_HT(arr_capacidade_instalada, n);
    modas.perdas_geracao_total = pegar_moda_HT(arr_perdas_geracao_total, n);
    modas.geracao_termica = pegar_moda_HT(arr_geracao_termica, n);
    modas.importacoes = pegar_moda_HT(arr_importacoes, n);

    free(arr_demanda_residual);
    free(arr_demanda_contratada);
    free(arr_geracao_despachavel);
    free(arr_geracao_renovavel_total);
    free(arr_carga_reduzida_manual);
    free(arr_capacidade_instalada);
    free(arr_perdas_geracao_total);
    free(arr_geracao_termica);
    free(arr_importacoes);

    return modas;
}

static int print_val_or_null(char *buffer, size_t size, double val) {
    if (isfinite(val)) {
        return snprintf(buffer, size, "%.2f", val);
    } else {
        return snprintf(buffer, size, "null");
    }
}

char* estatisticas_para_json_conteudo_HT(EstatisticasCamposHT est, MedianasHT med, ModasHT moda) {
    char *json = malloc(16384);
    if (!json) return NULL;

    int offset = 0, ret;
    char val_str[16];

    ret = snprintf(json + offset, 16384 - offset, "{");
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    // Media
    ret = snprintf(json + offset, 16384 - offset, "\"Media\":{");
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    #define PRINT_FIELD_MEDIA(field) \
        do { \
            ret = print_val_or_null(val_str, sizeof(val_str), est.field.media); \
            if (ret < 0) { free(json); return NULL; } \
            ret = snprintf(json + offset, 16384 - offset, "\"" #field "\":%s,", val_str); \
            if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; } \
            offset += ret; \
        } while (0)

    PRINT_FIELD_MEDIA(demanda_residual);
    PRINT_FIELD_MEDIA(demanda_contratada);
    PRINT_FIELD_MEDIA(geracao_despachavel);
    PRINT_FIELD_MEDIA(geracao_renovavel_total);
    PRINT_FIELD_MEDIA(carga_reduzida_manual);
    PRINT_FIELD_MEDIA(capacidade_instalada);
    PRINT_FIELD_MEDIA(perdas_geracao_total);
    PRINT_FIELD_MEDIA(geracao_termica);
    // Importações será a última sem vírgula
    ret = print_val_or_null(val_str, sizeof(val_str), est.importacoes.media);
    if (ret < 0) { free(json); return NULL; }
    ret = snprintf(json + offset, 16384 - offset, "\"importacoes\":%s", val_str);
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    ret = snprintf(json + offset, 16384 - offset, "},");
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    // Mediana
    ret = snprintf(json + offset, 16384 - offset, "\"Mediana\":{");
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    #define PRINT_FIELD_MEDIANA(field) \
        do { \
            ret = print_val_or_null(val_str, sizeof(val_str), med.field); \
            if (ret < 0) { free(json); return NULL; } \
            ret = snprintf(json + offset, 16384 - offset, "\"" #field "\":%s,", val_str); \
            if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; } \
            offset += ret; \
        } while (0)

    PRINT_FIELD_MEDIANA(demanda_residual);
    PRINT_FIELD_MEDIANA(demanda_contratada);
    PRINT_FIELD_MEDIANA(geracao_despachavel);
    PRINT_FIELD_MEDIANA(geracao_renovavel_total);
    PRINT_FIELD_MEDIANA(carga_reduzida_manual);
    PRINT_FIELD_MEDIANA(capacidade_instalada);
    PRINT_FIELD_MEDIANA(perdas_geracao_total);
    PRINT_FIELD_MEDIANA(geracao_termica);
    // Último campo sem vírgula
    ret = print_val_or_null(val_str, sizeof(val_str), med.importacoes);
    if (ret < 0) { free(json); return NULL; }
    ret = snprintf(json + offset, 16384 - offset, "\"importacoes\":%s", val_str);
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    ret = snprintf(json + offset, 16384 - offset, "},");
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    // Moda
    ret = snprintf(json + offset, 16384 - offset, "\"Moda\":{");
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    #define PRINT_FIELD_MODA(field) \
        do { \
            ret = print_val_or_null(val_str, sizeof(val_str), moda.field); \
            if (ret < 0) { free(json); return NULL; } \
            ret = snprintf(json + offset, 16384 - offset, "\"" #field "\":%s,", val_str); \
            if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; } \
            offset += ret; \
        } while (0)

    PRINT_FIELD_MODA(demanda_residual);
    PRINT_FIELD_MODA(demanda_contratada);
    PRINT_FIELD_MODA(geracao_despachavel);
    PRINT_FIELD_MODA(geracao_renovavel_total);
    PRINT_FIELD_MODA(carga_reduzida_manual);
    PRINT_FIELD_MODA(capacidade_instalada);
    PRINT_FIELD_MODA(perdas_geracao_total);
    PRINT_FIELD_MODA(geracao_termica);
    // Último campo sem vírgula
    ret = print_val_or_null(val_str, sizeof(val_str), moda.importacoes);
    if (ret < 0) { free(json); return NULL; }
    ret = snprintf(json + offset, 16384 - offset, "\"importacoes\":%s", val_str);
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    ret = snprintf(json + offset, 16384 - offset, "},");
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    // Desvio_Padrao
    ret = snprintf(json + offset, 16384 - offset, "\"Desvio_Padrao\":{");
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    #define PRINT_FIELD_DESVIO(field) \
        do { \
            ret = print_val_or_null(val_str, sizeof(val_str), est.field.desvio_padrao); \
            if (ret < 0) { free(json); return NULL; } \
            ret = snprintf(json + offset, 16384 - offset, "\"" #field "\":%s,", val_str); \
            if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; } \
            offset += ret; \
        } while (0)

    PRINT_FIELD_DESVIO(demanda_residual);
    PRINT_FIELD_DESVIO(demanda_contratada);
    PRINT_FIELD_DESVIO(geracao_despachavel);
    PRINT_FIELD_DESVIO(geracao_renovavel_total);
    PRINT_FIELD_DESVIO(carga_reduzida_manual);
    PRINT_FIELD_DESVIO(capacidade_instalada);
    PRINT_FIELD_DESVIO(perdas_geracao_total);
    PRINT_FIELD_DESVIO(geracao_termica);
    // Último campo sem vírgula
    ret = print_val_or_null(val_str, sizeof(val_str), est.importacoes.desvio_padrao);
    if (ret < 0) { free(json); return NULL; }
    ret = snprintf(json + offset, 16384 - offset, "\"importacoes\":%s", val_str);
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    ret = snprintf(json + offset, 16384 - offset, "},");
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    // Variancia
    ret = snprintf(json + offset, 16384 - offset, "\"Variancia\":{");
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    #define PRINT_FIELD_VARIANCIA(field) \
        do { \
            ret = print_val_or_null(val_str, sizeof(val_str), est.field.variancia); \
            if (ret < 0) { free(json); return NULL; } \
            ret = snprintf(json + offset, 16384 - offset, "\"" #field "\":%s,", val_str); \
            if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; } \
            offset += ret; \
        } while (0)

    PRINT_FIELD_VARIANCIA(demanda_residual);
    PRINT_FIELD_VARIANCIA(demanda_contratada);
    PRINT_FIELD_VARIANCIA(geracao_despachavel);
    PRINT_FIELD_VARIANCIA(geracao_renovavel_total);
    PRINT_FIELD_VARIANCIA(carga_reduzida_manual);
    PRINT_FIELD_VARIANCIA(capacidade_instalada);
    PRINT_FIELD_VARIANCIA(perdas_geracao_total);
    PRINT_FIELD_VARIANCIA(geracao_termica);
    // Último campo sem vírgula
    ret = print_val_or_null(val_str, sizeof(val_str), est.importacoes.variancia);
    if (ret < 0) { free(json); return NULL; }
    ret = snprintf(json + offset, 16384 - offset, "\"importacoes\":%s", val_str);
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    ret = snprintf(json + offset, 16384 - offset, "}}");
    if (ret < 0 || ret >= 16384 - offset) { free(json); return NULL; }
    offset += ret;

    #undef PRINT_FIELD_MEDIA
    #undef PRINT_FIELD_MEDIANA
    #undef PRINT_FIELD_MODA
    #undef PRINT_FIELD_DESVIO
    #undef PRINT_FIELD_VARIANCIA

    return json;
}