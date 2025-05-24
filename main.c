#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include "AVL/avl.h"
#include "Cuckoo_Hashing/CcH.h"
#include "Hash_table/Hash.h"

int main() {
    FILE *f = fopen("dataset/ESK2033.csv", "r");
    if (!f) {
        perror("Erro ao abrir o CSV");
        return 1;
    }

    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);

    const char *red = "\033[0;31m";
    const char *green = "\033[0;32m";
    const char *yellow = "\033[0;33m";
    const char *blue = "\033[0;34m";
    const char *cyan = "\033[0;36m";
    const char *reset = "\033[0m";

    printf("%s===============================%s\n", cyan, reset);
    printf("%s      Selecione a estrutura   %s\n", green, reset);
    printf("%s===============================%s\n", cyan, reset);

    printf("%s[1]%s AVL\n", yellow, reset);
    printf("%s[2]%s Lista Encadeada\n", yellow, reset);
    printf("%s[3]%s Tabela Hash\n", yellow, reset);
    printf("%s[4]%s Cuckoo Hashing\n", yellow, reset);
    printf("%s[5]%s Trie\n", yellow, reset);
    printf("%s[6]%s Dataframe\n", yellow, reset);

    printf("\n%sDigite a opcao desejada:%s ", blue, reset);

    int opcao = 0;
    scanf("%d", &opcao);

    printf("Voce escolheu a opcao %d\n", opcao);

    switch (opcao){
    case 1:
        
        char linha[4096];
        Node *raiz = NULL;

        fgets(linha, sizeof(linha), f); // pula cabeçalho

        while (fgets(linha, sizeof(linha), f)) {
            Registro r;
            char *token = strtok(linha, ",");

            if (!token) continue;

            // Copia data direto (esperando formato com AM/PM)
            strncpy(r.data, token, sizeof(r.data));
            r.data[sizeof(r.data)-1] = '\0';

            // Avança tokens e lê os campos que quer
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
        }

        fclose(f);

        printf("\nDados ordenados por data-hora:\n");
        imprimir_AVL(raiz);

        // Inicializa socket ZeroMQ
        while (1) {
            char buffer[4096];
            int bytes = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
            if (bytes == -1) break;
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\r\n")] = 0;

            char data_inicio[32], data_fim[32];
            if (sscanf(buffer, "%31[^,],%127[^\n]", data_inicio, data_fim) == 2) {
                char *resposta_json;
                printf(data_inicio);
                printf(data_fim);
                buscar_intervalo(raiz, data_inicio, data_fim, &resposta_json);
                zmq_send(responder, resposta_json, strlen(resposta_json), 0);
                free(resposta_json);
            } else {
                const char *msg_erro = "{\"erro\":\"formato inválido, envie 'data_inicio,data_fim'\"}";
                zmq_send(responder, msg_erro, strlen(msg_erro), 0);
            }
        }

        zmq_close(responder);
        zmq_ctx_term(context);

        break;
    case 2:

        
        
        // Inicializa socket ZeroMQ
        while (1) {
            char buffer[4096];
            int bytes = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
            if (bytes == -1) break;
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\r\n")] = 0;

            char data_inicio[32], data_fim[32];
            if (sscanf(buffer, "%31[^,],%127[^\n]", data_inicio, data_fim) == 2) {
                char *resposta_json;
                printf(data_inicio);
                printf(data_fim);
                buscar_intervalo(raiz, data_inicio, data_fim, &resposta_json);
                zmq_send(responder, resposta_json, strlen(resposta_json), 0);
                free(resposta_json);
            } else {
                const char *msg_erro = "{\"erro\":\"formato inválido, envie 'data_inicio,data_fim'\"}";
                zmq_send(responder, msg_erro, strlen(msg_erro), 0);
            }
        }

        zmq_close(responder);
        zmq_ctx_term(context);
        break;
    case 3:

        inicializarHash();
        char linha2[4096];
        // Pula o cabeçalho
        fgets(linha2, sizeof(linha2), f);

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
        }

        fclose(f);
        printf("Dados inseridos na tabela Cuckoo Hashing.\n");

        // Inicializa socket ZeroMQ
        while (1) {
            char buffer[4096];
            int bytes = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
            if (bytes == -1) break;
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\r\n")] = 0;

            char data_inicio[32], data_fim[32];
            if (sscanf(buffer, "%31[^,],%31[^\n]", data_inicio, data_fim) == 2) {
                char *resposta_json = buscar_intervalo_linear(data_inicio, data_fim);
                zmq_send(responder, resposta_json, strlen(resposta_json), 0);
                free(resposta_json);
            } else {
                const char *msg_erro = "{\"erro\":\"formato inválido, envie 'data_inicio,data_fim'\"}";
                zmq_send(responder, msg_erro, strlen(msg_erro), 0);
            }
        }

        zmq_close(responder);
        zmq_ctx_term(context);
        break;
        
    case 4:
        // Inicializa tabela cuckoo
        inicializarCuckoo(2053);
        char linha1[4096];
        // Pula o cabeçalho
        fgets(linha1, sizeof(linha1), f);

        while (fgets(linha1, sizeof(linha1), f)) {
            Registro1 r1;
            char *token = strtok(linha1, ",");
            if (!token) continue;

            strncpy(r1.data, token, sizeof(r1.data));
            r1.data[sizeof(r1.data) - 1] = '\0';

            // Avança tokens para preencher os campos conforme ordem e quantidade puladas no CSV
            for (int i = 0; i < 5; i++) token = strtok(NULL, ",");
            if (!token) continue;
            r1.demanda_residual = atof(token);

            token = strtok(NULL, ",");
            if (!token) continue;
            r1.demanda_contratada = atof(token);

            token = strtok(NULL, ",");
            if (!token) continue;
            r1.importacoes = atof(token);

            for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
            if (!token) continue;
            r1.geracao_termica = atof(token);

            for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
            if (!token) continue;
            r1.geracao_despachavel = atof(token);

            for (int i = 0; i < 6; i++) token = strtok(NULL, ",");
            if (!token) continue;
            r1.geracao_renovavel_total = atof(token);

            for (int i = 0; i < 7; i++) token = strtok(NULL, ",");
            if (!token) continue;
            r1.capacidade_instalada = atof(token);

            for (int i = 0; i < 3; i++) token = strtok(NULL, ",");
            if (!token) continue;
            r1.perdas_geracao_total = atof(token);

            for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
            if (!token) continue;
            r1.carga_reduzida_manual = atof(token);

            if (!inserirCuckoo(&r1)) {
                fprintf(stderr, "Falha ao inserir registro: %s\n", r1.data);
            }
        }

        fclose(f);
        printf("Dados inseridos na tabela Cuckoo Hashing.\n");
        exibirCuckoo();
        // Loop para responder consultas
        while (1) {
            char buffer[4096];
            int bytes = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
            if (bytes == -1) break;
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\r\n")] = 0;

            char data_inicio[32], data_fim[32];
            if (sscanf(buffer, "%31[^,],%31[^\n]", data_inicio, data_fim) == 2) {
                char *resposta_json;
                printf(data_inicio);
                printf(data_fim);
                buscar_intervalo_cuckoo(data_inicio, data_fim, &resposta_json);
                zmq_send(responder, resposta_json, strlen(resposta_json), 0);
                free(resposta_json);
            } else {
                const char *msg_erro = "{\"erro\":\"formato inválido, envie 'data_inicio,data_fim'\"}";
                zmq_send(responder, msg_erro, strlen(msg_erro), 0);
            }
        }

        zmq_close(responder);
        zmq_ctx_term(context);
        break;
    case 5:
        // Inicializa socket ZeroMQ
        while (1) {
            char buffer[4096];
            int bytes = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
            if (bytes == -1) break;
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\r\n")] = 0;

            char data_inicio[32], data_fim[32];
            if (sscanf(buffer, "%31[^,],%127[^\n]", data_inicio, data_fim) == 2) {
                char *resposta_json;
                printf(data_inicio);
                printf(data_fim);
                buscar_intervalo(raiz, data_inicio, data_fim, &resposta_json);
                zmq_send(responder, resposta_json, strlen(resposta_json), 0);
                free(resposta_json);
            } else {
                const char *msg_erro = "{\"erro\":\"formato inválido, envie 'data_inicio,data_fim'\"}";
                zmq_send(responder, msg_erro, strlen(msg_erro), 0);
            }
        }

        zmq_close(responder);
        zmq_ctx_term(context);
        break;
    case 6:



        // Inicializa socket ZeroMQ
        while (1) {
            char buffer[4096];
            int bytes = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
            if (bytes == -1) break;
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\r\n")] = 0;

            char data_inicio[32], data_fim[32];
            if (sscanf(buffer, "%31[^,],%127[^\n]", data_inicio, data_fim) == 2) {
                char *resposta_json;
                printf(data_inicio);
                printf(data_fim);
                buscar_intervalo(raiz, data_inicio, data_fim, &resposta_json);
                zmq_send(responder, resposta_json, strlen(resposta_json), 0);
                free(resposta_json);
            } else {
                const char *msg_erro = "{\"erro\":\"formato inválido, envie 'data_inicio,data_fim'\"}";
                zmq_send(responder, msg_erro, strlen(msg_erro), 0);
            }
        }

        zmq_close(responder);
        zmq_ctx_term(context);
        break;
    default:
        break;
    }
    return 0;
}