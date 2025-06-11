#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Hash.h"

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

    inicializarHash();
    char linha2[4096];
    VetorEntrada vetor;
    vetor_inicializar_HT(&vetor, 1023);
    fgets(linha2, sizeof(linha2), f);
    start = clock();
    while (fgets(linha2, sizeof(linha2), f)) {
        Entrada r2;
        char *token = strtok(linha2, ",");
        if (!token) continue;

        strncpy(r2.data, token, sizeof(r2.data));
        r2.data[sizeof(r2.data) - 1] = '\0';

        // Avança tokens para preencher os campos conforme ordem e quantidade puladas no CSV
        for (int i = 0; i < 5; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r2.demanda_residual = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        r2.demanda_contratada = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        r2.importacoes = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r2.geracao_termica = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r2.geracao_despachavel = atof(token);

        for (int i = 0; i < 6; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r2.geracao_renovavel_total = atof(token);

        for (int i = 0; i < 7; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r2.capacidade_instalada = atof(token);

        for (int i = 0; i < 3; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r2.perdas_geracao_total = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r2.carga_reduzida_manual = atof(token);

        if (!inserirHash_linear(&r2)) {
            fprintf(stderr, "Falha ao inserir registro: %s\n", r2.data);
        }
    }end = clock();
    printf("Tempo insercao Hashing table: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    buscar_intervalo_HT(data_inicio, data_fim, &resposta_json, &vetor);
    end = clock();
    printf("Tempo busca Hashing table: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    EstatisticasCamposHT est = calcular_estatisticas_HT(&vetor);
    MedianasHT med = calcular_mediana_HT(&vetor);
    ModasHT moda = calcular_moda_HT(&vetor);
    end = clock();
    printf("Tempo de calculo estatistico da Tabela Hash: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    free(resposta_json);
    vetor_liberar_HT(&vetor);
    liberarHashLinear();
}