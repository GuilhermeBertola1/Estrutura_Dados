#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
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

//Vetor que armaena os dados brutos
void vetor_inicializar(VetorRegistros *v, size_t capacidade_inicial) {
    v->dados = malloc(sizeof(Registro) * capacidade_inicial);
    if (!v->dados) {
        perror("malloc vetor");
        exit(1);
    }
    v->tamanho = 0;
    v->capacidade = capacidade_inicial;
}

void vetor_adicionar(VetorRegistros *v, Registro reg) {
    if (v->tamanho == v->capacidade) {
        v->capacidade *= 2;
        v->dados = realloc(v->dados, sizeof(Registro) * v->capacidade);
        if (!v->dados) {
            perror("realloc vetor");
            exit(1);
        }
    }
    v->dados[v->tamanho++] = reg;
}

void vetor_liberar(VetorRegistros *v) {
    free(v->dados);
    v->dados = NULL;
    v->tamanho = 0;
    v->capacidade = 0;
}

// Função recursiva para coletar registros entre data_inicio e data_fim, montando JSON no buffer 
static void buscar_intervalo_rec(Node *no, long long inicio, long long fim,
                                char **resultado, size_t *tamanho, size_t *usado, int *primeiro,
                                VetorRegistros *vetor) {
    if (!no) return;

    long long data_no = datetime_para_inteiro(no->data);

    if (data_no > inicio)
        buscar_intervalo_rec(no->esq, inicio, fim, resultado, tamanho, usado, primeiro, vetor);

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
                    perror("realloc resultado");
                    exit(1);
                }
            }

            memcpy(*resultado + *usado, item, n);
            *usado += n;
            (*resultado)[*usado] = '\0';
            *primeiro = 0;

            vetor_adicionar(vetor, lr->info);

            lr = lr->prox;
        }
    }

    if (data_no < fim)
        buscar_intervalo_rec(no->dir, inicio, fim, resultado, tamanho, usado, primeiro, vetor);
}


void buscar_intervalo(Node *raiz, const char *data_inicio, const char *data_fim,
                                      char **saida, VetorRegistros *vetor) {
    if (!raiz) {
        *saida = strdup("[]");
        vetor_inicializar(vetor, 1);
        return;
    }

    long long inicio = datetime_para_inteiro(data_inicio);
    long long fim = datetime_para_inteiro(data_fim);
    int primeiro = 1;

    size_t capacidade = 8192;
    size_t usado = 0;
    char *tmp_buffer = malloc(capacidade);
    if (!tmp_buffer) {
        perror("malloc resultado");
        exit(1);
    }

    usado = snprintf(tmp_buffer, capacidade, "[");
    vetor_inicializar(vetor, 128);

    buscar_intervalo_rec(raiz, inicio, fim, &tmp_buffer, &capacidade, &usado, &primeiro, vetor);

    if (usado + 2 >= capacidade) {
        capacidade += 2;
        tmp_buffer = realloc(tmp_buffer, capacidade);
        if (!tmp_buffer) {
            perror("realloc resultado final");
            exit(1);
        }
    }

    tmp_buffer[usado++] = ']';
    tmp_buffer[usado] = '\0';

    *saida = tmp_buffer;
}

void liberar_avl(Node *raiz) {
    if (raiz == NULL) return;
    liberar_avl(raiz->esq);
    liberar_avl(raiz->dir);
    free(raiz);
}

//---------------------- Funções estatísticas ----------------------

