#include "process.h"
#include "wallet.h"   // Adicionado para declarar execute_wallet
#include "server.h"   // Adicionado para declarar execute_server
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Função que inicia um novo processo Wallet através do fork.
 * No processo filho, executa execute_wallet e encerra com o retorno da função.
 * No processo pai, retorna o pid do processo criado.
 */
int launch_wallet(int wallet_id, struct info_container* info, struct buffers* buffs) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        int ret = execute_wallet(wallet_id, info, buffs);
        exit(ret);
    }
    return (int)pid;
}

/* Função que inicia um novo processo Server através do fork.
 * No processo filho, executa execute_server e encerra com o retorno da função.
 * No processo pai, retorna o pid do processo criado.
 */
int launch_server(int server_id, struct info_container* info, struct buffers* buffs) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        int ret = execute_server(server_id, info, buffs);
        exit(ret);
    }
    return (int)pid;
}

/* Função que espera que o processo com PID process_id termine.
 * Retorna o valor de saída do processo se este terminar normalmente.
 */
int wait_process(int process_id) {
    int status;
    pid_t w = waitpid(process_id, &status, 0);
    if (w == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        return -1;
    }
}
