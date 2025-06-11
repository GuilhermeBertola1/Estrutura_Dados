#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "Bm.h"

// --- Funções para lista dinâmica ---

void lista_inicializar(ListaDados *lista) {
    lista->itens = NULL;
    lista->tamanho = 0;
    lista->capacidade = 0;
}

void lista_adicionar(ListaDados *lista, Bdados valor) {
    if (lista->tamanho == lista->capacidade) {
        int nova_capacidade = (lista->capacidade == 0) ? 4 : lista->capacidade * 2;
        Bdados *temp = realloc(lista->itens, nova_capacidade * sizeof(Bdados));
        if (!temp) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        lista->itens = temp;
        lista->capacidade = nova_capacidade;
    }
    lista->itens[lista->tamanho++] = valor;
}

void lista_liberar(ListaDados *lista) {
    free(lista->itens);
    lista->itens = NULL;
    lista->tamanho = 0;
    lista->capacidade = 0;
}

// --- B+ Tree ---

BPlusNode *criar_no(bool folha) {
    BPlusNode *no = malloc(sizeof(BPlusNode));
    if (!no) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    no->folha = folha;
    no->n = 0;
    if (folha) {
        for (int i = 0; i < M; i++) {
            lista_inicializar(&no->dados[i]);
        }
        no->prox = NULL;
    } else {
        for (int i = 0; i <= M; i++)
            no->filhos[i] = NULL;
    }
    return no;
}

// Função para comparar chaves, wrapper para strcmp
int comparar_chaves(const char *a, const char *b) {
    return strcmp(a, b);
}

// Insere valor em nó folha, se chave existe adiciona à lista, senão insere nova chave
void inserir_em_no(BPlusNode *no, const char *chave, Bdados valor) {
    int i = 0;
    // Procura se chave já existe
    while (i < no->n && comparar_chaves(no->chaves[i], chave) < 0)
        i++;

    if (i < no->n && comparar_chaves(no->chaves[i], chave) == 0) {
        // chave já existe, adiciona à lista
        lista_adicionar(&no->dados[i], valor);
    } else {
        // nova chave, desloca para inserir
        for (int j = no->n; j > i; j--) {
            strcpy(no->chaves[j], no->chaves[j - 1]);
            // move listas de dados
            no->dados[j] = no->dados[j - 1];
        }
        strcpy(no->chaves[i], chave);
        lista_inicializar(&no->dados[i]);
        lista_adicionar(&no->dados[i], valor);
        no->n++;
    }
}

void dividir_no(BPlusNode *pai, int i, BPlusNode *cheio) {
    BPlusNode *novo = criar_no(cheio->folha);

    int meio = M / 2;

    novo->n = cheio->n - meio - 1;

    if (cheio->folha) {
        for (int j = 0; j < novo->n; j++) {
            strcpy(novo->chaves[j], cheio->chaves[meio + 1 + j]);
            // mover listas
            novo->dados[j] = cheio->dados[meio + 1 + j];
        }
        novo->prox = cheio->prox;
        cheio->prox = novo;

        cheio->n = meio + 1;

        // mover filhos do pai
        for (int j = pai->n; j >= i + 1; j--)
            pai->filhos[j + 1] = pai->filhos[j];
        pai->filhos[i + 1] = novo;

        // mover chaves do pai
        for (int j = pai->n - 1; j >= i; j--)
            strcpy(pai->chaves[j + 1], pai->chaves[j]);

        strcpy(pai->chaves[i], cheio->chaves[meio + 1]);
        pai->n++;
    } else {
        for (int j = 0; j < novo->n; j++) {
            strcpy(novo->chaves[j], cheio->chaves[meio + 1 + j]);
        }

        for (int j = 0; j <= novo->n; j++) {
            novo->filhos[j] = cheio->filhos[meio + 1 + j];
        }

        cheio->n = meio;

        for (int j = pai->n; j >= i + 1; j--)
            pai->filhos[j + 1] = pai->filhos[j];
        pai->filhos[i + 1] = novo;

        for (int j = pai->n - 1; j >= i; j--)
            strcpy(pai->chaves[j + 1], pai->chaves[j]);

        strcpy(pai->chaves[i], cheio->chaves[meio]);
        pai->n++;
    }
}

