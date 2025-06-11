#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include "AVL/avl.h"
#include "Cuckoo_Hashing/CcH.h"
#include "Hash_table/Hash.h"
#include "Lista_Encadeada/List.h"
#include "LSM_tree/Lsm.h"
#include "BM_tree/Bm.h"
#include "Trie/Trie.h"
#include "Comp_list/CmpList.h"

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

    printf("%s========================================%s\n", cyan, reset);
    printf("%s Client rodando:http://localhost:8501/ %s\n", green, reset);
    printf("%s========================================%s\n", cyan, reset);

    printf("%s========================================%s\n", cyan, reset);
    printf("%s          Selecione a estrutura        %s\n", green, reset);
    printf("%s========================================%s\n", cyan, reset);

    printf("%s[1]%s AVL\n", yellow, reset);
    printf("%s[2]%s Lista Encadeada\n", yellow, reset);
    printf("%s[3]%s Tabela Hash\n", yellow, reset);
    printf("%s[4]%s Cuckoo Hashing\n", yellow, reset);
    printf("%s[5]%s Trie\n", yellow, reset);
    printf("%s[6]%s LSM Tree\n", yellow, reset);
    printf("%s[7]%s B+ Tree\n", yellow, reset);
    printf("%s[8]%s Lista encadeada compactada\n", yellow, reset);

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
            if (sscanf(buffer, "%31[^,],%31[^\n]", data_inicio, data_fim) == 2) {
                char *resposta_json;
                VetorRegistros vetor;
                vetor_inicializar(&vetor, 1024);  // inicializa vetor antes do uso
                printf(data_inicio);
                printf(data_fim);
                buscar_intervalo(raiz, data_inicio, data_fim, &resposta_json, &vetor);

                EstatisticasCampos est = calcular_estatisticas(&vetor);
                Medianas med = calcular_mediana(&vetor);
                Modas moda = calcular_moda(&vetor);

                char *json_estatisticas = estatisticas_para_json_conteudo(est, med, moda);

                size_t tamanho = strlen(resposta_json) + strlen(json_estatisticas) + 1024; // mais folga para chaves e aspas
                char *json_completo = malloc(tamanho);
                if (!json_completo) {
                    fprintf(stderr, "Erro ao alocar memória para resposta JSON\n");
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberar(&vetor);
                    continue;
                }

                int len = snprintf(json_completo, tamanho, "{\"dados\":%s,\"estatisticas\":%s}", resposta_json, json_estatisticas);
                if (len < 0 || (size_t)len >= tamanho) {
                    fprintf(stderr, "Erro ao construir JSON completo\n");
                    free(json_completo);
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberar(&vetor);
                    continue;
                }

                zmq_send(responder, json_completo, strlen(json_completo), 0);

                free(json_completo);
                free(resposta_json);
                free(json_estatisticas);
                vetor_liberar(&vetor);
            } else {
                const char *msg_erro = "{\"erro\":\"formato inválido, envie 'data_inicio,data_fim'\"}";
                zmq_send(responder, msg_erro, strlen(msg_erro), 0);
            }
        }

        zmq_close(responder);
        zmq_ctx_term(context);

        break;
    case 2:
        char linha3[4096];

        fgets(linha3, sizeof(linha3), f); // pula cabeçalho

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
        }

        fclose(f);
        imprimir_lista(lista_ligada);
        
        // Inicializa socket ZeroMQ
        while (1) {
            char buffer[4096];
            int bytes = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
            if (bytes == -1) break;
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\r\n")] = 0;

            char data_inicio[32], data_fim[32];
            if (sscanf(buffer, "%31[^,],%31[^\n]", data_inicio, data_fim) == 2) {
                char *resposta_json;
                VetorEletricDates vetor;
                vetor_inicializarList(&vetor, 1024);
                printf(data_inicio);
                printf(data_fim);
                buscar_intervalo_list(data_inicio, data_fim, &resposta_json, &vetor);

                EstatisticasCamposList est = calcular_estatisticasList(&vetor);
                MedianasList med = calcular_mediana_list(&vetor);
                ModasList moda = calcular_moda_list(&vetor);

                char *json_estatisticas = estatisticas_para_json_conteudo_list(est, med, moda);

                size_t tamanho = strlen(resposta_json) + strlen(json_estatisticas) + 1024; // mais folga para chaves e aspas
                char *json_completo = malloc(tamanho);
                if (!json_completo) {
                    fprintf(stderr, "Erro ao alocar memória para resposta JSON\n");
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberarList(&vetor);
                    continue;
                }

                int len = snprintf(json_completo, tamanho, "{\"dados\":%s,\"estatisticas\":%s}", resposta_json, json_estatisticas);
                if (len < 0 || (size_t)len >= tamanho) {
                    fprintf(stderr, "Erro ao construir JSON completo\n");
                    free(json_completo);
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberarList(&vetor);
                    continue;
                }

                zmq_send(responder, json_completo, strlen(json_completo), 0);

                free(json_completo);
                free(resposta_json);
                free(json_estatisticas);
                vetor_liberarList(&vetor);
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
        printf("Dados inseridos na tabela Hashing.\n");

        // Inicializa socket ZeroMQ
        while (1) {
            char buffer[4096];
            int bytes = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
            if (bytes == -1) break;
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\r\n")] = 0;

            char data_inicio[32], data_fim[32];
            if (sscanf(buffer, "%31[^,],%31[^\n]", data_inicio, data_fim) == 2) {
                char *resposta_json;
                VetorEntrada vetor;
                vetor_inicializar_HT(&vetor, 1024);
                printf(data_inicio);
                printf(data_fim);
                buscar_intervalo_HT(data_inicio, data_fim, &resposta_json, &vetor);
                EstatisticasCamposHT est = calcular_estatisticas_HT(&vetor);
                MedianasHT med = calcular_mediana_HT(&vetor);
                ModasHT moda = calcular_moda_HT(&vetor);
                
                char *json_estatisticas = estatisticas_para_json_conteudo_HT(est, med, moda);
                
                size_t tamanho = strlen(resposta_json) + strlen(json_estatisticas) + 1024; // mais folga para chaves e aspas
                char *json_completo = malloc(tamanho);
                if (!json_completo) {
                    fprintf(stderr, "Erro ao alocar memória para resposta JSON\n");
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberar_HT(&vetor);
                    continue;
                }


                int len = snprintf(json_completo, tamanho, "{\"dados\":%s,\"estatisticas\":%s}", resposta_json, json_estatisticas);
                if (len < 0 || (size_t)len >= tamanho) {

                 fprintf(stderr, "Erro ao construir JSON completo\n");
                    free(json_completo);
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberar_HT(&vetor);
                    continue;
                }

                zmq_send(responder, json_completo, strlen(json_completo), 0);


                free(json_completo);
                free(resposta_json);
                free(json_estatisticas);
                vetor_liberar_HT(&vetor);
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
                VetorRegistro1 vetor;
                vetor_inicializar_cc(&vetor, 1024);  // inicializa vetor antes do uso
                printf(data_inicio);
                printf(data_fim);
                buscar_intervalo_cuckoo(data_inicio, data_fim, &resposta_json, &vetor);

                EstatisticasCamposCC est = calcular_estatisticas_cc(&vetor);
                MedianasCC med = calcular_mediana_cc(&vetor);
                ModasCC moda = calcular_moda_cc(&vetor);

                char *json_estatisticas = estatisticas_para_json_conteudo_cc(est, med, moda);

                size_t tamanho = strlen(resposta_json) + strlen(json_estatisticas) + 1024; // mais folga para chaves e aspas
                char *json_completo = malloc(tamanho);
                if (!json_completo) {
                    fprintf(stderr, "Erro ao alocar memória para resposta JSON\n");
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberar_cc(&vetor);
                    continue;
                }

                int len = snprintf(json_completo, tamanho, "{\"dados\":%s,\"estatisticas\":%s}", resposta_json, json_estatisticas);
                if (len < 0 || (size_t)len >= tamanho) {
                    fprintf(stderr, "Erro ao construir JSON completo\n");
                    free(json_completo);
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberar_cc(&vetor);
                    continue;
                }

                zmq_send(responder, json_completo, strlen(json_completo), 0);

                free(json_completo);
                free(resposta_json);
                free(json_estatisticas);
                vetor_liberar_cc(&vetor);
            } else {
                const char *msg_erro = "{\"erro\":\"formato inválido, envie 'data_inicio,data_fim'\"}";
                zmq_send(responder, msg_erro, strlen(msg_erro), 0);
            }
        }

        zmq_close(responder);
        zmq_ctx_term(context);

        break;
    case 5:
        Trie* trie = criar_trie();

        char linha20[4096];
        fgets(linha20, sizeof(linha20), f);

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
        }
        printf("\n==== Dados na Trie Temporal ====\n");
        imprimir_trie(trie);
        
        while (1) {
            char buffer[4096];
            int bytes = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
            if (bytes == -1) break;
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\r\n")] = 0;

            char data_inicio[32], data_fim[32];
            printf(data_inicio);
            printf(data_fim);
            if (sscanf(buffer, "%31[^,],%127[^\n]", data_inicio, data_fim) == 2) {
                char *resposta_json;
                VetorEntrada2 vetor;
                vetor_inicializar_Trie(&vetor, 1024);
                printf(data_inicio);
                printf(data_fim);
                buscar_intervalo_trie(trie,data_inicio, data_fim, &resposta_json, &vetor);

                EstatisticasCamposTrie est = calcular_estatisticas_Trie(&vetor);
                MedianasTrie med = calcular_mediana_Trie(&vetor);
                ModasTrie moda = calcular_moda_Trie(&vetor);

                char *json_estatisticas = estatisticas_para_json_conteudo_Trie(est, med, moda);

                size_t tamanho = strlen(resposta_json) + strlen(json_estatisticas) + 1024; // mais folga para chaves e aspas
                char *json_completo = malloc(tamanho);
                if (!json_completo) {
                    fprintf(stderr, "Erro ao alocar memória para resposta JSON\n");
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberar_Trie(&vetor);
                    continue;
                }

                int len = snprintf(json_completo, tamanho, "{\"dados\":%s,\"estatisticas\":%s}", resposta_json, json_estatisticas);
                if (len < 0 || (size_t)len >= tamanho) {
                    fprintf(stderr, "Erro ao construir JSON completo\n");
                    free(json_completo);
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberar_Trie(&vetor);
                    continue;
                }


                zmq_send(responder, json_completo, strlen(json_completo), 0);

                free(json_completo);
                free(resposta_json);
                free(json_estatisticas);
                vetor_liberar_Trie(&vetor);
            } else {
                const char *msg_erro = "{\"erro\":\"formato inválido, envie 'data_inicio,data_fim'\"}";
                zmq_send(responder, msg_erro, strlen(msg_erro), 0);
            }
        }

        zmq_close(responder);
        zmq_ctx_term(context);

        break;
    case 6:
        char linha4[4096];    
        fgets(linha4, sizeof(linha4), f);

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
        }

        fclose(f);
        printf("Dados inseridos na LSM Tree.\n");

        printar_dados_todos_arquivos("LSM_tree");

        while (1) {
            char buffer[4096];
            int bytes = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
            if (bytes == -1) break;
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\r\n")] = 0;

            char data_inicio[32], data_fim[32];
            printf(data_inicio);
            printf(data_fim);
            if (sscanf(buffer, "%31[^,],%127[^\n]", data_inicio, data_fim) == 2) {
                char *resposta_json;
                VetorDados vetor;
                vetor_inicializar_lsm(&vetor, 1024);
                printf(data_inicio);
                printf(data_fim);
                buscar_intervalo_lsm(data_inicio, data_fim, &resposta_json, &vetor);

                EstatisticasCamposlsm est = calcular_estatisticas_lsm(&vetor);
                Medianaslsm med = calcular_mediana_lsm(&vetor);
                Modaslsm moda = calcular_moda_lsm(&vetor);

                char *json_estatisticas = estatisticas_para_json_conteudo_lsm(est, med, moda);

                size_t tamanho = strlen(resposta_json) + strlen(json_estatisticas) + 1024; // mais folga para chaves e aspas
                char *json_completo = malloc(tamanho);
                if (!json_completo) {
                    fprintf(stderr, "Erro ao alocar memória para resposta JSON\n");
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberar_lsm(&vetor);
                    continue;
                }

                int len = snprintf(json_completo, tamanho, "{\"dados\":%s,\"estatisticas\":%s}", resposta_json, json_estatisticas);
                if (len < 0 || (size_t)len >= tamanho) {
                    fprintf(stderr, "Erro ao construir JSON completo\n");
                    free(json_completo);
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberar_lsm(&vetor);
                    continue;
                }


                zmq_send(responder, json_completo, strlen(json_completo), 0);

                free(json_completo);
                free(resposta_json);
                free(json_estatisticas);
                vetor_liberar_lsm(&vetor);
            } else {
                const char *msg_erro = "{\"erro\":\"formato inválido, envie 'data_inicio,data_fim'\"}";
                zmq_send(responder, msg_erro, strlen(msg_erro), 0);
            }
        }

        zmq_close(responder);
        zmq_ctx_term(context);

        break;

    case 7:
        char linha10[4096];
        BPlusNode *raizBM = NULL; 

        fgets(linha10, sizeof(linha10), f);

        while (fgets(linha10, sizeof(linha10), f)) {
            Bdados b1;
            char *token = strtok(linha10, ",");
            if (!token) continue;

            strncpy(b1.data, token, sizeof(b1.data));
            b1.data[sizeof(b1.data) - 1] = '\0';

            // Avança tokens para preencher os campos conforme ordem e quantidade puladas no CSV
            for (int i = 0; i < 5; i++) token = strtok(NULL, ",");
            if (!token) continue;
            b1.demanda_residual = atof(token);

            token = strtok(NULL, ",");
            if (!token) continue;
            b1.demanda_contratada = atof(token);

            token = strtok(NULL, ",");
            if (!token) continue;
            b1.importacoes = atof(token);

            for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
            if (!token) continue;
            b1.geracao_termica = atof(token);

            for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
            if (!token) continue;
            b1.geracao_despachavel = atof(token);

            for (int i = 0; i < 6; i++) token = strtok(NULL, ",");
            if (!token) continue;
            b1.geracao_renovavel_total = atof(token);

            for (int i = 0; i < 7; i++) token = strtok(NULL, ",");
            if (!token) continue;
            b1.capacidade_instalada = atof(token);

            for (int i = 0; i < 3; i++) token = strtok(NULL, ",");
            if (!token) continue;
            b1.perdas_geracao_total = atof(token);

            for (int i = 0; i < 2; i++) token = strtok(NULL, ",");
            if (!token) continue;
            b1.carga_reduzida_manual = atof(token);

            inserir_bplus(&raizBM, b1);
        }

        imprimir_bplus(raizBM);

        while (1) {
            char buffer[4096];
            int bytes = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
            if (bytes == -1) break;
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\r\n")] = 0;

            char data_inicio[32], data_fim[32];
            printf(data_inicio);
            printf(data_fim);
            if (sscanf(buffer, "%31[^,],%31[^\n]", data_inicio, data_fim) == 2) {
                char *resposta_json;
                VetorBdados vetor;
                vetor_inicializar_BM(&vetor, 1024);  // inicializa vetor antes do uso
                printf(data_inicio);
                printf(data_fim);
                buscar_intervalo_bplus_json(raizBM, data_inicio, data_fim, &resposta_json, &vetor);

                EstatisticasCampos_BM est = calcular_estatisticas_BM(&vetor);
                Medianas_BM med = calcular_mediana_BM(&vetor);
                Modas_BM moda = calcular_moda_BM(&vetor);

                char *json_estatisticas = estatisticas_para_json_conteudo_BM(est, med, moda);

                size_t tamanho = strlen(resposta_json) + strlen(json_estatisticas) + 50; // mais folga para chaves e aspas
                char *json_completo = malloc(tamanho);
                if (!json_completo) {
                    fprintf(stderr, "Erro ao alocar memória para resposta JSON\n");
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberar_BM(&vetor);
                    continue;
                }

                int len = snprintf(json_completo, tamanho, "{\"dados\":%s,\"estatisticas\":%s}", resposta_json, json_estatisticas);
                if (len < 0 || (size_t)len >= tamanho) {
                    fprintf(stderr, "Erro ao construir JSON completo\n");
                    free(json_completo);
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberar_BM(&vetor);
                    continue;
                }

                zmq_send(responder, json_completo, strlen(json_completo), 0);

                free(json_completo);
                free(resposta_json);
                free(json_estatisticas);
                vetor_liberar_BM(&vetor);
            } else {
                const char *msg_erro = "{\"erro\":\"formato inválido, envie 'data_inicio,data_fim'\"}";
                zmq_send(responder, msg_erro, strlen(msg_erro), 0);
            }
        }

        zmq_close(responder);
        zmq_ctx_term(context);
        break;

    case 8:
        Bloco* CmpLISTA = NULL;
        CmpLISTA = criar_bloco();
        char linha90[4096];

        fgets(linha90, sizeof(linha90), f);

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
        }

        imprimir_lista_CMP(CmpLISTA);

        while (1) {
            char buffer[4096];
            int bytes = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
            if (bytes == -1) break;
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\r\n")] = 0;

            char data_inicio[32], data_fim[32];
            if (sscanf(buffer, "%31[^,],%31[^\n]", data_inicio, data_fim) == 2) {
                char *resposta_json;
                VetorCompList vetor;
                vetor_inicializarListC(&vetor, 1024);  // inicializa vetor antes do uso
                printf(data_inicio);
                printf(data_fim);
                buscar_intervalo_lista_CMP(CmpLISTA, data_inicio, data_fim, &resposta_json, &vetor);

                EstatisticasCamposListC est = calcular_estatisticasListC(&vetor);
                MedianasListC med = calcular_mediana_listc(&vetor);
                ModasListC moda = calcular_moda_listc(&vetor);

                char *json_estatisticas = estatisticas_para_json_conteudo_listc(est, med, moda);

                size_t tamanho = strlen(resposta_json) + strlen(json_estatisticas) + 1024; // mais folga para chaves e aspas
                char *json_completo = malloc(tamanho);
                if (!json_completo) {
                    fprintf(stderr, "Erro ao alocar memória para resposta JSON\n");
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberarListC(&vetor);
                    continue;
                }

                int len = snprintf(json_completo, tamanho, "{\"dados\":%s,\"estatisticas\":%s}", resposta_json, json_estatisticas);
                if (len < 0 || (size_t)len >= tamanho) {
                    fprintf(stderr, "Erro ao construir JSON completo\n");
                    free(json_completo);
                    free(resposta_json);
                    free(json_estatisticas);
                    vetor_liberarListC(&vetor);
                    continue;
                }

                zmq_send(responder, json_completo, strlen(json_completo), 0);

                free(json_completo);
                free(resposta_json);
                free(json_estatisticas);
                vetor_liberarListC(&vetor);
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