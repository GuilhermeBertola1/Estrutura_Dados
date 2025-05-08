#include <stdio.h>
#include <stdlib.h>

void imprimir_csv(FILE *fptr);

int main() {
    FILE *fptr = fopen("dataset/ESK2033.csv", "r");

    if (fptr == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    imprimir_csv(fptr);

    rewind(fptr);

    getchar();
    fclose(fptr);
    return 0;
}

void imprimir_csv(FILE *fptr) {
    char linha[2048];

    while (fgets(linha, sizeof(linha), fptr)) {
        printf("%s", linha);
    }
}