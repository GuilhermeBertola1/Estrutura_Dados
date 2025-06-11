#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "List.h"

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

    char linha3[4096];
    VetorEletricDates vetor;
    vetor_inicializarList(&vetor, 1024);
    fgets(linha3, sizeof(linha3), f);
    start = clock();
    while (fgets(linha3, sizeof(linha3), f)) {
        EletricDates list;
        char *token = strtok(linha3, ",");

        if (!token) continue;

        // Copia data direto (esperando formato com AM/PM)
        strncpy(list.data, token, sizeof(list.data));
        list.data[sizeof(list.data)-1] = '\0';

        // Avança tokens e lê os campos que quer
        for (int i = 0; i < 5; i++) token = strtok(NULL, ",");
        if (!token) continue;
        list.demanda_residual = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        list.demanda_contratada = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        list.importacoes = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        list.geracao_termica = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        list.geracao_despachavel = atof(token);

        for (int i = 0; i < 6; i++) token = strtok(NULL, ",");
        if (!token) continue;
        list.geracao_renovavel_total = atof(token);

        for (int i = 0; i < 7; i++) token = strtok(NULL, ",");
        if (!token) continue;
        list.capacidade_instalada = atof(token);

        for (int i = 0; i < 3; i++) token = strtok(NULL, ",");
        if (!token) continue;
        list.perdas_geracao_total = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        list.carga_reduzida_manual = atof(token);

        if (!in(&lista_ligada, &list)) {
            fprintf(stderr, "Falha ao inserir registro: %s\n", list.data);
        }
    }end = clock();
    printf("Tempo insercao Lista encadeada: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    buscar_intervalo_list(data_inicio, data_fim, &resposta_json, &vetor);
    end = clock();
    printf("Tempo busca Lista encadeada: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    EstatisticasCamposList est = calcular_estatisticasList(&vetor);
    MedianasList med = calcular_mediana_list(&vetor);
    ModasList moda = calcular_moda_list(&vetor);
    end = clock();
    printf("Tempo de calculo estatistico da Lista encadeada: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    free(resposta_json);
    vetor_liberarList(&vetor);
    liberar_lista(&lista_ligada);
}