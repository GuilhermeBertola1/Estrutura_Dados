#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Bm.h"

// --- Funções para lista dinâmica ---

void lista_inicializar(ListaDados *lista) {
    lista->itens = NULL;
    lista->tamanho = 0;
    lista->capacidade = 0;
}

void lista_adicionar(ListaDados *lista, Bdados valor) {
    if (lista->tamanho == lista->capacidade) {
        int nova_capacidade = (lista->capacidade == 0) ? 4 : lista->capacidade * 2;
        Bdados *temp = realloc(lista->itens, nova_capacidade * sizeof(Bdados));
        if (!temp) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        lista->itens = temp;
        lista->capacidade = nova_capacidade;
    }
    lista->itens[lista->tamanho++] = valor;
}

void lista_liberar(ListaDados *lista) {
    free(lista->itens);
    lista->itens = NULL;
    lista->tamanho = 0;
    lista->capacidade = 0;
}

// --- B+ Tree ---

BPlusNode *criar_no(bool folha) {
    BPlusNode *no = malloc(sizeof(BPlusNode));
    if (!no) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    no->folha = folha;
    no->n = 0;
    if (folha) {
        for (int i = 0; i < M; i++) {
            lista_inicializar(&no->dados[i]);
        }
        no->prox = NULL;
    } else {
        for (int i = 0; i <= M; i++)
            no->filhos[i] = NULL;
    }
    return no;
}

// Função para comparar chaves, wrapper para strcmp
int comparar_chaves(const char *a, const char *b) {
    return strcmp(a, b);
}

// Insere valor em nó folha, se chave existe adiciona à lista, senão insere nova chave
void inserir_em_no(BPlusNode *no, const char *chave, Bdados valor) {
    int i = 0;
    // Procura se chave já existe
    while (i < no->n && comparar_chaves(no->chaves[i], chave) < 0)
        i++;

    if (i < no->n && comparar_chaves(no->chaves[i], chave) == 0) {
        // chave já existe, adiciona à lista
        lista_adicionar(&no->dados[i], valor);
    } else {
        // nova chave, desloca para inserir
        for (int j = no->n; j > i; j--) {
            strcpy(no->chaves[j], no->chaves[j - 1]);
            // move listas de dados
            no->dados[j] = no->dados[j - 1];
        }
        strcpy(no->chaves[i], chave);
        lista_inicializar(&no->dados[i]);
        lista_adicionar(&no->dados[i], valor);
        no->n++;
    }
}

void dividir_no(BPlusNode *pai, int i, BPlusNode *cheio) {
    BPlusNode *novo = criar_no(cheio->folha);

    int meio = M / 2;

    novo->n = cheio->n - meio - 1;

    if (cheio->folha) {
        for (int j = 0; j < novo->n; j++) {
            strcpy(novo->chaves[j], cheio->chaves[meio + 1 + j]);
            // mover listas
            novo->dados[j] = cheio->dados[meio + 1 + j];
        }
        novo->prox = cheio->prox;
        cheio->prox = novo;

        cheio->n = meio + 1;

        // mover filhos do pai
        for (int j = pai->n; j >= i + 1; j--)
            pai->filhos[j + 1] = pai->filhos[j];
        pai->filhos[i + 1] = novo;

        // mover chaves do pai
        for (int j = pai->n - 1; j >= i; j--)
            strcpy(pai->chaves[j + 1], pai->chaves[j]);

        strcpy(pai->chaves[i], cheio->chaves[meio + 1]);
        pai->n++;
    } else {
        for (int j = 0; j < novo->n; j++) {
            strcpy(novo->chaves[j], cheio->chaves[meio + 1 + j]);
        }

        for (int j = 0; j <= novo->n; j++) {
            novo->filhos[j] = cheio->filhos[meio + 1 + j];
        }

        cheio->n = meio;

        for (int j = pai->n; j >= i + 1; j--)
            pai->filhos[j + 1] = pai->filhos[j];
        pai->filhos[i + 1] = novo;

        for (int j = pai->n - 1; j >= i; j--)
            strcpy(pai->chaves[j + 1], pai->chaves[j]);

        strcpy(pai->chaves[i], cheio->chaves[meio]);
        pai->n++;
    }
}

void inserir_bplus(BPlusNode **raiz, Bdados valor) {
    const char *chave = valor.data;

    if (*raiz == NULL) {
        *raiz = criar_no(true);
    }

    BPlusNode *r = *raiz;

    if (r->n == M) {
        BPlusNode *s = criar_no(false);
        s->filhos[0] = r;
        dividir_no(s, 0, r);
        *raiz = s;
    }

    BPlusNode *atual = *raiz;

    while (!atual->folha) {
        int i = 0;
        while (i < atual->n && strcmp(chave, atual->chaves[i]) > 0)
            i++;

        if (atual->filhos[i] == NULL) {
            fprintf(stderr, "Erro: filho NULL durante descida na árvore\n");
            return;
        }

        BPlusNode *filho = atual->filhos[i];

        if (filho->n == M) {
            dividir_no(atual, i, filho);
            if (strcmp(chave, atual->chaves[i]) > 0)
                i++;

            filho = atual->filhos[i];
        }

        atual = filho;
    }

    inserir_em_no(atual, chave, valor);
}

