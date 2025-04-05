#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "main.h"
#include "process.h"

/* Lê os argumentos da linha de comando e guarda-os na estrutura info_container.
 * Espera 5 argumentos: init_balance, n_wallets, n_servers, buffers_size e max_txs.
 */
void main_args(int argc, char *argv[], struct info_container *info) {
    if (argc < 6) {
        fprintf(stderr, "Uso: %s init_balance n_wallets n_servers buffers_size max_txs\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    info->init_balance = atof(argv[1]);
    info->n_wallets = atoi(argv[2]);
    info->n_servers = atoi(argv[3]);
    info->buffers_size = atoi(argv[4]);
    info->max_txs = atoi(argv[5]);
}

/* Reserva a memória dinâmica necessária para os arrays da estrutura info_container
 * e para os buffers de comunicação. Inicializa os saldos e estatísticas.
 */
void create_dynamic_memory_structs(struct info_container* info, struct buffers* buffs) {
    info->balances = allocate_dynamic_memory(sizeof(float) * info->n_wallets);
    for (int i = 0; i < info->n_wallets; i++) {
        info->balances[i] = info->init_balance;
    }
    info->wallets_pids = allocate_dynamic_memory(sizeof(int) * info->n_wallets);
    info->wallets_stats = allocate_dynamic_memory(sizeof(int) * info->n_wallets);
    for (int i = 0; i < info->n_wallets; i++) {
        info->wallets_stats[i] = 0;
    }
    info->servers_pids = allocate_dynamic_memory(sizeof(int) * info->n_servers);
    info->servers_stats = allocate_dynamic_memory(sizeof(int) * info->n_servers);
    for (int i = 0; i < info->n_servers; i++) {
        info->servers_stats[i] = 0;
    }
    info->terminate = allocate_dynamic_memory(sizeof(int));
    *(info->terminate) = 0;

    buffs->buff_main_wallets = allocate_dynamic_memory(sizeof(struct ra_buffer));
    buffs->buff_wallets_servers = allocate_dynamic_memory(sizeof(struct circ_buffer));
    buffs->buff_servers_main = allocate_dynamic_memory(sizeof(struct ra_buffer));
}

/* Reserva a memória partilhada para os arrays e buffers necessários ao SOchain.
 * Para cada zona partilhada, utiliza a função create_shared_memory.
 */
void create_shared_memory_structs(struct info_container* info, struct buffers* buffs) {
    info->balances = create_shared_memory("SHM_BALANCES", sizeof(float) * info->n_wallets);
    info->wallets_stats = create_shared_memory("SHM_WALLETS_STATS", sizeof(int) * info->n_wallets);
    info->servers_stats = create_shared_memory("SHM_SERVERS_STATS", sizeof(int) * info->n_servers);
    info->terminate = create_shared_memory("SHM_TERMINATE", sizeof(int));
    for (int i = 0; i < info->n_wallets; i++) {
    	info->balances[i] = info->init_balance;
    }

    *(info->terminate) = 0;

    buffs->buff_main_wallets = create_shared_memory("SHM_MAIN_WALLETS_BUFFER", sizeof(struct transaction) * info->buffers_size);
    buffs->buff_wallets_servers = create_shared_memory("SHM_WALLETS_SERVERS_BUFFER", sizeof(struct transaction) * info->buffers_size);
    buffs->buff_servers_main = create_shared_memory("SHM_SERVERS_MAIN_BUFFER", sizeof(struct transaction) * info->buffers_size);
}

/* Liberta a memória dinâmica previamente alocada. */
void destroy_dynamic_memory_structs(struct info_container* info, struct buffers* buffs) {
    deallocate_dynamic_memory(info->balances);
    deallocate_dynamic_memory(info->wallets_pids);
    deallocate_dynamic_memory(info->wallets_stats);
    deallocate_dynamic_memory(info->servers_pids);
    deallocate_dynamic_memory(info->servers_stats);
    deallocate_dynamic_memory(info->terminate);
    deallocate_dynamic_memory(buffs->buff_main_wallets);
    deallocate_dynamic_memory(buffs->buff_wallets_servers);
    deallocate_dynamic_memory(buffs->buff_servers_main);
    deallocate_dynamic_memory(info);
    deallocate_dynamic_memory(buffs);
}

/* Remove a memória partilhada previamente alocada. */
void destroy_shared_memory_structs(struct info_container* info, struct buffers* buffs) {
    destroy_shared_memory("SHM_BALANCES", info->balances, sizeof(float) * info->n_wallets);
    destroy_shared_memory("SHM_WALLETS_STATS", info->wallets_stats, sizeof(int) * info->n_wallets);
    destroy_shared_memory("SHM_SERVERS_STATS", info->servers_stats, sizeof(int) * info->n_servers);
    destroy_shared_memory("SHM_TERMINATE", info->terminate, sizeof(int));
    destroy_shared_memory("SHM_MAIN_WALLETS_BUFFER", buffs->buff_main_wallets, sizeof(struct transaction) * info->buffers_size);
    destroy_shared_memory("SHM_WALLETS_SERVERS_BUFFER", buffs->buff_wallets_servers, sizeof(struct transaction) * info->buffers_size);
    destroy_shared_memory("SHM_SERVERS_MAIN_BUFFER", buffs->buff_servers_main, sizeof(struct transaction) * info->buffers_size);
}

/* Cria os processos para as carteiras e servidores.
 * Os PIDs resultantes são armazenados nos arrays da estrutura info_container.
 */
void create_processes(struct info_container* info, struct buffers* buffs) {
    // Lançar processos Wallet
    for (int i = 0; i < info->n_wallets; i++) {
        info->wallets_pids[i] = launch_wallet(i, info, buffs);
    }
    // Lançar processos Server
    for (int i = 0; i < info->n_servers; i++) {
        info->servers_pids[i] = launch_server(i, info, buffs);
    }
}

/* Função responsável pela interação com o utilizador.
 * Lê comandos do stdin e invoca a operação correspondente.
 */
void user_interaction(struct info_container* info, struct buffers* buffs) {
    char command[16];
    int tx_counter = 0;
    while (1) {
        printf("SOchain> ");
        scanf("%s", command);
        if (strcmp(command, "bal") == 0) {
            print_balance(info);
        } else if (strcmp(command, "trx") == 0) {
            create_transaction(&tx_counter, info, buffs);
        } else if (strcmp(command, "rcp") == 0) {
            receive_receipt(info, buffs);
        } else if (strcmp(command, "stat") == 0) {
            print_stat(tx_counter, info);
        } else if (strcmp(command, "help") == 0) {
            help();
        } else if (strcmp(command, "end") == 0) {
            end_execution(info, buffs);
            break;
        } else {
            printf("Comando inválido. Digite 'help' para ver os comandos disponíveis.\n");
        }
    }
}

/* Imprime as estatísticas finais do SOchain. */
void write_final_statistics(struct info_container* info) {
    printf("\nEstatísticas finais:\n");
    printf("Carteiras:\n");
    for (int i = 0; i < info->n_wallets; i++) {
        printf("  Wallet %d assinou %d transações\n", i, info->wallets_stats[i]);
    }
    printf("Servidores:\n");
    for (int i = 0; i < info->n_servers; i++) {
        printf("  Server %d processou %d transações\n", i, info->servers_stats[i]);
    }
}

/* Termina a execução do SOchain.
 * Atualiza a flag terminate, aguarda a terminação dos processos filhos e escreve as estatísticas finais.
 */
void end_execution(struct info_container* info, struct buffers* buffs) {
    (void)buffs;
    *(info->terminate) = 1;
    wait_processes(info);
    write_final_statistics(info);
}

/* Aguarda a terminação dos processos filhos (Wallets e Servers). */
void wait_processes(struct info_container* info) {
    for (int i = 0; i < info->n_wallets; i++) {
        wait_process(info->wallets_pids[i]);
    }
    for (int i = 0; i < info->n_servers; i++) {
        wait_process(info->servers_pids[i]);
    }
}

/* Lê do stdin o identificador de uma wallet e imprime o seu saldo atual. */
void print_balance(struct info_container* info) {
    int wallet_id;
    printf("Insira o ID da wallet: ");
    scanf("%d", &wallet_id);
    if (wallet_id < 0 || wallet_id >= info->n_wallets) {
        printf("ID inválido.\n");
    } else {
        printf("Saldo da wallet %d: %.2f\n", wallet_id, info->balances[wallet_id]);
    }
}

/* Cria uma nova transação com os dados inseridos pelo utilizador e
 * escreve-a no buffer entre a main e as carteiras. Incrementa o contador de transações.
 */
void create_transaction(int* tx_counter, struct info_container* info, struct buffers* buffs) {
    if (*tx_counter >= info->max_txs) {
        printf("Número máximo de transações atingido.\n");
        return;
    }
    struct transaction tx;
    printf("Insira src_id, dest_id e amount: ");
    scanf("%d %d %f", &tx.src_id, &tx.dest_id, &tx.amount);
    tx.id = *tx_counter;
    tx.wallet_signature = -1;
    tx.server_signature = -1;
    write_main_wallets_buffer(buffs->buff_main_wallets, info->buffers_size, &tx);
    (*tx_counter)++;
}

/* Lê do buffer de comunicação entre os servidores e a main o recibo da transação com o ID indicado. */
void receive_receipt(struct info_container* info, struct buffers* buffs) {
    int tx_id;
    printf("Insira o ID da transação: ");
    scanf("%d", &tx_id);
    struct transaction tx;
    tx.id = -1;
    read_servers_main_buffer(buffs->buff_servers_main, tx_id, info->buffers_size, &tx);
    if (tx.id == -1) {
        printf("Recibo não disponível ou transação não concluída.\n");
    } else {
        printf("Recibo da transação %d:\n", tx.id);
        printf("  src: %d, dest: %d, amount: %.2f\n", tx.src_id, tx.dest_id, tx.amount);
        printf("  wallet_signature: %d, server_signature: %d\n", tx.wallet_signature, tx.server_signature);
    }
}

/* Imprime as estatísticas atuais do sistema. */
void print_stat(int tx_counter, struct info_container* info) {
    printf("\nEstatísticas atuais:\n");
    printf("Transações criadas: %d\n", tx_counter);
    printf("Saldos das wallets:\n");
    for (int i = 0; i < info->n_wallets; i++) {
        printf("  Wallet %d: %.2f\n", i, info->balances[i]);
    }
    printf("Wallets (transações assinadas):\n");
    for (int i = 0; i < info->n_wallets; i++) {
        printf("  Wallet %d: %d\n", i, info->wallets_stats[i]);
    }
    printf("Servers (transações processadas):\n");
    for (int i = 0; i < info->n_servers; i++) {
        printf("  Server %d: %d\n", i, info->servers_stats[i]);
    }
    printf("\n");
}

/* Exibe os comandos disponíveis para o utilizador. */
void help() {
    printf("Comandos disponíveis:\n");
    printf("  bal  - Consultar saldo de uma wallet\n");
    printf("  trx  - Criar uma nova transação\n");
    printf("  rcp  - Consultar recibo de uma transação\n");
    printf("  stat - Mostrar estatísticas atuais do sistema\n");
    printf("  help - Exibir esta mensagem de ajuda\n");
    printf("  end  - Encerrar a execução do SOchain\n");
}

/* Função principal do SOchain.
 * Inicializa as estruturas, cria a memória dinâmica e partilhada, lança os processos filhos,
 * inicia a interação com o utilizador e, ao terminar, liberta os recursos alocados.
 */
int main(int argc, char *argv[]) {
    struct info_container* info = allocate_dynamic_memory(sizeof(struct info_container));
    struct buffers* buffs = allocate_dynamic_memory(sizeof(struct buffers));

    main_args(argc, argv, info);
    create_dynamic_memory_structs(info, buffs);
    create_shared_memory_structs(info, buffs);
    create_processes(info, buffs);
    user_interaction(info, buffs);
    destroy_shared_memory_structs(info, buffs);
    destroy_dynamic_memory_structs(info, buffs);
    return 0;
}
