#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "List.h"

NodeList *lista_ligada = NULL;

//inserção de dados como numa fial para não quebrar a estrutura temporal
int in(NodeList** head, EletricDates *dado) {
    NodeList* novo = malloc(sizeof(NodeList));
    if (!novo) {
        perror("malloc");
        return 0;
    }
    novo->dado = *dado;
    novo->next = NULL;

    //printf("Inserindo data: %s\n", novo->dado.data); // debug

    if (*head == NULL) {
        *head = novo;
        //printf("Inserido como primeiro nó.\n");
    } else {
        NodeList* atual = *head;
        while (atual->next != NULL) {
            atual = atual->next;
        }
        atual->next = novo;
        //printf("Inserido no final da lista.\n");
    }
    return 1;
}


long long datetime_para_inteiro_list(const char *datetime) {
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

void vetor_inicializarList (VetorEletricDates *v, size_t capacidade_inicial){
     v->dados = malloc(sizeof(EletricDates) * capacidade_inicial);
    if (!v->dados) {
        perror("malloc vetor");
        exit(1);
    }
    v->qtd = 0;
    v->capacidade = capacidade_inicial;
}

void vetor_adicionarList (VetorEletricDates *v, EletricDates reg){
     if (v->qtd == v->capacidade) {
        v->capacidade *= 2;
        v->dados = realloc(v->dados, sizeof(EletricDates) * v->capacidade);
        if (!v->dados) {
            perror("realloc vetor");
            exit(1);
        }
    }
    v->dados[v->qtd++] = reg;
}

void vetor_liberarList (VetorEletricDates *v) {
    free(v->dados);
    v->dados = NULL;
    v->qtd = 0;
    v->capacidade = 0;
}

//função de busca de dados por intervalo de tempo e sai para json (comunicação com zeromq)
void buscar_intervalo_list(const char *inicio_str, const char *fim_str, char **saida, VetorEletricDates *vetor) {
    long long inicio = datetime_para_inteiro_list(inicio_str);
    long long fim = datetime_para_inteiro_list(fim_str);

    size_t capacidade = 8192;
    size_t usado = 0;
    int primeiro = 1;

    char *json = malloc(capacidade);
    if (!json) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    usado += snprintf(json + usado, capacidade - usado, "[");

    NodeList *atual = lista_ligada;
    int contagem = 0;
    while (atual != NULL) {
        long long data_valor = datetime_para_inteiro_list(atual->dado.data);

        if (data_valor >= inicio && data_valor <= fim) {
            contagem++;

            // Adiciona ao vetor os dados brutos
            if (vetor != NULL) {
                vetor_adicionarList(vetor, atual->dado);
            }

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
                atual->dado.data,
                atual->dado.demanda_residual,
                atual->dado.demanda_contratada,
                atual->dado.geracao_despachavel,
                atual->dado.geracao_termica,
                atual->dado.importacoes,
                atual->dado.geracao_renovavel_total,
                atual->dado.carga_reduzida_manual,
                atual->dado.capacidade_instalada,
                atual->dado.perdas_geracao_total
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
        atual = atual->next;
    }

    usado += snprintf(json + usado, capacidade - usado, "\n]");
    json[usado] = '\0';

    *saida = json;
}

void imprimir_lista(NodeList* no) {
    if (no == NULL) return;

    printf("Data: %s\n", no->dado.data);
    printf("  Demanda Residual: %.2f\n", no->dado.demanda_residual);
    printf("  Demanda Contratada: %.2f\n", no->dado.demanda_contratada);
    printf("  Geração Despachável: %.2f\n", no->dado.geracao_despachavel);
    printf("  Geração Térmica: %.2f\n", no->dado.geracao_termica);
    printf("  Importações: %.2f\n", no->dado.importacoes);
    printf("  Geração Renovável Total: %.2f\n", no->dado.geracao_renovavel_total);
    printf("  Carga Reduzida Manual: %.2f\n", no->dado.carga_reduzida_manual);
    printf("  Capacidade Instalada: %.2f\n", no->dado.capacidade_instalada);
    printf("  Perdas Geração Total: %.2f\n", no->dado.perdas_geracao_total);
    printf("---------------------------\n");

    imprimir_lista(no->next);
}

void liberar_lista(NodeList** head) {
    NodeList* atual = *head;
    while (atual != NULL) {
        NodeList* proximo = atual->next;
        free(atual);
        atual = proximo;
    }
    *head = NULL;
}


// --------------- Funções estatísticas ------------

EstatisticasCamposList calcular_estatisticasList (VetorEletricDates *v) {
    EstatisticasCamposList stats = {0};
    size_t n = v->qtd;
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
        EletricDates *list = &v->dados[i];

        sum_demanda_residual += list->demanda_residual;
        sumsq_demanda_residual += list->demanda_residual * list->demanda_residual;

        sum_demanda_contratada += list->demanda_contratada;
        sumsq_demanda_contratada += list->demanda_contratada * list->demanda_contratada;

        sum_geracao_despachavel += list->geracao_despachavel;
        sumsq_geracao_despachavel += list->geracao_despachavel * list->geracao_despachavel;

        sum_geracao_renovavel_total += list->geracao_renovavel_total;
        sumsq_geracao_renovavel_total += list->geracao_renovavel_total * list->geracao_renovavel_total;

        sum_carga_reduzida_manual += list->carga_reduzida_manual;
        sumsq_carga_reduzida_manual += list->carga_reduzida_manual * list->carga_reduzida_manual;

        sum_capacidade_instalada += list->capacidade_instalada;
        sumsq_capacidade_instalada += list->capacidade_instalada * list->capacidade_instalada;

        sum_perdas_geracao_total += list->perdas_geracao_total;
        sumsq_perdas_geracao_total += list->perdas_geracao_total * list->perdas_geracao_total;

        sum_geracao_termica += list->geracao_termica;
        sumsq_geracao_termica += list->geracao_termica * list->geracao_termica;

        sum_importacoes += list->importacoes;
        sumsq_importacoes += list->importacoes * list->importacoes;
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

int comparador_double_list(const void *a, const void *b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

// Função para pegar mediana de array ordenado
static double pegar_mediana_list(double *arr, size_t n) {
    if (n == 0) return 0.0;
    if (n % 2 == 1)
        return arr[n / 2];
    else
        return (arr[(n/2) - 1] + arr[n/2]) / 2.0;
}

// Comparação "aproximada" de doubles
static int doubles_iguais_list(double a, double b) {
    return fabs(a - b) < EPSILON_list;
}

// Função para pegar moda de array ordenado
static double pegar_moda_list(double *arr, size_t n) {
    if (n == 0) return 0.0;

    double moda = arr[0];
    size_t maior_contagem = 1;
    size_t contagem_atual = 1;

    for (size_t i = 1; i < n; i++) {
        if (doubles_iguais_list(arr[i], arr[i-1])) {
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

MedianasList calcular_mediana_list(VetorEletricDates *v) {
    MedianasList medianas = {0};
    size_t n = v->qtd;
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
        EletricDates *list = &v->dados[i];
        arr_demanda_residual[i] = list->demanda_residual;
        arr_demanda_contratada[i] = list->demanda_contratada;
        arr_geracao_despachavel[i] = list->geracao_despachavel;
        arr_geracao_renovavel_total[i] = list->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = list->carga_reduzida_manual;
        arr_capacidade_instalada[i] = list->capacidade_instalada;
        arr_perdas_geracao_total[i] = list->perdas_geracao_total;
        arr_geracao_termica[i] = list->geracao_termica;
        arr_importacoes[i] = list->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double_list);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double_list);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double_list);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double_list);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double_list);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double_list);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double_list);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double_list);
    qsort(arr_importacoes, n, sizeof(double), comparador_double_list);

    medianas.demanda_residual = pegar_mediana_list(arr_demanda_residual, n);
    medianas.demanda_contratada = pegar_mediana_list(arr_demanda_contratada, n);
    medianas.geracao_despachavel = pegar_mediana_list(arr_geracao_despachavel, n);
    medianas.geracao_renovavel_total = pegar_mediana_list(arr_geracao_renovavel_total, n);
    medianas.carga_reduzida_manual = pegar_mediana_list(arr_carga_reduzida_manual, n);
    medianas.capacidade_instalada = pegar_mediana_list(arr_capacidade_instalada, n);
    medianas.perdas_geracao_total = pegar_mediana_list(arr_perdas_geracao_total, n);
    medianas.geracao_termica = pegar_mediana_list(arr_geracao_termica, n);
    medianas.importacoes = pegar_mediana_list(arr_importacoes, n);

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

