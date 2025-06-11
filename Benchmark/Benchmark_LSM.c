#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Lsm.h"

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

    char linha4[4096];    
    VetorDados vetor;
    vetor_inicializar_lsm(&vetor, 1024);
    fgets(linha4, sizeof(linha4), f);
    start = clock();
    while (fgets(linha4, sizeof(linha4), f)) {
        Dados d1;
        char *token = strtok(linha4, ",");
        if (!token) continue;

        strncpy(d1.data, token, sizeof(d1.data));
        d1.data[sizeof(d1.data) - 1] = '\0';

        // Avança tokens para preencher os campos conforme ordem e quantidade puladas no CSV
        for (int i = 0; i < 5; i++) token = strtok(NULL, ",");
        if (!token) continue;
        d1.demanda_residual = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        d1.demanda_contratada = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        d1.importacoes = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        d1.geracao_termica = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        d1.geracao_despachavel = atof(token);

        for (int i = 0; i < 6; i++) token = strtok(NULL, ",");
        if (!token) continue;
        d1.geracao_renovavel_total = atof(token);

        for (int i = 0; i < 7; i++) token = strtok(NULL, ",");
        if (!token) continue;
        d1.capacidade_instalada = atof(token);

        for (int i = 0; i < 3; i++) token = strtok(NULL, ",");
        if (!token) continue;
        d1.perdas_geracao_total = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        d1.carga_reduzida_manual = atof(token);

        if (!inserir_dado(&d1)) {
            fprintf(stderr, "Falha ao inserir registro: %s\n", d1.data);
        }
    }end = clock();
    printf("Tempo insercao LSM Tree: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    buscar_intervalo_lsm(data_inicio, data_fim, &resposta_json, &vetor);
    end = clock();
    printf("Tempo busca LSM Tree: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    EstatisticasCamposlsm est = calcular_estatisticas_lsm(&vetor);
    Medianaslsm med = calcular_mediana_lsm(&vetor);
    Modaslsm moda = calcular_moda_lsm(&vetor);
    end = clock();
    printf("Tempo de calculo estatistico da LSM tree: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    vetor_liberar_lsm(&vetor);
    free(resposta_json);
}