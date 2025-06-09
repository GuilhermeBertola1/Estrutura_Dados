#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "List.h"

NodeList *lista_ligada = NULL;

//inserção de dados como numa fial para não quebrar a estrutura temporal
int in(NodeList** head, EletricDates *dado) {
    NodeList* novo = malloc(sizeof(NodeList));
    if (!novo) {
        perror("malloc");
        return 0;
    }
    novo->dado = *dado;
    novo->next = NULL;

    //printf("Inserindo data: %s\n", novo->dado.data); // debug

    if (*head == NULL) {
        *head = novo;
        //printf("Inserido como primeiro nó.\n");
    } else {
        NodeList* atual = *head;
        while (atual->next != NULL) {
            atual = atual->next;
        }
        atual->next = novo;
        //printf("Inserido no final da lista.\n");
    }
    return 1;
}


long long datetime_para_inteiro_list(const char *datetime) {
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

//função de busca de dados por intervalo de tempo
void buscar_intervalo_list(const char *inicio_str, const char *fim_str, char **saida) {
    long long inicio = datetime_para_inteiro_list(inicio_str);
    long long fim = datetime_para_inteiro_list(fim_str);

    printf("Intervalo: %lld - %lld\n", inicio, fim);

    size_t capacidade = 8192;
    size_t usado = 0;
    int primeiro = 1;

    char *json = malloc(capacidade);
    if (!json) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    usado += snprintf(json + usado, capacidade - usado, "[");

    NodeList *atual = lista_ligada;
    int contagem = 0;
    while (atual != NULL) {
        long long data_valor = datetime_para_inteiro_list(atual->dado.data);
        //printf("Checando data %s (%lld)\n", atual->dado.data, data_valor);

        if (data_valor >= inicio && data_valor <= fim) {
            contagem++;
            char item[1024];
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
                primeiro ? "" : ",\n",
                atual->dado.data,
                atual->dado.demanda_residual,
                atual->dado.demanda_contratada,
                atual->dado.geracao_despachavel,
                atual->dado.geracao_termica,
                atual->dado.importacoes,
                atual->dado.geracao_renovavel_total,
                atual->dado.carga_reduzida_manual,
                atual->dado.capacidade_instalada,
                atual->dado.perdas_geracao_total
            );

            while (usado + n + 1 >= capacidade) {
                capacidade *= 2;
                char *temp = realloc(json, capacidade);
                if (!temp) {
                    free(json);
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
                json = temp;
            }

            memcpy(json + usado, item, n);
            usado += n;
            primeiro = 0;
        }
        atual = atual->next;
    }

    usado += snprintf(json + usado, capacidade - usado, "\n]");
    json[usado] = '\0';
    printf("Itens encontrados: %d\n", contagem);

    *saida = json;
}

//impressão da lista
void imprimir_lista(NodeList* no) {
    if (no == NULL) return;

    printf("Data: %s\n", no->dado.data);
    printf("  Demanda Residual: %.2f\n", no->dado.demanda_residual);
    printf("  Demanda Contratada: %.2f\n", no->dado.demanda_contratada);
    printf("  Geração Despachável: %.2f\n", no->dado.geracao_despachavel);
    printf("  Geração Térmica: %.2f\n", no->dado.geracao_termica);
    printf("  Importações: %.2f\n", no->dado.importacoes);
    printf("  Geração Renovável Total: %.2f\n", no->dado.geracao_renovavel_total);
    printf("  Carga Reduzida Manual: %.2f\n", no->dado.carga_reduzida_manual);
    printf("  Capacidade Instalada: %.2f\n", no->dado.capacidade_instalada);
    printf("  Perdas Geração Total: %.2f\n", no->dado.perdas_geracao_total);
    printf("---------------------------\n");

    imprimir_lista(no->next);
}

void media(NodeList* head, const char* data_inicio, const char* data_fim, Estatisticas* est) {
    long long inicio = datetime_para_inteiro_list(data_inicio);
    long long fim = datetime_para_inteiro_list(data_fim);

    memset(est, 0, sizeof(Estatisticas));

    NodeList* atual = head;
    while (atual != NULL) {
        long long data_valor = datetime_para_inteiro_list(atual->dado.data);
        if (data_valor >= inicio && data_valor <= fim) {
            est->media_demanda_residual += atual->dado.demanda_residual;
            est->media_demanda_contratada += atual->dado.demanda_contratada;
            est->media_geracao_despachavel += atual->dado.geracao_despachavel;
            est->media_geracao_termica += atual->dado.geracao_termica;
            est->media_importacoes += atual->dado.importacoes;
            est->media_geracao_renovavel_total += atual->dado.geracao_renovavel_total;
            est->media_carga_reduzida_manual += atual->dado.carga_reduzida_manual;
            est->media_capacidade_instalada += atual->dado.capacidade_instalada;
            est->media_perdas_geracao_total += atual->dado.perdas_geracao_total;

            est->n++;
        }
        atual = atual->next;
    }

    if (est->n > 0) {
        est->media_demanda_residual /= est->n;
        est->media_demanda_contratada /= est->n;
        est->media_geracao_despachavel /= est->n;
        est->media_geracao_termica /= est->n;
        est->media_importacoes /= est->n;
        est->media_geracao_renovavel_total /= est->n;
        est->media_carga_reduzida_manual /= est->n;
        est->media_capacidade_instalada /= est->n;
        est->media_perdas_geracao_total /= est->n;
    }
}

void desvio(NodeList* head, const char* data_inicio, const char* data_fim, Estatisticas* est) {
    long long inicio = datetime_para_inteiro_list(data_inicio);
    long long fim = datetime_para_inteiro_list(data_fim);

    if (est->n == 0) return; 

    double ssd_demanda_residual = 0;
    double ssd_demanda_contratada = 0;
    double ssd_geracao_despachavel = 0;
    double ssd_geracao_termica = 0;
    double ssd_importacoes = 0;
    double ssd_geracao_renovavel_total = 0;
    double ssd_carga_reduzida_manual = 0;
    double ssd_capacidade_instalada = 0;
    double ssd_perdas_geracao_total = 0;

    NodeList* atual = head;
    while (atual != NULL) {
        long long data_valor = datetime_para_inteiro_list(atual->dado.data);
        if (data_valor >= inicio && data_valor <= fim) {
            ssd_demanda_residual += pow(atual->dado.demanda_residual - est->media_demanda_residual, 2);
            ssd_demanda_contratada += pow(atual->dado.demanda_contratada - est->media_demanda_contratada, 2);
            ssd_geracao_despachavel += pow(atual->dado.geracao_despachavel - est->media_geracao_despachavel, 2);
            ssd_geracao_termica += pow(atual->dado.geracao_termica - est->media_geracao_termica, 2);
            ssd_importacoes += pow(atual->dado.importacoes - est->media_importacoes, 2);
            ssd_geracao_renovavel_total += pow(atual->dado.geracao_renovavel_total - est->media_geracao_renovavel_total, 2);
            ssd_carga_reduzida_manual += pow(atual->dado.carga_reduzida_manual - est->media_carga_reduzida_manual, 2);
            ssd_capacidade_instalada += pow(atual->dado.capacidade_instalada - est->media_capacidade_instalada, 2);
            ssd_perdas_geracao_total += pow(atual->dado.perdas_geracao_total - est->media_perdas_geracao_total, 2);
        }
        atual = atual->next;
    }

    est->dp_demanda_residual = sqrt(ssd_demanda_residual / est->n);
    est->dp_demanda_contratada = sqrt(ssd_demanda_contratada / est->n);
    est->dp_geracao_despachavel = sqrt(ssd_geracao_despachavel / est->n);
    est->dp_geracao_termica = sqrt(ssd_geracao_termica / est->n);
    est->dp_importacoes = sqrt(ssd_importacoes / est->n);
    est->dp_geracao_renovavel_total = sqrt(ssd_geracao_renovavel_total / est->n);
    est->dp_carga_reduzida_manual = sqrt(ssd_carga_reduzida_manual / est->n);
    est->dp_capacidade_instalada = sqrt(ssd_capacidade_instalada / est->n);
    est->dp_perdas_geracao_total = sqrt(ssd_perdas_geracao_total / est->n);
    est->dp_capacidade_instalada = sqrt(ssd_capacidade_instalada / est->n);
}

void liberar_lista(NodeList** head) {
    NodeList* atual = *head;
    while (atual != NULL) {
        NodeList* proximo = atual->next;
        free(atual);
        atual = proximo;
    }
    *head = NULL;
}