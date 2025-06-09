#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Bm.h"

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
    
    char linha10[4096];
    BPlusNode *raizBM = NULL; 
    fgets(linha10, sizeof(linha10), f);

    start = clock();
    while (fgets(linha10, sizeof(linha10), f)) {
        Bdados b1;
        char *token = strtok(linha10, ",");
        if (!token) continue;

        strncpy(b1.data, token, sizeof(b1.data));
        b1.data[sizeof(b1.data) - 1] = '\0';

        for (int i = 0; i < 5; i++) token = strtok(NULL, ",");
        if (!token) continue;
        b1.demanda_residual = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        b1.demanda_contratada = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        b1.importacoes = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        b1.geracao_termica = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        b1.geracao_despachavel = atof(token);

        for (int i = 0; i < 6; i++) token = strtok(NULL, ",");
        if (!token) continue;
        b1.geracao_renovavel_total = atof(token);

        for (int i = 0; i < 7; i++) token = strtok(NULL, ",");
        if (!token) continue;
        b1.capacidade_instalada = atof(token);

        for (int i = 0; i < 3; i++) token = strtok(NULL, ",");
        if (!token) continue;
        b1.perdas_geracao_total = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        b1.carga_reduzida_manual = atof(token);

        inserir_bplus(&raizBM, b1);
    }end = clock();
    printf("Tempo insercao B+ tree: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    buscar_intervalo_bplus_json(raizBM, data_inicio, data_fim, &resposta_json);
    end = clock();
    printf("Tempo busca B+ tree: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);
    free(resposta_json);
    liberar_bplus(raizBM);
}