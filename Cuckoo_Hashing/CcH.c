#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CcH.h"

CuckooEntry *tabela1 = NULL;
CuckooEntry *tabela2 = NULL;
size_t tamanho_atual = 0;
size_t quantidade_itens = 0;

// Funções de hash (modificadas para usar tamanho_atual)
unsigned int hash1(const char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % tamanho_atual;
}

unsigned int hash2(const char *str) {
    unsigned int hash = 0;
    int c;
    while ((c = *str++))
        hash = hash * 131 + c;
    return hash % tamanho_atual;
}

void inicializarCuckoo(size_t tamanho_inicial) {
    tamanho_atual = tamanho_inicial;
    quantidade_itens = 0;
    tabela1 = calloc(tamanho_atual, sizeof(CuckooEntry));
    tabela2 = calloc(tamanho_atual, sizeof(CuckooEntry));
    if (!tabela1 || !tabela2) {
        perror("calloc");
        exit(1);
    }
}

void liberarCuckoo(void) {
    free(tabela1);
    free(tabela2);
    tabela1 = NULL;
    tabela2 = NULL;
    tamanho_atual = 0;
    quantidade_itens = 0;
}

// Verifica se a chave já existe na tabela para evitar duplicados
int existe_na_tabela(const char *chave) {
    unsigned int pos1 = hash1(chave);
    if (tabela1[pos1].ocupado && strcmp(tabela1[pos1].chave, chave) == 0)
        return 1;
    unsigned int pos2 = hash2(chave);
    if (tabela2[pos2].ocupado && strcmp(tabela2[pos2].chave, chave) == 0)
        return 1;
    return 0;
}

// Função interna para inserir sem chamar rehash (usada no rehash)
static int inserir_sem_rehash(const Registro1 *r) {
    char chave[MAX_STR];
    strncpy(chave, r->data, MAX_STR);
    chave[MAX_STR - 1] = '\0';

    Registro1 temp = *r;
    char tempChave[MAX_STR];
    strncpy(tempChave, chave, MAX_STR);

    unsigned int pos;
    int tabela = 1;
    int max_tentativas = 100;

    for (int i = 0; i < max_tentativas; i++) {
        if (tabela == 1) {
            pos = hash1(tempChave);
            if (!tabela1[pos].ocupado) {
                tabela1[pos].ocupado = 1;
                tabela1[pos].valor = temp;
                strncpy(tabela1[pos].chave, tempChave, MAX_STR);
                return 1;
            }
            Registro1 aux = tabela1[pos].valor;
            char auxChave[MAX_STR];
            strncpy(auxChave, tabela1[pos].chave, MAX_STR);

            tabela1[pos].valor = temp;
            strncpy(tabela1[pos].chave, tempChave, MAX_STR);

            temp = aux;
            strncpy(tempChave, auxChave, MAX_STR);

            tabela = 2;
        } else {
            pos = hash2(tempChave);
            if (!tabela2[pos].ocupado) {
                tabela2[pos].ocupado = 1;
                tabela2[pos].valor = temp;
                strncpy(tabela2[pos].chave, tempChave, MAX_STR);
                return 1;
            }
            Registro1 aux = tabela2[pos].valor;
            char auxChave[MAX_STR];
            strncpy(auxChave, tabela2[pos].chave, MAX_STR);

            tabela2[pos].valor = temp;
            strncpy(tabela2[pos].chave, tempChave, MAX_STR);

            temp = aux;
            strncpy(tempChave, auxChave, MAX_STR);

            tabela = 1;
        }
    }
    fprintf(stderr, "Falha na inserção do registro %s após %d tentativas\n", r->data, max_tentativas);
    return 0; // falhou
}

