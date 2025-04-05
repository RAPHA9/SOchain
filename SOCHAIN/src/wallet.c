#include "wallet.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Função que lê uma transação do buffer entre a Main e as Wallets.
 * Se *info->terminate for 1, retorna imediatamente definindo tx->id como -1.
 * Apenas lê a transação se o src_id for igual ao wallet_id; caso contrário, ignora-a.
 */
void wallet_receive_transaction(struct transaction* tx, int wallet_id, struct info_container* info, struct buffers* buffs) {
    if (*info->terminate == 1) {
        tx->id = -1;
        return;
    }
   
    read_main_wallets_buffer(buffs->buff_main_wallets, wallet_id, info->buffers_size, tx);
    
    
    if (tx->id != -1 && tx->src_id != wallet_id) {
        tx->id = -1;
    }
}

/* Função que processa a transação.
 * Se o src_id da transação for igual a wallet_id, assina a transação (atribui wallet_signature) 
 * e incrementa o contador de transações assinadas pela carteira.
 */
void wallet_process_transaction(struct transaction* tx, int wallet_id, struct info_container* info) {
    if (tx->src_id == wallet_id) {
        tx->wallet_signature = wallet_id;
        info->wallets_stats[wallet_id]++;
    }
}

/* Função que envia a transação assinada para o buffer entre as Wallets e os Servers.
 * Se não houver espaço disponível, a transação é perdida.
 */
void wallet_send_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs) {
    write_wallets_servers_buffer(buffs->buff_wallets_servers, info->buffers_size, tx);
}

/* Função principal de uma Wallet.
 * Executa num ciclo infinito enquanto *info->terminate for 0. Em cada iteração:
 * - Tenta ler uma transação da Main cujo src_id seja igual ao wallet_id.
 * - Se a transação for válida, a assina e encaminha para os Servers.
 * - Se não houver transação (tx.id == -1), aguarda alguns milisegundos antes de tentar novamente.
 * Ao término, retorna o número de transações assinadas.
 */
int execute_wallet(int wallet_id, struct info_container* info, struct buffers* buffs) {
    int signed_count = 0;
    while (*info->terminate == 0) {
        struct transaction tx;
        tx.id = -1;
        
        wallet_receive_transaction(&tx, wallet_id, info, buffs);
        
        if (tx.id == -1) {
            usleep(100000); 
            continue;
        }
        
        wallet_process_transaction(&tx, wallet_id, info);
        wallet_send_transaction(&tx, info, buffs);
        signed_count++;
    }
    return signed_count;
}