ModasList calcular_moda_list(VetorEletricDates *v) {
    ModasList modas = {0};
    size_t n = v->qtd;
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
        EletricDates *list = &v->dados[i];
        arr_demanda_residual[i] = list->demanda_residual;
        arr_demanda_contratada[i] = list->demanda_contratada;
        arr_geracao_despachavel[i] = list->geracao_despachavel;
        arr_geracao_renovavel_total[i] = list->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = list->carga_reduzida_manual;
        arr_capacidade_instalada[i] = list->capacidade_instalada;
        arr_perdas_geracao_total[i] = list->perdas_geracao_total;
        arr_geracao_termica[i] = list->geracao_termica;
        arr_importacoes[i] = list->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double_list);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double_list);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double_list);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double_list);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double_list);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double_list);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double_list);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double_list);
    qsort(arr_importacoes, n, sizeof(double), comparador_double_list);

    modas.demanda_residual = pegar_moda_list(arr_demanda_residual, n);
    modas.demanda_contratada = pegar_moda_list(arr_demanda_contratada, n);
    modas.geracao_despachavel = pegar_moda_list(arr_geracao_despachavel, n);
    modas.geracao_renovavel_total = pegar_moda_list(arr_geracao_renovavel_total, n);
    modas.carga_reduzida_manual = pegar_moda_list(arr_carga_reduzida_manual, n);
    modas.capacidade_instalada = pegar_moda_list(arr_capacidade_instalada, n);
    modas.perdas_geracao_total = pegar_moda_list(arr_perdas_geracao_total, n);
    modas.geracao_termica = pegar_moda_list(arr_geracao_termica, n);
    modas.importacoes = pegar_moda_list(arr_importacoes, n);

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

char* estatisticas_para_json_conteudo_list(EstatisticasCamposList est, MedianasList med, ModasList moda) {
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