#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Função que lê uma transação do buffer entre as carteiras e os servidores.
 * Se *info->terminate for 1, retorna imediatamente com tx->id = -1.
 */
void server_receive_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs) {
    if (*info->terminate == 1) {
        tx->id = -1;
        return;
    }
    // Tenta ler uma transação do buffer circular entre carteiras e servidores usando buffers_size
    read_wallets_servers_buffer(buffs->buff_wallets_servers, info->buffers_size, tx);
}

/* Função que processa a transação.
 * Verifica se os identificadores são válidos, se a carteira de origem tem saldo suficiente
 * e se a assinatura da carteira corresponde ao src_id. Se válida, atualiza os saldos,
 * assina a transação com o server_id e incrementa o contador de transações processadas pelo servidor.
 * Se inválida, define tx->server_signature com um valor indicativo (aqui, -1).
 */
void server_process_transaction(struct transaction* tx, int server_id, struct info_container* info) {
    if (tx->id == -1) return;
    
    if (tx->src_id < 0 || tx->dest_id < 0 || tx->amount <= 0) {
        tx->server_signature = -1;
        return;
    }
    
    if (tx->wallet_signature != tx->src_id) {
        tx->server_signature = -1;
        return;
    }
    
    if (info->balances[tx->src_id] < tx->amount) {
        tx->server_signature = -1;
        return;
    }
    
    info->balances[tx->src_id] -= tx->amount;
    info->balances[tx->dest_id] += tx->amount;
    
    tx->server_signature = server_id;
    
    info->servers_stats[server_id]++;
}

/* Função que envia a transação processada para a Main através do buffer correspondente.
 * Se o servidor não tiver assinado a transação, nada é enviado.
 */
void server_send_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs) {
    if (tx->server_signature != -1) {
        write_servers_main_buffer(buffs->buff_servers_main, info->buffers_size, tx);
    }
}

/* Função principal de um servidor.
 * Executa num ciclo infinito enquanto *info->terminate for 0.
 * Em cada iteração, tenta receber uma transação; se existir (tx.id != -1), processa e envia o comprovativo.
 * Aguarda alguns milisegundos se não houver transações disponíveis.
 * Ao terminar, retorna o número de transações processadas.
 */
int execute_server(int server_id, struct info_container* info, struct buffers* buffs) {
    int processed = 0;
    while(*info->terminate == 0) {
        struct transaction tx;
        tx.id = -1;
        
        server_receive_transaction(&tx, info, buffs);
        
        if (tx.id == -1) {
            usleep(100000); // 100 milisegundos
            continue;
        }
        
        server_process_transaction(&tx, server_id, info);
        
        if (tx.server_signature != -1) {
            server_send_transaction(&tx, info, buffs);
            processed++;
        }
    }
    return processed;
}
