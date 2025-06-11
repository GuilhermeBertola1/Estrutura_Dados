#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <dirent.h>
#include <math.h>
#include "Lsm.h"

Dados memtable[MAX_MEMTABLE];
int memtable_size = 0;
int sstable_counter = 0;

char* get_lsm_directory() {
    static char path[1024];
    strcpy(path, __FILE__);  // __FILE__ é o caminho completo de Lsm.c
    char *last_slash = strrchr(path, '/');
    if (last_slash != NULL) {
        *last_slash = '\0';  // Remove o nome do arquivo, deixa só o diretório
    } else {
        strcpy(path, ".");   // Se não encontrar barra, assume diretório atual
    }
    return path;
}

long long datetime_para_inteiro_LSM(const char *datetime) {
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

int comparar_dados(const void *a, const void *b) {
    const Dados *d1 = (const Dados *)a;
    const Dados *d2 = (const Dados *)b;
    return strcmp(d1->data, d2->data);
}

void merge_sstables(const char *file1, const char *file2, const char *output) {
    FILE *f1 = fopen(file1, "rb");
    FILE *f2 = fopen(file2, "rb");
    FILE *out = fopen(output, "wb");

    if (!f1 || !f2 || !out) {
        perror("Erro ao abrir arquivos para merge");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        if (out) fclose(out);
        exit(EXIT_FAILURE);
    }

    Dados d1, d2;
    int r1 = fread(&d1, sizeof(Dados), 1, f1);
    int r2 = fread(&d2, sizeof(Dados), 1, f2);

    while (r1 && r2) {
        if (strcmp(d1.data, d2.data) <= 0) {
            fwrite(&d1, sizeof(Dados), 1, out);
            r1 = fread(&d1, sizeof(Dados), 1, f1);
        } else {
            fwrite(&d2, sizeof(Dados), 1, out);
            r2 = fread(&d2, sizeof(Dados), 1, f2);
        }
    }

    while (r1) {
        fwrite(&d1, sizeof(Dados), 1, out);
        r1 = fread(&d1, sizeof(Dados), 1, f1);
    }
    while (r2) {
        fwrite(&d2, sizeof(Dados), 1, out);
        r2 = fread(&d2, sizeof(Dados), 1, f2);
    }

    fclose(f1);
    fclose(f2);
    fclose(out);

    // Agora apaga os arquivos originais:
    remove(file1);
    remove(file2);

    // E renomeia o arquivo output para o nome do primeiro arquivo, se quiser:
    rename(output, file1);
}

void flush_memtable() {
    qsort(memtable, memtable_size, sizeof(Dados), comparar_dados);

    char filename[64];
    snprintf(filename, sizeof(filename), "%s/sstable_%03d.dat", get_lsm_directory(), sstable_counter++);
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("Erro ao criar SSTable");
        exit(EXIT_FAILURE);
    }
    fwrite(memtable, sizeof(Dados), memtable_size, f);
    fclose(f);

    memtable_size = 0;

    // Merge automático se houver mais de 2 arquivos
    if (sstable_counter >= 2) {
        char file1[128], file2[128], merged[128];

        snprintf(file1, sizeof(file1), "%s/sstable_%03d.dat", get_lsm_directory(), sstable_counter - 2);
        snprintf(file2, sizeof(file2), "%s/sstable_%03d.dat", get_lsm_directory(), sstable_counter - 1);
        snprintf(merged, sizeof(merged), "%s/sstable_%03d_merged.dat", get_lsm_directory(), sstable_counter - 2);

        merge_sstables(file1, file2, merged);
        sstable_counter--;
    }
}

int inserir_dado(const Dados *dado) {
    if (memtable_size >= MAX_MEMTABLE) {
        flush_memtable();
        memtable_size = 0;
    }

    memtable[memtable_size++] = *dado;
    return 1;
}

void vetor_inicializar_lsm (VetorDados *v, size_t capacidade_inicial){
     v->dados = malloc(sizeof(Dados) * capacidade_inicial);
    if (!v->dados) {
        perror("malloc vetor");
        exit(1);
    }
    v->tamanho = 0;
    v->capacidade = capacidade_inicial;
}

