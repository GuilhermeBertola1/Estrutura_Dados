#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <dirent.h>
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
    sscanf(datetime, "%d-%d-%d %d:%d:%d", &ano, &mes, &dia, &hora, &min, &seg);
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

void buscar_intervalo_lsm(const char *inicio_str, const char *fim_str, char **saida) {
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
            printf("%s\n", caminho);

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
    printf("Itens encontrados no intervalo: %d\n", contagem);
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
