#ifndef LSM_TREE
#define LSM_TREE

#define MAX_MEMTABLE 1000

extern int memtable_size;
extern int sstable_counter;

typedef struct {
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
}Dados;

extern Dados memtable[MAX_MEMTABLE];

char* get_lsm_directory();
long long datetime_para_inteiro_LSM(const char *datetime);
int comparar_dados(const void *a, const void *b);
void merge_sstables(const char *file1, const char *file2, const char *output);
void flush_memtable();
int inserir_dado(const Dados *dado);
void buscar_intervalo_lsm(const char *inicio_str, const char *fim_str, char **saida);

int eh_arquivo_sstable(const char *nome);
void printar_arquivo(const char *caminho_completo, const char *nome);
void printar_dados_todos_arquivos(const char *pasta);

#endif