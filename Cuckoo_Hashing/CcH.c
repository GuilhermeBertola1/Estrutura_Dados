#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "CcH.h"

CuckooEntry *tabela1 = NULL;
CuckooEntry *tabela2 = NULL;
size_t tamanho_atual = 0;
size_t quantidade_itens = 0;

// Funções de hash (modificadas para usar tamanho_atual)
unsigned int hash1(const char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % tamanho_atual;
}

unsigned int hash2(const char *str) {
    unsigned int hash = 0;
    int c;
    while ((c = *str++))
        hash = hash * 131 + c;
    return hash % tamanho_atual;
}

void inicializarCuckoo(size_t tamanho_inicial) {
    tamanho_atual = tamanho_inicial;
    quantidade_itens = 0;
    tabela1 = calloc(tamanho_atual, sizeof(CuckooEntry));
    tabela2 = calloc(tamanho_atual, sizeof(CuckooEntry));
    if (!tabela1 || !tabela2) {
        perror("calloc");
        exit(1);
    }
}

void liberarCuckoo(void) {
    free(tabela1);
    free(tabela2);
    tabela1 = NULL;
    tabela2 = NULL;
    tamanho_atual = 0;
    quantidade_itens = 0;
}

// Verifica se a chave já existe na tabela para evitar duplicados
int existe_na_tabela(const char *chave) {
    unsigned int pos1 = hash1(chave);
    if (tabela1[pos1].ocupado && strcmp(tabela1[pos1].chave, chave) == 0)
        return 1;
    unsigned int pos2 = hash2(chave);
    if (tabela2[pos2].ocupado && strcmp(tabela2[pos2].chave, chave) == 0)
        return 1;
    return 0;
}

// Função interna para inserir sem chamar rehash (usada no rehash)
static int inserir_sem_rehash(const Registro1 *r) {
    char chave[MAX_STR];
    strncpy(chave, r->data, MAX_STR);
    chave[MAX_STR - 1] = '\0';

    Registro1 temp = *r;
    char tempChave[MAX_STR];
    strncpy(tempChave, chave, MAX_STR);

    unsigned int pos;
    int tabela = 1;
    int max_tentativas = 100;

    for (int i = 0; i < max_tentativas; i++) {
        if (tabela == 1) {
            pos = hash1(tempChave);
            if (!tabela1[pos].ocupado) {
                tabela1[pos].ocupado = 1;
                tabela1[pos].valor = temp;
                strncpy(tabela1[pos].chave, tempChave, MAX_STR);
                return 1;
            }
            Registro1 aux = tabela1[pos].valor;
            char auxChave[MAX_STR];
            strncpy(auxChave, tabela1[pos].chave, MAX_STR);

            tabela1[pos].valor = temp;
            strncpy(tabela1[pos].chave, tempChave, MAX_STR);

            temp = aux;
            strncpy(tempChave, auxChave, MAX_STR);

            tabela = 2;
        } else {
            pos = hash2(tempChave);
            if (!tabela2[pos].ocupado) {
                tabela2[pos].ocupado = 1;
                tabela2[pos].valor = temp;
                strncpy(tabela2[pos].chave, tempChave, MAX_STR);
                return 1;
            }
            Registro1 aux = tabela2[pos].valor;
            char auxChave[MAX_STR];
            strncpy(auxChave, tabela2[pos].chave, MAX_STR);

            tabela2[pos].valor = temp;
            strncpy(tabela2[pos].chave, tempChave, MAX_STR);

            temp = aux;
            strncpy(tempChave, auxChave, MAX_STR);

            tabela = 1;
        }
    }
    fprintf(stderr, "Falha na inserção do registro %s após %d tentativas\n", r->data, max_tentativas);
    return 0; // falhou
}

void rehash(size_t novo_tamanho) {
    CuckooEntry *antiga1 = tabela1;
    CuckooEntry *antiga2 = tabela2;
    size_t antiga_tam = tamanho_atual;

    tabela1 = calloc(novo_tamanho, sizeof(CuckooEntry));
    tabela2 = calloc(novo_tamanho, sizeof(CuckooEntry));
    if (!tabela1 || !tabela2) {
        perror("calloc");
        exit(1);
    }

    tamanho_atual = novo_tamanho;
    quantidade_itens = 0;

    for (size_t i = 0; i < antiga_tam; i++) {
        if (antiga1[i].ocupado) {
            if (!inserir_sem_rehash(&antiga1[i].valor)) {
                fprintf(stderr, "Erro no rehash (tabela1)\n");
                exit(1);
            }
            quantidade_itens++; // atualiza contador ao re-inserir
        }
        if (antiga2[i].ocupado) {
            if (!inserir_sem_rehash(&antiga2[i].valor)) {
                fprintf(stderr, "Erro no rehash (tabela2)\n");
                exit(1);
            }
            quantidade_itens++; // atualiza contador ao re-inserir
        }
    }

    free(antiga1);
    free(antiga2);
}

