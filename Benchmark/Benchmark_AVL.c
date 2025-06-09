#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "AVL/avl.h"

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
    char linha[4096];
    Node *raiz = NULL;
    fgets(linha, sizeof(linha), f);

    start = clock();
    while (fgets(linha, sizeof(linha), f)) {
        Registro r;
        char *token = strtok(linha, ",");

        if (!token) continue;

        strncpy(r.data, token, sizeof(r.data));
        r.data[sizeof(r.data)-1] = '\0';

        
        for (int i = 0; i < 5; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.demanda_residual = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        r.demanda_contratada = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        r.importacoes = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.geracao_termica = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.geracao_despachavel = atof(token);

        for (int i = 0; i < 6; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.geracao_renovavel_total = atof(token);

        for (int i = 0; i < 7; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.capacidade_instalada = atof(token);

        for (int i = 0; i < 3; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.perdas_geracao_total = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.carga_reduzida_manual = atof(token);

        raiz = inserir(raiz, r);
    }end = clock();
    printf("Tempo insercao AVL: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    buscar_intervalo(raiz, data_inicio, data_fim, &resposta_json);
    end = clock();
    printf("Tempo busca AVL: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);
    free(resposta_json);
    liberar_avl(raiz);
}