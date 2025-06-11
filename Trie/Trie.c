#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
void vetor_inicializar_Trie(VetorEntrada2 *v, size_t capacidade_inicial) {
    v->dados = malloc(sizeof(Entrada2) * capacidade_inicial);
    if (!v->dados) {
        perror("malloc vetor");
        exit(1);
    }
    v->tamanho = 0;
    v->capacidade = capacidade_inicial;
}

void vetor_adicionar_Trie(VetorEntrada2 *v, Entrada2 reg) {
    if (v->tamanho == v->capacidade) {
        v->capacidade *= 2;
        v->dados = realloc(v->dados, sizeof(Entrada2) * v->capacidade);
        if (!v->dados) {
            perror("realloc vetor");
            exit(1);
        }
    }
    v->dados[v->tamanho++] = reg;
}

void vetor_liberar_Trie(VetorEntrada2 *v) {
    free(v->dados);
    v->dados = NULL;
    v->tamanho = 0;
    v->capacidade = 0;
}

void buscar_intervalo_trie_rec(TrieNode *no, char *prefixo, int nivel, long long inicio, long long fim, char **json, size_t *usado, size_t *capacidade, int *primeiro, int *contagem, VetorEntrada2 *vetor) {
    if (!no) return;

    if (no->eh_folha && no->dado) {
        long long data_valor = datetime_para_inteiro_TRIE(no->dado->data);
        if (data_valor >= inicio && data_valor <= fim) {
            (*contagem)++;

            if(vetor != NULL){
                vetor_adicionar_Trie(vetor, *(no->dado));
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
            buscar_intervalo_trie_rec(no->filhos[i], prefixo, nivel + 1, inicio, fim, json, usado, capacidade, primeiro, contagem, vetor);
        }
    }
}

void buscar_intervalo_trie(Trie *trie, const char *inicio_str, const char *fim_str, char **saida, VetorEntrada2 *vetor) {
    long long inicio = datetime_para_inteiro_TRIE(inicio_str);
    long long fim = datetime_para_inteiro_TRIE(fim_str);

   vetor_inicializar_Trie(vetor, 1024);


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
    buscar_intervalo_trie_rec(trie->raiz, prefixo, 0, inicio, fim, &json, &usado, &capacidade, &primeiro, &contagem, vetor);

    usado += snprintf(json + usado, capacidade - usado, "\n]");
    json[usado] = '\0';

    //printf("Itens encontrados: %d\n", contagem);
    *saida = json;
}

//---------------------- Funções estatísticas ----------------------

EstatisticasCamposTrie calcular_estatisticas_Trie(VetorEntrada2 *v) {
    EstatisticasCamposTrie stats = {0};
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
        Entrada2 *t1 = &v->dados[i];

        sum_demanda_residual += t1->demanda_residual;
        sumsq_demanda_residual += t1->demanda_residual * t1->demanda_residual;

        sum_demanda_contratada += t1->demanda_contratada;
        sumsq_demanda_contratada += t1->demanda_contratada * t1->demanda_contratada;

        sum_geracao_despachavel += t1->geracao_despachavel;
        sumsq_geracao_despachavel += t1->geracao_despachavel * t1->geracao_despachavel;

        sum_geracao_renovavel_total += t1->geracao_renovavel_total;
        sumsq_geracao_renovavel_total += t1->geracao_renovavel_total * t1->geracao_renovavel_total;

        sum_carga_reduzida_manual += t1->carga_reduzida_manual;
        sumsq_carga_reduzida_manual += t1->carga_reduzida_manual * t1->carga_reduzida_manual;

        sum_capacidade_instalada += t1->capacidade_instalada;
        sumsq_capacidade_instalada += t1->capacidade_instalada * t1->capacidade_instalada;

        sum_perdas_geracao_total += t1->perdas_geracao_total;
        sumsq_perdas_geracao_total += t1->perdas_geracao_total * t1->perdas_geracao_total;

        sum_geracao_termica += t1->geracao_termica;
        sumsq_geracao_termica += t1->geracao_termica * t1->geracao_termica;

        sum_importacoes += t1->importacoes;
        sumsq_importacoes += t1->importacoes * t1->importacoes;
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

int comparador_double_Trie(const void *a, const void *b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

// Função para pegar mediana de array ordenado
static double pegar_mediana_Trie(double *arr, size_t n) {
    if (n == 0) return 0.0;
    if (n % 2 == 1)
        return arr[n / 2];
    else
        return (arr[(n/2) - 1] + arr[n/2]) / 2.0;
}

// Comparação "aproximada" de doubles
static int doubles_iguais_Trie(double a, double b) {
    return fabs(a - b) < EPSILON_Trie;
}

// Função para pegar moda de array ordenado
static double pegar_moda_Trie(double *arr, size_t n) {
    if (n == 0) return 0.0;

    double moda = arr[0];
    size_t maior_contagem = 1;
    size_t contagem_atual = 1;

    for (size_t i = 1; i < n; i++) {
        if (doubles_iguais_Trie(arr[i], arr[i-1])) {
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

MedianasTrie calcular_mediana_Trie(VetorEntrada2 *v) {
    MedianasTrie medianas = {0};
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
        Entrada2 *t1 = &v->dados[i];
        arr_demanda_residual[i] = t1->demanda_residual;
        arr_demanda_contratada[i] = t1->demanda_contratada;
        arr_geracao_despachavel[i] = t1->geracao_despachavel;
        arr_geracao_renovavel_total[i] = t1->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = t1->carga_reduzida_manual;
        arr_capacidade_instalada[i] = t1->capacidade_instalada;
        arr_perdas_geracao_total[i] = t1->perdas_geracao_total;
        arr_geracao_termica[i] = t1->geracao_termica;
        arr_importacoes[i] = t1->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double_Trie);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double_Trie);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double_Trie);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double_Trie);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double_Trie);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double_Trie);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double_Trie);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double_Trie);
    qsort(arr_importacoes, n, sizeof(double), comparador_double_Trie);

    medianas.demanda_residual = pegar_mediana_Trie(arr_demanda_residual, n);
    medianas.demanda_contratada = pegar_mediana_Trie(arr_demanda_contratada, n);
    medianas.geracao_despachavel = pegar_mediana_Trie(arr_geracao_despachavel, n);
    medianas.geracao_renovavel_total = pegar_mediana_Trie(arr_geracao_renovavel_total, n);
    medianas.carga_reduzida_manual = pegar_mediana_Trie(arr_carga_reduzida_manual, n);
    medianas.capacidade_instalada = pegar_mediana_Trie(arr_capacidade_instalada, n);
    medianas.perdas_geracao_total = pegar_mediana_Trie(arr_perdas_geracao_total, n);
    medianas.geracao_termica = pegar_mediana_Trie(arr_geracao_termica, n);
    medianas.importacoes = pegar_mediana_Trie(arr_importacoes, n);

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

