#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "CmpList.h"

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

    Bloco* CmpLISTA = NULL;
    CmpLISTA = criar_bloco();
    char linha90[4096];
    VetorCompList vetor;
    vetor_inicializarListC(&vetor, 1024);
    fgets(linha90, sizeof(linha90), f);
    start = clock();
    while (fgets(linha90, sizeof(linha90), f)) {
        CompList g1;
        char *token = strtok(linha90, ",");
        if (!token) continue;

        strncpy(g1.data, token, sizeof(g1.data));
        g1.data[sizeof(g1.data) - 1] = '\0';

        // Avança tokens para preencher os campos conforme ordem e quantidade puladas no CSV
        for (int i = 0; i < 5; i++) token = strtok(NULL, ",");
        if (!token) continue;
        g1.demanda_residual = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        g1.demanda_contratada = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        g1.importacoes = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        g1.geracao_termica = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        g1.geracao_despachavel = atof(token);

        for (int i = 0; i < 6; i++) token = strtok(NULL, ",");
        if (!token) continue;
        g1.geracao_renovavel_total = atof(token);

        for (int i = 0; i < 7; i++) token = strtok(NULL, ",");
        if (!token) continue;
        g1.capacidade_instalada = atof(token);

        for (int i = 0; i < 3; i++) token = strtok(NULL, ",");
        if (!token) continue;
        g1.perdas_geracao_total = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        g1.carga_reduzida_manual = atof(token);

        inserir_CMP(&CmpLISTA, g1);
    }end = clock();
    printf("Tempo insercao Compact List: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    buscar_intervalo_lista_CMP(CmpLISTA, data_inicio, data_fim, &resposta_json, &vetor);
    end = clock();
    printf("Tempo busca Compact List: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    EstatisticasCamposListC est = calcular_estatisticasListC(&vetor);
    MedianasListC med = calcular_mediana_listc(&vetor);
    ModasListC moda = calcular_moda_listc(&vetor);
    end = clock();
    printf("Tempo de calculo estatistico da lista compacta encadeada: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    free(resposta_json);
    vetor_liberarListC(&vetor);
    liberar_lista_CMP(CmpLISTA);
}