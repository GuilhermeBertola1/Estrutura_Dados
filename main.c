#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include "AVL/avl.h"

int main() {
    FILE *f = fopen("dataset/ESK2033.csv", "r");
    if (!f) {
        perror("Erro ao abrir o CSV");
        return 1;
    }

    char linha[4096];
    Node *raiz = NULL;

    fgets(linha, sizeof(linha), f); // pula cabeçalho

    while (fgets(linha, sizeof(linha), f)) {
        Registro r;
        char *token = strtok(linha, ",");

        if (!token) continue;
        strncpy(r.data, token, sizeof(r.data));
        r.data[sizeof(r.data)-1] = '\0';

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

        for (int i = 0; i < 6; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.geracao_renovavel_total = atof(token); // Total RE

        for (int i = 0; i < 7; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.capacidade_instalada = atof(token);   // Installed Eskom Capacity

        for (int i = 0; i < 3; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.perdas_geracao_total = atof(token);   // Total UCLF+OCLF

        for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
        if (!token) continue;
        r.carga_reduzida_manual = atof(token);  // Manual Load Reduction (MLR)

        raiz = inserir(raiz, r);
    }

    fclose(f);

    printf("\nDados ordenados por data-hora:\n");
    em_ordem(raiz);

    preencher_vetor_nos(raiz);

    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);

    int i = 0;
    while (1) {
        char buffer [4096];
        zmq_recv (responder, buffer, 4096, 0);
        printf ("Received Hello, %d\n", i);

        char msg[4096];
        if (i < contador) {
            registro_to_json_completo(vetor_nos[i], msg, sizeof(msg));
            zmq_send(responder, msg, strlen(msg), 0);
        } else {
            zmq_send(responder, "FIM", 4, 0);
        }

        i++;
        sleep(1);
    }

    return 0;
}