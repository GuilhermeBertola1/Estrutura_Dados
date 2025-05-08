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

    char linha[2048];
    Node *raiz = NULL;

    fgets(linha, sizeof(linha), f);

    while (fgets(linha, sizeof(linha), f)) {
        Registro r;

        // Ajuste os índices conforme seu CSV real
        sscanf(linha, "%[^,],%lf,%d,%lf,%lf,%lf",
               r.data, &r.consumo, &r.clientes,
               &r.receita_total, &r.preco_kwh, &r.uso_medio);

        raiz = inserir(raiz, r);
    }

    fclose(f);

    printf("\nDados ordenados pela data:\n");
    em_ordem(raiz);

    getchar();
    return 0;
}
