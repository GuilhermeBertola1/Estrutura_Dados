#ifndef CUCKOO_HASH_H
#define CUCKOO_HASH_H

#include <stddef.h>  // para size_t

#define MAX_STR 2048

typedef struct {
    char data[MAX_STR];
    float demanda_residual;
    float demanda_contratada;
    float geracao_despachavel;
    float geracao_termica;
    float importacoes;
    float geracao_renovavel_total;
    float carga_reduzida_manual;
    float capacidade_instalada;
    float perdas_geracao_total;
} Registro1;

typedef struct {
    char chave[MAX_STR];
    Registro1 valor;
    int ocupado;
} CuckooEntry;

extern CuckooEntry *tabela1;
extern CuckooEntry *tabela2;
extern size_t tamanho_atual;
extern size_t quantidade_itens;

void inicializarCuckoo(size_t tamanho_inicial);
void liberarCuckoo(void);

int inserirCuckoo(const Registro1 *r);
long long datetime_para_inteiro_cuck(const char *datetime);
void buscar_intervalo_cuckoo(const char *data_inicio, const char *data_fim, char **saida);

// Declaração da nova função para conversão de 24h para 12h AM/PM
void exibirCuckoo();

#endif // CUCKOO_HASH_H