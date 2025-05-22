#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TABLE_SIZE 17

typedef struct {
    char* chave;  // pode ser a data ou um identificador
    double demanda_residual;
    double demanda_contratada;
    double geracao_despachavel;
    double geracao_termica;
    double importacoes;
    double geracao_renovavel_total;
    double carga_reduzida_manual;
    double capacidade_instalada;
    double perdas_geracao_total;
    time_t timestamp;  // para busca por intervalo
    int ocupado;
} Entrada;

Entrada hashLinear[TABLE_SIZE];

// Função de hash simples
int hashSimples(const char* chave) {
    int soma = 0;
    for (int i = 0; chave[i]; i++) soma += chave[i];
    return soma % TABLE_SIZE;
}

// Conversão de string para time_t
time_t string_para_time(const char* data_str) {
    struct tm tm_data = {0};
    strptime(data_str, "%Y-%m-%d %I:%M:%S%p", &tm_data);
    return mktime(&tm_data);
}

// Inicialização da hash
void inicializarHash() {
    for (int i = 0; i < TABLE_SIZE; i++) hashLinear[i].ocupado = 0;
}

// Inserção na hash
int inserirHash(
    const char* chave, double demanda_residual, double demanda_contratada,
    double geracao_despachavel, double geracao_termica, double importacoes,
    double geracao_renovavel_total, double carga_reduzida_manual,
    double capacidade_instalada, double perdas_geracao_total,
    const char* data_str
) {
    int pos = hashSimples(chave);
    time_t t = string_para_time(data_str);
    for (int i = 0; i < TABLE_SIZE; i++) {
        int idx = (pos + i) % TABLE_SIZE;
        if (!hashLinear[idx].ocupado) {
            hashLinear[idx].chave = strdup(chave);
            hashLinear[idx].demanda_residual = demanda_residual;
            hashLinear[idx].demanda_contratada = demanda_contratada;
            hashLinear[idx].geracao_despachavel = geracao_despachavel;
            hashLinear[idx].geracao_termica = geracao_termica;
            hashLinear[idx].importacoes = importacoes;
            hashLinear[idx].geracao_renovavel_total = geracao_renovavel_total;
            hashLinear[idx].carga_reduzida_manual = carga_reduzida_manual;
            hashLinear[idx].capacidade_instalada = capacidade_instalada;
            hashLinear[idx].perdas_geracao_total = perdas_geracao_total;
            hashLinear[idx].timestamp = t;
            hashLinear[idx].ocupado = 1;
            return 1;
        }
    }
    return 0; // tabela cheia
}

// Remoção da hash
int removerHash(const char* chave) {
    int pos = hashSimples(chave);
    for (int i = 0; i < TABLE_SIZE; i++) {
        int idx = (pos + i) % TABLE_SIZE;
        if (hashLinear[idx].ocupado && strcmp(hashLinear[idx].chave, chave) == 0) {
            free(hashLinear[idx].chave);
            hashLinear[idx].ocupado = 0;
            return 1;
        }
    }
    return 0;
}

// Serializa uma entrada para JSON
void entrada_para_json(const Entrada* e, char* buffer, size_t size, int is_first) {
    snprintf(buffer, size,
        "%s{\n"
        "  \"chave\": \"%s\",\n"
        "  \"demanda_residual\": %.2f,\n"
        "  \"demanda_contratada\": %.2f,\n"
        "  \"geracao_despachavel\": %.2f,\n"
        "  \"geracao_termica\": %.2f,\n"
        "  \"importacoes\": %.2f,\n"
        "  \"geracao_renovavel_total\": %.2f,\n"
        "  \"carga_reduzida_manual\": %.2f,\n"
        "  \"capacidade_instalada\": %.2f,\n"
        "  \"perdas_geracao_total\": %.2f,\n"
        "  \"timestamp\": \"%s\"\n"
        "}",
        is_first ? "" : ",\n",
        e->chave,
        e->demanda_residual,
        e->demanda_contratada,
        e->geracao_despachavel,
        e->geracao_termica,
        e->importacoes,
        e->geracao_renovavel_total,
        e->carga_reduzida_manual,
        e->capacidade_instalada,
        e->perdas_geracao_total,
        asctime(localtime(&e->timestamp))  // inclui \n no final!
    );
}

// Busca intervalo e gera JSON
void buscar_intervalo_hash(const char* inicio, const char* fim, char **saida) {
    time_t t_inicio = string_para_time(inicio);
    time_t t_fim = string_para_time(fim);

    size_t capacidade = 1024;
    size_t usado = 0;
    char* buffer = malloc(capacidade);
    if (!buffer) {
        perror("malloc");
        exit(1);
    }

    usado += snprintf(buffer + usado, capacidade - usado, "[\n");

    int primeiro = 1;

    for (int i = 0; i < TABLE_SIZE; i++) {
        if (hashLinear[i].ocupado) {
            time_t t = hashLinear[i].timestamp;
            if (t >= t_inicio && t <= t_fim) {
                char item[1024];
                entrada_para_json(&hashLinear[i], item, sizeof(item), primeiro);

                size_t n = strlen(item);
                if (usado + n + 2 > capacidade) {
                    capacidade *= 2;
                    buffer = realloc(buffer, capacidade);
                    if (!buffer) {
                        perror("realloc");
                        exit(1);
                    }
                }
                strcpy(buffer + usado, item);
                usado += n;
                primeiro = 0;
            }
        }
    }

    usado += snprintf(buffer + usado, capacidade - usado, "\n]\n");
    *saida = buffer;  // retornando o JSON dinâmico
}

// Estatísticas da hash
void estatisticasHash() {
    int count = 0;
    double soma = 0, min = 1e9, max = -1e9;
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (hashLinear[i].ocupado) {
            double v = hashLinear[i].demanda_residual;  // exemplo: usando demanda_residual
            soma += v;
            if (v < min) min = v;
            if (v > max) max = v;
            count++;
        }
    }
    if (count)
        printf("Média demanda_residual: %.2f | Mín: %.2f | Máx: %.2f\n", soma / count, min, max);
    else
        printf("Nenhum dado disponível.\n");
}





int main() {
    inicializarHash();

    inserirHash("2025-05-21 10:00:00PM", 100, 200, 300, 400, 500, 600, 700, 800, 900, "2025-05-21 10:00:00PM");
    inserirHash("2025-05-22 11:00:00AM", 110, 210, 310, 410, 510, 610, 710, 810, 910, "2025-05-22 11:00:00AM");

    char* json_resultado;
    buscar_intervalo_hash("2025-05-21 09:00:00AM", "2025-05-22 12:00:00PM", &json_resultado);

    printf("%s\n", json_resultado);
    free(json_resultado);

    estatisticasHash();

    return 0;
}