// Função principal de inserção, que chama rehash se necessário
int inserirCuckoo(const Registro1 *r) {
    //printf("Inserindo: [%s]\n", r->data);
    if (existe_na_tabela(r->data)) {
        return 1;
    }

    if (quantidade_itens >= tamanho_atual * 0.4) {
        rehash(tamanho_atual * 2);
    }

    int sucesso = inserir_sem_rehash(r);
    if (!sucesso) {
        rehash(tamanho_atual * 2);
        sucesso = inserir_sem_rehash(r);
        if (!sucesso) {
            fprintf(stderr, "Falha ao inserir mesmo após rehash\n");
            return 0;
        }
    }
    quantidade_itens++;
    return 1;
}

// As outras funções continuam iguais:

long long datetime_para_inteiro_cuck(const char *datetime) {
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
void vetor_inicializar_cc(VetorRegistro1 *v, size_t capacidade_inicial) {
    v->dados = malloc(sizeof(Registro1) * capacidade_inicial);
    if (!v->dados) {
        perror("malloc vetor");
        exit(1);
    }
    v->tamanho = 0;
    v->capacidade = capacidade_inicial;
}

void vetor_adicionar_cc(VetorRegistro1 *v, Registro1 reg) {
    if (v->tamanho == v->capacidade) {
        v->capacidade *= 2;
        v->dados = realloc(v->dados, sizeof(Registro1) * v->capacidade);
        if (!v->dados) {
            perror("realloc vetor");
            exit(1);
        }
    }
    v->dados[v->tamanho++] = reg;
}

void vetor_liberar_cc(VetorRegistro1 *v) {
    free(v->dados);
    v->dados = NULL;
    v->tamanho = 0;
    v->capacidade = 0;
}

void buscar_intervalo_cuckoo(const char *data_inicio, const char *data_fim, char **saida, VetorRegistro1 *vetor) {
    long long inicio = datetime_para_inteiro_cuck(data_inicio);
    long long fim = datetime_para_inteiro_cuck(data_fim);
    int primeiro = 1;

    size_t capacidade = 8192;
    size_t usado = 0;
    char *tmp_buffer = malloc(capacidade);
    if (!tmp_buffer) {
        perror("malloc");
        exit(1);
    }

    usado = snprintf(tmp_buffer, capacidade, "[");

    for (size_t i = 0; i < tamanho_atual; i++) {
        CuckooEntry *registros[2] = { &tabela1[i], &tabela2[i] };
        for (int j = 0; j < 2; j++) {
            if (registros[j]->ocupado && strlen(registros[j]->valor.data) > 0) {
                long long data_reg = datetime_para_inteiro_cuck(registros[j]->valor.data);
                if (data_reg >= inicio && data_reg <= fim) {
                    
                    // Adiciona ao vetor os dados brutos
            if (vetor != NULL) {
                vetor_adicionar_cc(vetor, registros[j]->valor);
            }

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
                        (primeiro ? "" : ",\n"),
                        registros[j]->valor.data,
                        registros[j]->valor.demanda_residual,
                        registros[j]->valor.demanda_contratada,
                        registros[j]->valor.geracao_despachavel,
                        registros[j]->valor.geracao_termica,
                        registros[j]->valor.importacoes,
                        registros[j]->valor.geracao_renovavel_total,
                        registros[j]->valor.carga_reduzida_manual,
                        registros[j]->valor.capacidade_instalada,
                        registros[j]->valor.perdas_geracao_total
                    );

                    if (usado + n + 1 >= capacidade) {
                        capacidade *= 2;
                        tmp_buffer = realloc(tmp_buffer, capacidade);
                        if (!tmp_buffer) {
                            perror("realloc");
                            exit(1);
                        }
                    }

                    memcpy(tmp_buffer + usado, item, n);
                    usado += n;
                    tmp_buffer[usado] = '\0';
                    primeiro = 0;
                }
            }
        }
    }

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

    *saida = tmp_buffer;
}



