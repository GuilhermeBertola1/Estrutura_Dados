#ifndef AVL_H
#define AVL_H

#define MAX_NODES 10000

typedef struct {
    char data[20];
    double demanda_residual;
    double demanda_contratada;
    double geracao_despachavel;
    double geracao_termica;
    double importacoes;
    double geracao_renovavel_total;
    double carga_reduzida_manual;
    double capacidade_instalada;
    double perdas_geracao_total;
} Registro;

typedef struct ListaRegistro {
    Registro info;
    struct ListaRegistro *prox;
} ListaRegistro;

typedef struct Node {
    char data[20];
    ListaRegistro *registros;
    int altura;
    struct Node *esq, *dir;
} Node;

extern Node* vetor_nos[MAX_NODES];
extern int contador;

Node* inserir(Node *raiz, Registro r);
void em_ordem(Node *raiz);

void preencher_vetor_nos(Node *raiz);
void registro_to_json_completo(Node *no, char *buffer, size_t size);

int safe_strcat(char *dest, size_t dest_size, const char *src);
void buscar_intervalo(Node *raiz, const char *data_inicio, const char *data_fim, char *buffer, size_t size);

#endif