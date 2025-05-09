#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

int main() {
    FILE *f = fopen("ESK2033.csv", "r");
    if (!f) {
        perror("Erro ao abrir o CSV");
        return 1;
    }

    char linha[4096];
    Node *raiz = NULL;

    fgets(linha, sizeof(linha), f);

    while (fgets(linha, sizeof(linha), f)) {
        Registro r;
        char *token = strtok(linha, ",");

        if (!token) continue;
        strncpy(r.data, token, sizeof(r.data));

        for (int i = 0; i < 5; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.demanda_residual = atof(token);       // Residual Demand

        token = strtok(NULL, ",");
        if (!token) continue;
        r.demanda_contratada = atof(token);     // RSA Contracted Demand

        token = strtok(NULL, ",");
        if (!token) continue;
        r.importacoes = atof(token);            // International Imports

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.geracao_termica = atof(token);        // Thermal Generation

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.geracao_despachavel = atof(token);    // Dispatchable Generation

        raiz = inserir(raiz, r);
    }

    fclose(f);

    printf("\nDados ordenados por data-hora:\n");
    em_ordem(raiz);

    getchar();
    return 0;
}