void exibirCuckoo() {
    int contador = 0;
    printf("Exibindo dados da Tabela Cuckoo:\n");
    printf("Tamanho atual: %zu | Quantidade de itens: %zu\n", tamanho_atual, quantidade_itens);
    printf("------------------------------------------------------------\n");

    for (size_t i = 0; i < tamanho_atual; i++) {
        if (tabela1[i].ocupado) {
            printf("[Tabela 1][%zu]: Chave: %s\n", i, tabela1[i].chave);
            printf("  Data: %s\n", tabela1[i].valor.data);
            printf("  Demanda Residual: %.2f\n", tabela1[i].valor.demanda_residual);
            printf("  Demanda Contratada: %.2f\n", tabela1[i].valor.demanda_contratada);
            printf("  Geração Despachável: %.2f\n", tabela1[i].valor.geracao_despachavel);
            printf("  Geração Térmica: %.2f\n", tabela1[i].valor.geracao_termica);
            printf("  Importações: %.2f\n", tabela1[i].valor.importacoes);
            printf("  Geração Renovável Total: %.2f\n", tabela1[i].valor.geracao_renovavel_total);
            printf("  Carga Reduzida Manual: %.2f\n", tabela1[i].valor.carga_reduzida_manual);
            printf("  Capacidade Instalada: %.2f\n", tabela1[i].valor.capacidade_instalada);
            printf("  Perdas Geração Total: %.2f\n", tabela1[i].valor.perdas_geracao_total);
            printf("------------------------------------------------------------\n");
            contador++;
        }
    }

    for (size_t i = 0; i < tamanho_atual; i++) {
        if (tabela2[i].ocupado) {
            printf("[Tabela 2][%zu]: Chave: %s\n", i, tabela2[i].chave);
            printf("  Data: %s\n", tabela2[i].valor.data);
            printf("  Demanda Residual: %.2f\n", tabela2[i].valor.demanda_residual);
            printf("  Demanda Contratada: %.2f\n", tabela2[i].valor.demanda_contratada);
            printf("  Geração Despachável: %.2f\n", tabela2[i].valor.geracao_despachavel);
            printf("  Geração Térmica: %.2f\n", tabela2[i].valor.geracao_termica);
            printf("  Importações: %.2f\n", tabela2[i].valor.importacoes);
            printf("  Geração Renovável Total: %.2f\n", tabela2[i].valor.geracao_renovavel_total);
            printf("  Carga Reduzida Manual: %.2f\n", tabela2[i].valor.carga_reduzida_manual);
            printf("  Capacidade Instalada: %.2f\n", tabela2[i].valor.capacidade_instalada);
            printf("  Perdas Geração Total: %.2f\n", tabela2[i].valor.perdas_geracao_total);
            printf("------------------------------------------------------------\n");
            contador++;
        }
    }

    printf("Numero de itens: %d", contador);
}

//---------------------- Funções estatísticas ----------------------