void inserir_bplus(BPlusNode **raiz, Bdados valor) {
    const char *chave = valor.data;

    if (*raiz == NULL) {
        *raiz = criar_no(true);
    }

    BPlusNode *r = *raiz;

    if (r->n == M) {
        BPlusNode *s = criar_no(false);
        s->filhos[0] = r;
        dividir_no(s, 0, r);
        *raiz = s;
    }

    BPlusNode *atual = *raiz;

    while (!atual->folha) {
        int i = 0;
        while (i < atual->n && strcmp(chave, atual->chaves[i]) > 0)
            i++;

        if (atual->filhos[i] == NULL) {
            fprintf(stderr, "Erro: filho NULL durante descida na árvore\n");
            return;
        }

        BPlusNode *filho = atual->filhos[i];

        if (filho->n == M) {
            dividir_no(atual, i, filho);
            if (strcmp(chave, atual->chaves[i]) > 0)
                i++;

            filho = atual->filhos[i];
        }

        atual = filho;
    }

    inserir_em_no(atual, chave, valor);
}

void imprimir_bplus(BPlusNode *raiz) {
    if (raiz == NULL) {
        printf("arvore B+ vazia.\n");
        return;
    }

    // Vai até o primeiro nó folha
    while (raiz && !raiz->folha) {
        if (raiz->filhos[0] == NULL) {
            printf("Erro: filho[0] é NULL.\n");
            return;
        }
        raiz = raiz->filhos[0];
    }

    printf("=== Dados na arvore B+ ===\n");
    while (raiz) {
        printf("No folha com %d chaves\n", raiz->n);
        for (int i = 0; i < raiz->n; i++) {
            printf("Chave %s com %d registros:\n", raiz->chaves[i], raiz->dados[i].tamanho);
            for (int j = 0; j < raiz->dados[i].tamanho; j++) {
                Bdados d = raiz->dados[i].itens[j];
                printf("  %.24s: %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n",
                    d.data,
                    d.demanda_residual,
                    d.demanda_contratada,
                    d.geracao_despachavel,
                    d.geracao_termica,
                    d.importacoes,
                    d.geracao_renovavel_total,
                    d.carga_reduzida_manual,
                    d.capacidade_instalada,
                    d.perdas_geracao_total);
            }
        }

        if (raiz->prox == raiz) {
            printf("Erro: raiz->prox aponta para si mesma.\n");
            break;
        }
        raiz = raiz->prox;
    }
}

