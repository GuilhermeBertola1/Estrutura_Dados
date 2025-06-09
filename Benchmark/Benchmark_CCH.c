#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Cuckoo_Hashing/CcH.h"

int main(){
    FILE *f = fopen("dataset/ESK2033.csv", "r");
    if (!f) {
        perror("Erro ao abrir o CSV");
        return 1;
    }
    char data_inicio[32] = "2018-04-01 12:00:00 AM";
    char data_fim[32] = "2022-07-19 11:00:00 PM";
    char* resposta_json;
    clock_t start, end;

    inicializarCuckoo(2053);
    char linha1[4096];
    fgets(linha1, sizeof(linha1), f);
    start = clock();
    while (fgets(linha1, sizeof(linha1), f)) {
        Registro1 r1;
        char *token = strtok(linha1, ",");
        if (!token) continue;

        strncpy(r1.data, token, sizeof(r1.data));
        r1.data[sizeof(r1.data) - 1] = '\0';

        // Avança tokens para preencher os campos conforme ordem e quantidade puladas no CSV
        for (int i = 0; i < 5; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r1.demanda_residual = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        r1.demanda_contratada = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        r1.importacoes = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r1.geracao_termica = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r1.geracao_despachavel = atof(token);

        for (int i = 0; i < 6; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r1.geracao_renovavel_total = atof(token);

        for (int i = 0; i < 7; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r1.capacidade_instalada = atof(token);

        for (int i = 0; i < 3; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r1.perdas_geracao_total = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r1.carga_reduzida_manual = atof(token);

        if (!inserirCuckoo(&r1)) {
            fprintf(stderr, "Falha ao inserir registro: %s\n", r1.data);
        }
    }end = clock();
    printf("Tempo insercao Cuckoo Hashing: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    buscar_intervalo_cuckoo(data_inicio, data_fim, &resposta_json);
    end = clock();
    printf("Tempo busca Cuckoo Hashing: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);
    free(resposta_json);
    liberarCuckoo();
}