void rehash(size_t novo_tamanho) {
    CuckooEntry *antiga1 = tabela1;
    CuckooEntry *antiga2 = tabela2;
    size_t antiga_tam = tamanho_atual;

    tabela1 = calloc(novo_tamanho, sizeof(CuckooEntry));
    tabela2 = calloc(novo_tamanho, sizeof(CuckooEntry));
    if (!tabela1 || !tabela2) {
        perror("calloc");
        exit(1);
    }

    tamanho_atual = novo_tamanho;
    quantidade_itens = 0;

    for (size_t i = 0; i < antiga_tam; i++) {
        if (antiga1[i].ocupado) {
            if (!inserir_sem_rehash(&antiga1[i].valor)) {
                fprintf(stderr, "Erro no rehash (tabela1)\n");
                exit(1);
            }
            quantidade_itens++; // atualiza contador ao re-inserir
        }
        if (antiga2[i].ocupado) {
            if (!inserir_sem_rehash(&antiga2[i].valor)) {
                fprintf(stderr, "Erro no rehash (tabela2)\n");
                exit(1);
            }
            quantidade_itens++; // atualiza contador ao re-inserir
        }
    }

    free(antiga1);
    free(antiga2);
}

// Função principal de inserção, que chama rehash se necessário
int inserirCuckoo(const Registro1 *r) {
    //printf("Inserindo: [%s]\n", r->data);
    if (existe_na_tabela(r->data)) {
        return 1;
    }

    if (quantidade_itens >= tamanho_atual * 0.4) {
        rehash(tamanho_atual * 2);
    }

    int sucesso = inserir_sem_rehash(r);
    if (!sucesso) {
        rehash(tamanho_atual * 2);
        sucesso = inserir_sem_rehash(r);
        if (!sucesso) {
            fprintf(stderr, "Falha ao inserir mesmo após rehash\n");
            return 0;
        }
    }
    quantidade_itens++;
    return 1;
}

// As outras funções continuam iguais:

long long datetime_para_inteiro_cuck(const char *datetime) {
    int ano, mes, dia, hora, min, seg;
    char ampm[3] = "";

    // Verifica se tem AM/PM no final
    if (strstr(datetime, "AM") || strstr(datetime, "PM") || strstr(datetime, "am") || strstr(datetime, "pm")) {
        sscanf(datetime, "%d-%d-%d %d:%d:%d %2s", &ano, &mes, &dia, &hora, &min, &seg, ampm);

        if ((strcmp(ampm, "PM") == 0 || strcmp(ampm, "pm") == 0) && hora != 12)
            hora += 12;
        else if ((strcmp(ampm, "AM") == 0 || strcmp(ampm, "am") == 0) && hora == 12)
            hora = 0;
    } else {
        sscanf(datetime, "%d-%d-%d %d:%d:%d", &ano, &mes, &dia, &hora, &min, &seg);
    }

    return (long long)ano * 10000000000LL +
           (long long)mes * 100000000 +
           (long long)dia * 1000000 +
           (long long)hora * 10000 +
           (long long)min * 100 +
           (long long)seg;
}

void buscar_intervalo_cuckoo(const char *data_inicio, const char *data_fim, char **saida) {
    long long inicio = datetime_para_inteiro_cuck(data_inicio);
    long long fim = datetime_para_inteiro_cuck(data_fim);
    int primeiro = 1;

    size_t capacidade = 8192;
    size_t usado = 0;
    char *tmp_buffer = malloc(capacidade);
    if (!tmp_buffer) {
        perror("malloc");
        exit(1);
    }

    usado = snprintf(tmp_buffer, capacidade, "[");

    for (size_t i = 0; i < tamanho_atual; i++) {
        CuckooEntry *registros[2] = { &tabela1[i], &tabela2[i] };
        for (int j = 0; j < 2; j++) {
            if (registros[j]->ocupado && strlen(registros[j]->valor.data) > 0) {
                long long data_reg = datetime_para_inteiro_cuck(registros[j]->valor.data);
                if (data_reg >= inicio && data_reg <= fim) {
                    char item[2048];
                    int n = snprintf(item, sizeof(item),
                        "%s{\n"
                        "  \"data\": \"%s\",\n"
                        "  \"demanda_residual\": %.2f,\n"
                        "  \"demanda_contratada\": %.2f,\n"
                        "  \"geracao_despachavel\": %.2f,\n"
                        "  \"geracao_termica\": %.2f,\n"
                        "  \"importacoes\": %.2f,\n"
                        "  \"geracao_renovavel_total\": %.2f,\n"
                        "  \"carga_reduzida_manual\": %.2f,\n"
                        "  \"capacidade_instalada\": %.2f,\n"
                        "  \"perdas_geracao_total\": %.2f\n"
                        "}",
                        (primeiro ? "" : ",\n"),
                        registros[j]->valor.data,
                        registros[j]->valor.demanda_residual,
                        registros[j]->valor.demanda_contratada,
                        registros[j]->valor.geracao_despachavel,
                        registros[j]->valor.geracao_termica,
                        registros[j]->valor.importacoes,
                        registros[j]->valor.geracao_renovavel_total,
                        registros[j]->valor.carga_reduzida_manual,
                        registros[j]->valor.capacidade_instalada,
                        registros[j]->valor.perdas_geracao_total
                    );

                    if (usado + n + 1 >= capacidade) {
                        capacidade *= 2;
                        tmp_buffer = realloc(tmp_buffer, capacidade);
                        if (!tmp_buffer) {
                            perror("realloc");
                            exit(1);
                        }
                    }

                    memcpy(tmp_buffer + usado, item, n);
                    usado += n;
                    tmp_buffer[usado] = '\0';
                    primeiro = 0;
                }
            }
        }
    }

    if (usado + 2 >= capacidade) {
        capacidade += 2;
        tmp_buffer = realloc(tmp_buffer, capacidade);
        if (!tmp_buffer) {
            perror("realloc");
            exit(1);
        }
    }

    tmp_buffer[usado++] = ']';
    tmp_buffer[usado] = '\0';

    *saida = tmp_buffer;
}