EstatisticasCampos calcular_estatisticas(VetorRegistros *v) {
    EstatisticasCampos stats = {0};
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
        Registro *r = &v->dados[i];

        sum_demanda_residual += r->demanda_residual;
        sumsq_demanda_residual += r->demanda_residual * r->demanda_residual;

        sum_demanda_contratada += r->demanda_contratada;
        sumsq_demanda_contratada += r->demanda_contratada * r->demanda_contratada;

        sum_geracao_despachavel += r->geracao_despachavel;
        sumsq_geracao_despachavel += r->geracao_despachavel * r->geracao_despachavel;

        sum_geracao_renovavel_total += r->geracao_renovavel_total;
        sumsq_geracao_renovavel_total += r->geracao_renovavel_total * r->geracao_renovavel_total;

        sum_carga_reduzida_manual += r->carga_reduzida_manual;
        sumsq_carga_reduzida_manual += r->carga_reduzida_manual * r->carga_reduzida_manual;

        sum_capacidade_instalada += r->capacidade_instalada;
        sumsq_capacidade_instalada += r->capacidade_instalada * r->capacidade_instalada;

        sum_perdas_geracao_total += r->perdas_geracao_total;
        sumsq_perdas_geracao_total += r->perdas_geracao_total * r->perdas_geracao_total;

        sum_geracao_termica += r->geracao_termica;
        sumsq_geracao_termica += r->geracao_termica * r->geracao_termica;

        sum_importacoes += r->importacoes;
        sumsq_importacoes += r->importacoes * r->importacoes;
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

int comparador_double(const void *a, const void *b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

// Função para pegar mediana de array ordenado
static double pegar_mediana(double *arr, size_t n) {
    if (n == 0) return 0.0;
    if (n % 2 == 1)
        return arr[n / 2];
    else
        return (arr[(n/2) - 1] + arr[n/2]) / 2.0;
}

// Comparação "aproximada" de doubles
static int doubles_iguais(double a, double b) {
    return fabs(a - b) < EPSILON;
}

// Função para pegar moda de array ordenado
static double pegar_moda(double *arr, size_t n) {
    if (n == 0) return 0.0;

    double moda = arr[0];
    size_t maior_contagem = 1;
    size_t contagem_atual = 1;

    for (size_t i = 1; i < n; i++) {
        if (doubles_iguais(arr[i], arr[i-1])) {
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

Medianas calcular_mediana(VetorRegistros *v) {
    Medianas medianas = {0};
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
        Registro *r = &v->dados[i];
        arr_demanda_residual[i] = r->demanda_residual;
        arr_demanda_contratada[i] = r->demanda_contratada;
        arr_geracao_despachavel[i] = r->geracao_despachavel;
        arr_geracao_renovavel_total[i] = r->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = r->carga_reduzida_manual;
        arr_capacidade_instalada[i] = r->capacidade_instalada;
        arr_perdas_geracao_total[i] = r->perdas_geracao_total;
        arr_geracao_termica[i] = r->geracao_termica;
        arr_importacoes[i] = r->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double);
    qsort(arr_importacoes, n, sizeof(double), comparador_double);

    medianas.demanda_residual = pegar_mediana(arr_demanda_residual, n);
    medianas.demanda_contratada = pegar_mediana(arr_demanda_contratada, n);
    medianas.geracao_despachavel = pegar_mediana(arr_geracao_despachavel, n);
    medianas.geracao_renovavel_total = pegar_mediana(arr_geracao_renovavel_total, n);
    medianas.carga_reduzida_manual = pegar_mediana(arr_carga_reduzida_manual, n);
    medianas.capacidade_instalada = pegar_mediana(arr_capacidade_instalada, n);
    medianas.perdas_geracao_total = pegar_mediana(arr_perdas_geracao_total, n);
    medianas.geracao_termica = pegar_mediana(arr_geracao_termica, n);
    medianas.importacoes = pegar_mediana(arr_importacoes, n);

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

Modas calcular_moda(VetorRegistros *v) {
    Modas modas = {0};
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
        Registro *r = &v->dados[i];
        arr_demanda_residual[i] = r->demanda_residual;
        arr_demanda_contratada[i] = r->demanda_contratada;
        arr_geracao_despachavel[i] = r->geracao_despachavel;
        arr_geracao_renovavel_total[i] = r->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = r->carga_reduzida_manual;
        arr_capacidade_instalada[i] = r->capacidade_instalada;
        arr_perdas_geracao_total[i] = r->perdas_geracao_total;
        arr_geracao_termica[i] = r->geracao_termica;
        arr_importacoes[i] = r->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double);
    qsort(arr_importacoes, n, sizeof(double), comparador_double);

    modas.demanda_residual = pegar_moda(arr_demanda_residual, n);
    modas.demanda_contratada = pegar_moda(arr_demanda_contratada, n);
    modas.geracao_despachavel = pegar_moda(arr_geracao_despachavel, n);
    modas.geracao_renovavel_total = pegar_moda(arr_geracao_renovavel_total, n);
    modas.carga_reduzida_manual = pegar_moda(arr_carga_reduzida_manual, n);
    modas.capacidade_instalada = pegar_moda(arr_capacidade_instalada, n);
    modas.perdas_geracao_total = pegar_moda(arr_perdas_geracao_total, n);
    modas.geracao_termica = pegar_moda(arr_geracao_termica, n);
    modas.importacoes = pegar_moda(arr_importacoes, n);

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

char* estatisticas_para_json_conteudo(EstatisticasCampos est, Medianas med, Modas moda) {
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