void imprimir_bplus(BPlusNode *raiz) {
    if (raiz == NULL) {
        printf("arvore B+ vazia.\n");
        return;
    }

    // Vai até o primeiro nó folha
    while (raiz && !raiz->folha) {
        if (raiz->filhos[0] == NULL) {
            printf("Erro: filho[0] é NULL.\n");
            return;
        }
        raiz = raiz->filhos[0];
    }

    printf("=== Dados na arvore B+ ===\n");
    while (raiz) {
        printf("No folha com %d chaves\n", raiz->n);
        for (int i = 0; i < raiz->n; i++) {
            printf("Chave %s com %d registros:\n", raiz->chaves[i], raiz->dados[i].tamanho);
            for (int j = 0; j < raiz->dados[i].tamanho; j++) {
                Bdados d = raiz->dados[i].itens[j];
                printf("  %.24s: %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n",
                    d.data,
                    d.demanda_residual,
                    d.demanda_contratada,
                    d.geracao_despachavel,
                    d.geracao_termica,
                    d.importacoes,
                    d.geracao_renovavel_total,
                    d.carga_reduzida_manual,
                    d.capacidade_instalada,
                    d.perdas_geracao_total);
            }
        }

        if (raiz->prox == raiz) {
            printf("Erro: raiz->prox aponta para si mesma.\n");
            break;
        }
        raiz = raiz->prox;
    }
}

long long datetime_para_inteiro_BM(const char *datetime) {
    int ano, mes, dia, hora, min, seg;
    sscanf(datetime, "%d-%d-%d %d:%d:%d", &ano, &mes, &dia, &hora, &min, &seg);
    return (long long)ano * 10000000000LL +
           (long long)mes * 100000000 +
           (long long)dia * 1000000 +
           (long long)hora * 10000 +
           (long long)min * 100 +
           (long long)seg;
}

void buscar_intervalo_bplus_json(BPlusNode *raiz, const char *inicio_str, const char *fim_str, char **saida) {
    if (!raiz) {
        *saida = NULL;
        return;
    }

    long long inicio = datetime_para_inteiro_BM(inicio_str);
    long long fim = datetime_para_inteiro_BM(fim_str);

    size_t capacidade = 8192;
    size_t usado = 0;
    int primeiro = 1;

    char *json = malloc(capacidade);
    if (!json) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    usado += snprintf(json + usado, capacidade - usado, "[");

    // Pilha simples para percorrer árvore (usando recursão manual para evitar função aninhada)
    typedef struct {
        BPlusNode *no;
        int i;
    } StackItem;

    StackItem stack[128];
    int sp = 0;
    stack[sp].no = raiz;
    stack[sp].i = 0;

    while (sp >= 0) {
        BPlusNode *no = stack[sp].no;
        int i = stack[sp].i;

        if (!no->folha) {
            if (i <= no->n) {
                // empilha o filho i
                stack[sp].i++;
                sp++;
                if (sp >= 128) {
                    fprintf(stderr, "Stack overflow\n");
                    free(json);
                    exit(EXIT_FAILURE);
                }
                stack[sp].no = (BPlusNode *)no->filhos[i];
                stack[sp].i = 0;
            } else {
                // acabou filhos desse nó
                sp--;
            }
        } else {
            if (i < no->n) {
                ListaDados *lista = &no->dados[i];
                for (int j = 0; j < lista->tamanho; j++) {
                    Bdados *d = &lista->itens[j];
                    long long data_valor = datetime_para_inteiro_LSM(d->data);
                    if (data_valor >= inicio && data_valor <= fim) {
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
                            d->data,
                            d->demanda_residual,
                            d->demanda_contratada,
                            d->geracao_despachavel,
                            d->geracao_termica,
                            d->importacoes,
                            d->geracao_renovavel_total,
                            d->carga_reduzida_manual,
                            d->capacidade_instalada,
                            d->perdas_geracao_total
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
                }
                stack[sp].i++;
            } else {
                sp--;
            }
        }
    }

    usado += snprintf(json + usado, capacidade - usado, "\n]");
    json[usado] = '\0';

    *saida = json;
}

void liberar_bplus(BPlusNode *raiz) {
    if (!raiz) return;

    if (!raiz->folha) {
        for (int i = 0; i <= raiz->n; i++) {
            liberar_bplus(raiz->filhos[i]);
        }
    } else {
        for (int i = 0; i < raiz->n; i++) {
            lista_liberar(&raiz->dados[i]);
        }
    }
    free(raiz);
}