ModasTrie calcular_moda_Trie(VetorEntrada2 *v) {
    ModasTrie modas = {0};
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
        Entrada2 *t1 = &v->dados[i];
        arr_demanda_residual[i] = t1->demanda_residual;
        arr_demanda_contratada[i] = t1->demanda_contratada;
        arr_geracao_despachavel[i] = t1->geracao_despachavel;
        arr_geracao_renovavel_total[i] = t1->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = t1->carga_reduzida_manual;
        arr_capacidade_instalada[i] = t1->capacidade_instalada;
        arr_perdas_geracao_total[i] = t1->perdas_geracao_total;
        arr_geracao_termica[i] = t1->geracao_termica;
        arr_importacoes[i] = t1->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double_Trie);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double_Trie);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double_Trie);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double_Trie);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double_Trie);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double_Trie);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double_Trie);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double_Trie);
    qsort(arr_importacoes, n, sizeof(double), comparador_double_Trie);

    modas.demanda_residual = pegar_moda_Trie(arr_demanda_residual, n);
    modas.demanda_contratada = pegar_moda_Trie(arr_demanda_contratada, n);
    modas.geracao_despachavel = pegar_moda_Trie(arr_geracao_despachavel, n);
    modas.geracao_renovavel_total = pegar_moda_Trie(arr_geracao_renovavel_total, n);
    modas.carga_reduzida_manual = pegar_moda_Trie(arr_carga_reduzida_manual, n);
    modas.capacidade_instalada = pegar_moda_Trie(arr_capacidade_instalada, n);
    modas.perdas_geracao_total = pegar_moda_Trie(arr_perdas_geracao_total, n);
    modas.geracao_termica = pegar_moda_Trie(arr_geracao_termica, n);
    modas.importacoes = pegar_moda_Trie(arr_importacoes, n);

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

char* estatisticas_para_json_conteudo_Trie(EstatisticasCamposTrie est, MedianasTrie med, ModasTrie moda) {
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