void vetor_adicionar_lsm (VetorDados *v, Dados reg){
     if (v->tamanho == v->capacidade) {
        v->capacidade *= 2;
        v->dados = realloc(v->dados, sizeof(Dados) * v->capacidade);
        if (!v->dados) {
            perror("realloc vetor");
            exit(1);
        }
    }
    v->dados[v->tamanho++] = reg;
}

void vetor_liberar_lsm (VetorDados *v) {
    free(v->dados);
    v->dados = NULL;
    v->tamanho = 0;
    v->capacidade = 0;
}

void buscar_intervalo_lsm(const char *inicio_str, const char *fim_str, char **saida, VetorDados *vetor) {
    long long inicio = datetime_para_inteiro_LSM(inicio_str);
    long long fim = datetime_para_inteiro_LSM(fim_str);

    size_t capacidade = 8192;
    size_t usado = 0;
    int primeiro = 1;

    char *json = malloc(capacidade);
    if (!json) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    usado += snprintf(json + usado, capacidade - usado, "[");

    // Abrir diretório "LSM_tree/"
    DIR *dir = opendir("LSM_tree");
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    int contagem = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "sstable_", 8) == 0 && strstr(entry->d_name, ".dat")) {
            // Montar o caminho completo do arquivo
            char caminho[1024];
            snprintf(caminho, sizeof(caminho), "LSM_tree/%s", entry->d_name);
            //printf("%s\n", caminho);

            FILE *f = fopen(caminho, "rb");
            if (!f) continue;

            Dados d;
            while (fread(&d, sizeof(Dados), 1, f) == 1) {
                long long data_valor = datetime_para_inteiro_LSM(d.data);

                if (data_valor >= inicio && data_valor <= fim) {
                    contagem++;
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
                        d.data,
                        d.demanda_residual,
                        d.demanda_contratada,
                        d.geracao_despachavel,
                        d.geracao_termica,
                        d.importacoes,
                        d.geracao_renovavel_total,
                        d.carga_reduzida_manual,
                        d.capacidade_instalada,
                        d.perdas_geracao_total
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

            fclose(f);
        }
    }

    closedir(dir);

    for (int i = 0; i < memtable_size; ++i) {
        long long data_valor = datetime_para_inteiro_LSM(memtable[i].data);
        if (data_valor >= inicio && data_valor <= fim) {
            contagem++;
            if (vetor != NULL) {
                vetor_adicionar_lsm(vetor, memtable[i]);
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
                memtable[i].data,
                memtable[i].demanda_residual,
                memtable[i].demanda_contratada,
                memtable[i].geracao_despachavel,
                memtable[i].geracao_termica,
                memtable[i].importacoes,
                memtable[i].geracao_renovavel_total,
                memtable[i].carga_reduzida_manual,
                memtable[i].capacidade_instalada,
                memtable[i].perdas_geracao_total
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

    usado += snprintf(json + usado, capacidade - usado, "\n]");
    json[usado] = '\0';

    *saida = json;
    //printf("Itens encontrados no intervalo: %d\n", contagem);
}

int eh_arquivo_sstable(const char *nome) {
    return strncmp(nome, "sstable_", 8) == 0 && strstr(nome, ".dat") != NULL;
}

void printar_arquivo(const char *caminho_completo, const char *nome) {
    FILE *fp = fopen(caminho_completo, "rb");
    if (!fp) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    Dados d;
    printf("Conteúdo de %s:\n", nome);
    printf("-----------------------------------------------------------------------------------------------------------\n");
    printf("| Data                    | DemandaRes | DemandaCont | Despachável | Térmica  | Import. | Renovável | Cap.Inst. |\n");
    printf("-----------------------------------------------------------------------------------------------------------\n");

    while (fread(&d, sizeof(Dados), 1, fp)) {
        printf("| %-22s | %11.2f | %12.2f | %11.2f | %8.2f | %7.2f | %9.2f | %9.2f |\n",
               d.data,
               d.demanda_residual,
               d.demanda_contratada,
               d.geracao_despachavel,
               d.geracao_termica,
               d.importacoes,
               d.geracao_renovavel_total,
               d.capacidade_instalada
        );
    }

    printf("-----------------------------------------------------------------------------------------------------------\n\n");

    fclose(fp);
}

void printar_dados_todos_arquivos(const char *pasta) {
    DIR *dir = opendir(pasta);
    if (!dir) {
        perror("Erro ao abrir diretório");
        return;
    }

    struct dirent *entry;
    char caminho[256];

    while ((entry = readdir(dir)) != NULL) {
        if (eh_arquivo_sstable(entry->d_name)) {
            snprintf(caminho, sizeof(caminho), "%s/%s", pasta, entry->d_name);
            printar_arquivo(caminho, entry->d_name);
        }
    }

    closedir(dir);
}

//---------------------- Funções estatísticas ----------------------

EstatisticasCamposlsm calcular_estatisticas_lsm(VetorDados *v) {
    EstatisticasCamposlsm stats = {0};
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
        Dados *d1 = &v->dados[i];

        sum_demanda_residual += d1->demanda_residual;
        sumsq_demanda_residual += d1->demanda_residual * d1->demanda_residual;

        sum_demanda_contratada += d1->demanda_contratada;
        sumsq_demanda_contratada += d1->demanda_contratada * d1->demanda_contratada;

        sum_geracao_despachavel += d1->geracao_despachavel;
        sumsq_geracao_despachavel += d1->geracao_despachavel * d1->geracao_despachavel;

        sum_geracao_renovavel_total += d1->geracao_renovavel_total;
        sumsq_geracao_renovavel_total += d1->geracao_renovavel_total * d1->geracao_renovavel_total;

        sum_carga_reduzida_manual += d1->carga_reduzida_manual;
        sumsq_carga_reduzida_manual += d1->carga_reduzida_manual * d1->carga_reduzida_manual;

        sum_capacidade_instalada += d1->capacidade_instalada;
        sumsq_capacidade_instalada += d1->capacidade_instalada * d1->capacidade_instalada;

        sum_perdas_geracao_total += d1->perdas_geracao_total;
        sumsq_perdas_geracao_total += d1->perdas_geracao_total * d1->perdas_geracao_total;

        sum_geracao_termica += d1->geracao_termica;
        sumsq_geracao_termica += d1->geracao_termica * d1->geracao_termica;

        sum_importacoes += d1->importacoes;
        sumsq_importacoes += d1->importacoes * d1->importacoes;
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

int comparador_double_lsm(const void *a, const void *b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

// Função para pegar mediana de array ordenado
static double pegar_mediana_lsm(double *arr, size_t n) {
    if (n == 0) return 0.0;
    if (n % 2 == 1)
        return arr[n / 2];
    else
        return (arr[(n/2) - 1] + arr[n/2]) / 2.0;
}

// Comparação "aproximada" de doubles
static int doubles_iguais_lsm(double a, double b) {
    return fabs(a - b) < EPSILON_lsm;
}

// Função para pegar moda de array ordenado
static double pegar_moda_lsm(double *arr, size_t n) {
    if (n == 0) return 0.0;

    double moda = arr[0];
    size_t maior_contagem = 1;
    size_t contagem_atual = 1;

    for (size_t i = 1; i < n; i++) {
        if (doubles_iguais_lsm(arr[i], arr[i-1])) {
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

Medianaslsm calcular_mediana_lsm(VetorDados *v) {
    Medianaslsm medianas = {0};
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
        Dados *d1 = &v->dados[i];
        arr_demanda_residual[i] = d1->demanda_residual;
        arr_demanda_contratada[i] = d1->demanda_contratada;
        arr_geracao_despachavel[i] = d1->geracao_despachavel;
        arr_geracao_renovavel_total[i] = d1->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = d1->carga_reduzida_manual;
        arr_capacidade_instalada[i] = d1->capacidade_instalada;
        arr_perdas_geracao_total[i] = d1->perdas_geracao_total;
        arr_geracao_termica[i] = d1->geracao_termica;
        arr_importacoes[i] = d1->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double_lsm);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double_lsm);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double_lsm);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double_lsm);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double_lsm);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double_lsm);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double_lsm);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double_lsm);
    qsort(arr_importacoes, n, sizeof(double), comparador_double_lsm);

    medianas.demanda_residual = pegar_mediana_lsm(arr_demanda_residual, n);
    medianas.demanda_contratada = pegar_mediana_lsm(arr_demanda_contratada, n);
    medianas.geracao_despachavel = pegar_mediana_lsm(arr_geracao_despachavel, n);
    medianas.geracao_renovavel_total = pegar_mediana_lsm(arr_geracao_renovavel_total, n);
    medianas.carga_reduzida_manual = pegar_mediana_lsm(arr_carga_reduzida_manual, n);
    medianas.capacidade_instalada = pegar_mediana_lsm(arr_capacidade_instalada, n);
    medianas.perdas_geracao_total = pegar_mediana_lsm(arr_perdas_geracao_total, n);
    medianas.geracao_termica = pegar_mediana_lsm(arr_geracao_termica, n);
    medianas.importacoes = pegar_mediana_lsm(arr_importacoes, n);

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

Modaslsm calcular_moda_lsm(VetorDados *v) {
    Modaslsm modas = {0};
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
        Dados *d1 = &v->dados[i];
        arr_demanda_residual[i] = d1->demanda_residual;
        arr_demanda_contratada[i] = d1->demanda_contratada;
        arr_geracao_despachavel[i] = d1->geracao_despachavel;
        arr_geracao_renovavel_total[i] = d1->geracao_renovavel_total;
        arr_carga_reduzida_manual[i] = d1->carga_reduzida_manual;
        arr_capacidade_instalada[i] = d1->capacidade_instalada;
        arr_perdas_geracao_total[i] = d1->perdas_geracao_total;
        arr_geracao_termica[i] = d1->geracao_termica;
        arr_importacoes[i] = d1->importacoes;
    }

    qsort(arr_demanda_residual, n, sizeof(double), comparador_double_lsm);
    qsort(arr_demanda_contratada, n, sizeof(double), comparador_double_lsm);
    qsort(arr_geracao_despachavel, n, sizeof(double), comparador_double_lsm);
    qsort(arr_geracao_renovavel_total, n, sizeof(double), comparador_double_lsm);
    qsort(arr_carga_reduzida_manual, n, sizeof(double), comparador_double_lsm);
    qsort(arr_capacidade_instalada, n, sizeof(double), comparador_double_lsm);
    qsort(arr_perdas_geracao_total, n, sizeof(double), comparador_double_lsm);
    qsort(arr_geracao_termica, n, sizeof(double), comparador_double_lsm);
    qsort(arr_importacoes, n, sizeof(double), comparador_double_lsm);

    modas.demanda_residual = pegar_moda_lsm(arr_demanda_residual, n);
    modas.demanda_contratada = pegar_moda_lsm(arr_demanda_contratada, n);
    modas.geracao_despachavel = pegar_moda_lsm(arr_geracao_despachavel, n);
    modas.geracao_renovavel_total = pegar_moda_lsm(arr_geracao_renovavel_total, n);
    modas.carga_reduzida_manual = pegar_moda_lsm(arr_carga_reduzida_manual, n);
    modas.capacidade_instalada = pegar_moda_lsm(arr_capacidade_instalada, n);
    modas.perdas_geracao_total = pegar_moda_lsm(arr_perdas_geracao_total, n);
    modas.geracao_termica = pegar_moda_lsm(arr_geracao_termica, n);
    modas.importacoes = pegar_moda_lsm(arr_importacoes, n);

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

char* estatisticas_para_json_conteudo_lsm(EstatisticasCamposlsm est, Medianaslsm med, Modaslsm moda) {
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
