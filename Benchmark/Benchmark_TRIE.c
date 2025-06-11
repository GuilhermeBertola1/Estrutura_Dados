#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Trie.h"

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

    Trie* trie = criar_trie();
    char linha20[4096];
    VetorEntrada2 vetor;
    vetor_inicializar_Trie(&vetor, 1024);
    fgets(linha20, sizeof(linha20), f);
    start = clock();
    while (fgets(linha20, sizeof(linha20), f)) {
        Entrada2 t1;
        char *token = strtok(linha20, ",");
        if (!token) continue;

        strncpy(t1.data, token, sizeof(t1.data));
        t1.data[sizeof(t1.data) - 1] = '\0';

        // Avança tokens para preencher os campos conforme ordem e quantidade puladas no CSV
        for (int i = 0; i < 5; i++) token = strtok(NULL, ",");
        if (!token) continue;
        t1.demanda_residual = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        t1.demanda_contratada = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        t1.importacoes = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        t1.geracao_termica = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        t1.geracao_despachavel = atof(token);

        for (int i = 0; i < 6; i++) token = strtok(NULL, ",");
        if (!token) continue;
        t1.geracao_renovavel_total = atof(token);

        for (int i = 0; i < 7; i++) token = strtok(NULL, ",");
        if (!token) continue;
        t1.capacidade_instalada = atof(token);

        for (int i = 0; i < 3; i++) token = strtok(NULL, ",");
        if (!token) continue;
        t1.perdas_geracao_total = atof(token);

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        t1.carga_reduzida_manual = atof(token);

        t1.ocupado = 1;

        inserir_trie_temporal(trie, t1);
    }end = clock();
    printf("Tempo insercao Trie: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    buscar_intervalo_trie(trie, data_inicio, data_fim, &resposta_json, &vetor);
    end = clock();
    printf("Tempo busca Trie: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    EstatisticasCamposTrie est = calcular_estatisticas_Trie(&vetor);
    MedianasTrie med = calcular_mediana_Trie(&vetor);
    ModasTrie moda = calcular_moda_Trie(&vetor);
    end = clock();
    printf("Tempo de calculo estatistico da AVL: %f s\n", (double)(end - start) / CLOCKS_PER_SEC);

    free(resposta_json);
    vetor_liberar_Trie(&vetor);
    destruir_trie(trie);
}