EstatisticasCamposCC calcular_estatisticas_cc(VetorRegistro1 *v) {
    EstatisticasCamposCC stats = {0};
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
        Registro1 *r1 = &v->dados[i];

        sum_demanda_residual += r1->demanda_residual;
        sumsq_demanda_residual += r1->demanda_residual * r1->demanda_residual;

        sum_demanda_contratada += r1->demanda_contratada;
        sumsq_demanda_contratada += r1->demanda_contratada * r1->demanda_contratada;

        sum_geracao_despachavel += r1->geracao_despachavel;
        sumsq_geracao_despachavel += r1->geracao_despachavel * r1->geracao_despachavel;

        sum_geracao_renovavel_total += r1->geracao_renovavel_total;
        sumsq_geracao_renovavel_total += r1->geracao_renovavel_total * r1->geracao_renovavel_total;

        sum_carga_reduzida_manual += r1->carga_reduzida_manual;
        sumsq_carga_reduzida_manual += r1->carga_reduzida_manual * r1->carga_reduzida_manual;

        sum_capacidade_instalada += r1->capacidade_instalada;
        sumsq_capacidade_instalada += r1->capacidade_instalada * r1->capacidade_instalada;

        sum_perdas_geracao_total += r1->perdas_geracao_total;
        sumsq_perdas_geracao_total += r1->perdas_geracao_total * r1->perdas_geracao_total;

        sum_geracao_termica += r1->geracao_termica;
        sumsq_geracao_termica += r1->geracao_termica * r1->geracao_termica;

        sum_importacoes += r1->importacoes;
        sumsq_importacoes += r1->importacoes * r1->importacoes;
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

int comparador_double_cc(const void *a, const void *b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

// Função para pegar mediana de array ordenado
static double pegar_mediana_cc(double *arr, size_t n) {
    if (n == 0) return 0.0;
    if (n % 2 == 1)
        return arr[n / 2];
    else
        return (arr[(n/2) - 1] + arr[n/2]) / 2.0;
}

// Comparação "aproximada" de doubles
static int doubles_iguais_cc(double a, double b) {
    return fabs(a - b) < EPSILON_cc;
}

// Função para pegar moda de array ordenado
static double pegar_moda_cc(double *arr, size_t n) {
    if (n == 0) return 0.0;

    double moda = arr[0];
    size_t maior_contagem = 1;
    size_t contagem_atual = 1;

    for (size_t i = 1; i < n; i++) {
        if (doubles_iguais_cc(arr[i], arr[i-1])) {
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

MedianasCC calcular_mediana_cc(VetorRegistro1 *v) {
    MedianasCC medianas = {0};
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
        Registro1 *r1 = &v->dados[i];
        arr_demanda_residual[i] = r1->demanda_residual;
        arr_demanda_contratada[i] = r1->demanda_contratada;
        arr_geracao_despachavel[i] = r1->geracao_despachavel;
        arr_geracao_renovavel_total[i] = r1->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = r1->carga_reduzida_manual;
        arr_capacidade_instalada[i] = r1->capacidade_instalada;
        arr_perdas_geracao_total[i] = r1->perdas_geracao_total;
        arr_geracao_termica[i] = r1->geracao_termica;
        arr_importacoes[i] = r1->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double_cc);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double_cc);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double_cc);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double_cc);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double_cc);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double_cc);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double_cc);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double_cc);
    qsort(arr_importacoes, n, sizeof(double), comparador_double_cc);

    medianas.demanda_residual = pegar_mediana_cc(arr_demanda_residual, n);
    medianas.demanda_contratada = pegar_mediana_cc(arr_demanda_contratada, n);
    medianas.geracao_despachavel = pegar_mediana_cc(arr_geracao_despachavel, n);
    medianas.geracao_renovavel_total = pegar_mediana_cc(arr_geracao_renovavel_total, n);
    medianas.carga_reduzida_manual = pegar_mediana_cc(arr_carga_reduzida_manual, n);
    medianas.capacidade_instalada = pegar_mediana_cc(arr_capacidade_instalada, n);
    medianas.perdas_geracao_total = pegar_mediana_cc(arr_perdas_geracao_total, n);
    medianas.geracao_termica = pegar_mediana_cc(arr_geracao_termica, n);
    medianas.importacoes = pegar_mediana_cc(arr_importacoes, n);

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

ModasCC calcular_moda_cc(VetorRegistro1 *v) {
    ModasCC modas = {0};
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
        Registro1 *r1 = &v->dados[i];
        arr_demanda_residual[i] = r1->demanda_residual;
        arr_demanda_contratada[i] = r1->demanda_contratada;
        arr_geracao_despachavel[i] = r1->geracao_despachavel;
        arr_geracao_renovavel_total[i] = r1->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = r1->carga_reduzida_manual;
        arr_capacidade_instalada[i] = r1->capacidade_instalada;
        arr_perdas_geracao_total[i] = r1->perdas_geracao_total;
        arr_geracao_termica[i] = r1->geracao_termica;
        arr_importacoes[i] = r1->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double_cc);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double_cc);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double_cc);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double_cc);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double_cc);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double_cc);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double_cc);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double_cc);
    qsort(arr_importacoes, n, sizeof(double), comparador_double_cc);

    modas.demanda_residual = pegar_moda_cc(arr_demanda_residual, n);
    modas.demanda_contratada = pegar_moda_cc(arr_demanda_contratada, n);
    modas.geracao_despachavel = pegar_moda_cc(arr_geracao_despachavel, n);
    modas.geracao_renovavel_total = pegar_moda_cc(arr_geracao_renovavel_total, n);
    modas.carga_reduzida_manual = pegar_moda_cc(arr_carga_reduzida_manual, n);
    modas.capacidade_instalada = pegar_moda_cc(arr_capacidade_instalada, n);
    modas.perdas_geracao_total = pegar_moda_cc(arr_perdas_geracao_total, n);
    modas.geracao_termica = pegar_moda_cc(arr_geracao_termica, n);
    modas.importacoes = pegar_moda_cc(arr_importacoes, n);

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

char* estatisticas_para_json_conteudo_cc(EstatisticasCamposCC est, MedianasCC med, ModasCC moda) {
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

