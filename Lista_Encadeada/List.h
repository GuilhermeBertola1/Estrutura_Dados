#ifndef LISTA_ENCADEADA_H
#define LISTA_ENCADEADA_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef struct EletricDates {
    char data[24]; 
    double demanda_residual; 
    double demanda_contratada;
    double geracao_despachavel;
    double geracao_termica;
    double importacoes;
    double geracao_renovavel_total;
    double carga_reduzida_manual;
    double capacidade_instalada;
    double perdas_geracao_total;
} EletricDates; 

typedef struct Estatisticas {
    double media_demanda_residual;
    double media_demanda_contratada;
    double media_geracao_despachavel;
    double media_geracao_termica;
    double media_importacoes;
    double media_geracao_renovavel_total;
    double media_carga_reduzida_manual;
    double media_capacidade_instalada;
    double media_perdas_geracao_total;

    double dp_demanda_residual;
    double dp_demanda_contratada;
    double dp_geracao_despachavel;
    double dp_geracao_termica;
    double dp_importacoes;
    double dp_geracao_renovavel_total;
    double dp_carga_reduzida_manual;
    double dp_capacidade_instalada;
    double dp_perdas_geracao_total;

    int n;  
} Estatisticas;

typedef struct NodeList {
    EletricDates dado;
    struct NodeList* next;
} NodeList;

extern NodeList *lista_ligada;

int in(NodeList** head, EletricDates *dado);

long long datetime_para_inteiro_list(const char *datetime);
void buscar_intervalo_list(const char *inicio_str, const char *fim_str, char **saida);
void imprimir_lista(NodeList* no);
void media(NodeList* head, const char* data_inicio, const char* data_fim, Estatisticas* est);
void desvio(NodeList* head, const char* data_inicio, const char* data_fim, Estatisticas* est);
void liberar_lista(NodeList** head);

#endif 