long long datetime_para_inteiro_BM(const char *datetime) {
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
void vetor_inicializar_BM(VetorBdados *v, size_t capacidade_inicial) {
    v->dados = malloc(sizeof(Bdados) * capacidade_inicial);
    if (!v->dados) {
        perror("malloc vetor");
        exit(1);
    }
    v->tamanho = 0;
    v->capacidade = capacidade_inicial;
}

void vetor_adicionar_BM(VetorBdados *v, Bdados reg) {
    if (v->tamanho == v->capacidade) {
        v->capacidade *= 2;
        v->dados = realloc(v->dados, sizeof(Bdados) * v->capacidade);
        if (!v->dados) {
            perror("realloc vetor");
            exit(1);
        }
    }
    v->dados[v->tamanho++] = reg;
}

void vetor_liberar_BM(VetorBdados *v) {
    free(v->dados);
    v->dados = NULL;
    v->tamanho = 0;
    v->capacidade = 0;
}

void buscar_intervalo_bplus_json(BPlusNode *raiz, const char *inicio_str, const char *fim_str, char **saida, VetorBdados *vetor) {
    if (!raiz) {
        *saida = NULL;
        return;
    }

    long long inicio = datetime_para_inteiro_BM(inicio_str);
    long long fim = datetime_para_inteiro_BM(fim_str);

    size_t capacidade = 8192;
    size_t usado = 0;
    int primeiro = 1;

    char *json = malloc(capacidade);
    if (!json) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    usado += snprintf(json + usado, capacidade - usado, "[");

    // Pilha simples para percorrer árvore (usando recursão manual para evitar função aninhada)
    typedef struct {
        BPlusNode *no;
        int i;
    } StackItem;

    StackItem stack[128];
    int sp = 0;
    stack[sp].no = raiz;
    stack[sp].i = 0;

    while (sp >= 0) {
        BPlusNode *no = stack[sp].no;
        int i = stack[sp].i;

        if (!no->folha) {
            if (i <= no->n) {
                // empilha o filho i
                stack[sp].i++;
                sp++;
                if (sp >= 128) {
                    fprintf(stderr, "Stack overflow\n");
                    free(json);
                    exit(EXIT_FAILURE);
                }
                stack[sp].no = (BPlusNode *)no->filhos[i];
                stack[sp].i = 0;
            } else {
                // acabou filhos desse nó
                sp--;
            }
        } else {
            if (i < no->n) {
                ListaDados *lista = &no->dados[i];
                for (int j = 0; j < lista->tamanho; j++) {
                    Bdados *d = &lista->itens[j];
                    long long data_valor = datetime_para_inteiro_BM(d->data);
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
                        vetor_adicionar_BM(vetor, *d);
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
                stack[sp].i++;
            } else {
                sp--;
            }
        }
    }

    usado += snprintf(json + usado, capacidade - usado, "\n]");
    json[usado] = '\0';

    *saida = json;
}

void liberar_bplus(BPlusNode *raiz) {
    if (!raiz) return;

    if (!raiz->folha) {
        for (int i = 0; i <= raiz->n; i++) {
            liberar_bplus(raiz->filhos[i]);
        }
    } else {
        for (int i = 0; i < raiz->n; i++) {
            lista_liberar(&raiz->dados[i]);
        }
    }
    free(raiz);
}

//---------------------- Funções estatísticas ----------------------

EstatisticasCampos_BM calcular_estatisticas_BM(VetorBdados *v) {
    EstatisticasCampos_BM stats = {0};
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
        Bdados *b1 = &v->dados[i];

        sum_demanda_residual += b1->demanda_residual;
        sumsq_demanda_residual += b1->demanda_residual * b1->demanda_residual;

        sum_demanda_contratada += b1->demanda_contratada;
        sumsq_demanda_contratada += b1->demanda_contratada * b1->demanda_contratada;

        sum_geracao_despachavel += b1->geracao_despachavel;
        sumsq_geracao_despachavel += b1->geracao_despachavel * b1->geracao_despachavel;

        sum_geracao_renovavel_total += b1->geracao_renovavel_total;
        sumsq_geracao_renovavel_total += b1->geracao_renovavel_total * b1->geracao_renovavel_total;

        sum_carga_reduzida_manual += b1->carga_reduzida_manual;
        sumsq_carga_reduzida_manual += b1->carga_reduzida_manual * b1->carga_reduzida_manual;

        sum_capacidade_instalada += b1->capacidade_instalada;
        sumsq_capacidade_instalada += b1->capacidade_instalada * b1->capacidade_instalada;

        sum_perdas_geracao_total += b1->perdas_geracao_total;
        sumsq_perdas_geracao_total += b1->perdas_geracao_total * b1->perdas_geracao_total;

        sum_geracao_termica += b1->geracao_termica;
        sumsq_geracao_termica += b1->geracao_termica * b1->geracao_termica;

        sum_importacoes += b1->importacoes;
        sumsq_importacoes += b1->importacoes * b1->importacoes;
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

int comparador_double_BM(const void *a, const void *b) {
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
    return fabs(a - b) < EPSILON_BM;
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

Medianas_BM calcular_mediana_BM(VetorBdados *v) {
    Medianas_BM medianas = {0};
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
        Bdados *b1 = &v->dados[i];
        arr_demanda_residual[i] = b1->demanda_residual;
        arr_demanda_contratada[i] = b1->demanda_contratada;
        arr_geracao_despachavel[i] = b1->geracao_despachavel;
        arr_geracao_renovavel_total[i] = b1->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = b1->carga_reduzida_manual;
        arr_capacidade_instalada[i] = b1->capacidade_instalada;
        arr_perdas_geracao_total[i] = b1->perdas_geracao_total;
        arr_geracao_termica[i] = b1->geracao_termica;
        arr_importacoes[i] = b1->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double_BM);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double_BM);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double_BM);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double_BM);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double_BM);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double_BM);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double_BM);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double_BM);
    qsort(arr_importacoes, n, sizeof(double), comparador_double_BM);

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

Modas_BM calcular_moda_BM(VetorBdados *v) {
    Modas_BM modas = {0};
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
        Bdados *b1 = &v->dados[i];
        arr_demanda_residual[i] = b1->demanda_residual;
        arr_demanda_contratada[i] = b1->demanda_contratada;
        arr_geracao_despachavel[i] = b1->geracao_despachavel;
        arr_geracao_renovavel_total[i] = b1->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = b1->carga_reduzida_manual;
        arr_capacidade_instalada[i] = b1->capacidade_instalada;
        arr_perdas_geracao_total[i] = b1->perdas_geracao_total;
        arr_geracao_termica[i] = b1->geracao_termica;
        arr_importacoes[i] = b1->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double_BM);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double_BM);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double_BM);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double_BM);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double_BM);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double_BM);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double_BM);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double_BM);
    qsort(arr_importacoes, n, sizeof(double), comparador_double_BM);

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

char* estatisticas_para_json_conteudo_BM(EstatisticasCampos_BM est, Medianas_BM med, Modas_BM moda) {
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