void exibirCuckoo() {
    int contador = 0;
    printf("Exibindo dados da Tabela Cuckoo:\n");
    printf("Tamanho atual: %zu | Quantidade de itens: %zu\n", tamanho_atual, quantidade_itens);
    printf("------------------------------------------------------------\n");

    for (size_t i = 0; i < tamanho_atual; i++) {
        if (tabela1[i].ocupado) {
            printf("[Tabela 1][%zu]: Chave: %s\n", i, tabela1[i].chave);
            printf("  Data: %s\n", tabela1[i].valor.data);
            printf("  Demanda Residual: %.2f\n", tabela1[i].valor.demanda_residual);
            printf("  Demanda Contratada: %.2f\n", tabela1[i].valor.demanda_contratada);
            printf("  Geração Despachável: %.2f\n", tabela1[i].valor.geracao_despachavel);
            printf("  Geração Térmica: %.2f\n", tabela1[i].valor.geracao_termica);
            printf("  Importações: %.2f\n", tabela1[i].valor.importacoes);
            printf("  Geração Renovável Total: %.2f\n", tabela1[i].valor.geracao_renovavel_total);
            printf("  Carga Reduzida Manual: %.2f\n", tabela1[i].valor.carga_reduzida_manual);
            printf("  Capacidade Instalada: %.2f\n", tabela1[i].valor.capacidade_instalada);
            printf("  Perdas Geração Total: %.2f\n", tabela1[i].valor.perdas_geracao_total);
            printf("------------------------------------------------------------\n");
            contador++;
        }
    }

    for (size_t i = 0; i < tamanho_atual; i++) {
        if (tabela2[i].ocupado) {
            printf("[Tabela 2][%zu]: Chave: %s\n", i, tabela2[i].chave);
            printf("  Data: %s\n", tabela2[i].valor.data);
            printf("  Demanda Residual: %.2f\n", tabela2[i].valor.demanda_residual);
            printf("  Demanda Contratada: %.2f\n", tabela2[i].valor.demanda_contratada);
            printf("  Geração Despachável: %.2f\n", tabela2[i].valor.geracao_despachavel);
            printf("  Geração Térmica: %.2f\n", tabela2[i].valor.geracao_termica);
            printf("  Importações: %.2f\n", tabela2[i].valor.importacoes);
            printf("  Geração Renovável Total: %.2f\n", tabela2[i].valor.geracao_renovavel_total);
            printf("  Carga Reduzida Manual: %.2f\n", tabela2[i].valor.carga_reduzida_manual);
            printf("  Capacidade Instalada: %.2f\n", tabela2[i].valor.capacidade_instalada);
            printf("  Perdas Geração Total: %.2f\n", tabela2[i].valor.perdas_geracao_total);
            printf("------------------------------------------------------------\n");
            contador++;
        }
    }

    printf("Numero de itens: %d", contador);
}
