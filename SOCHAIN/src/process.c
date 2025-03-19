#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


/* Função que inicia um novo processo Wallet */
int launch_wallet(int wallet_id, struct info_container* info, struct buffers* buffs) {
   pid_t pid = fork();

   if(pid < 0){
        perror("Error creating wallet");
        return -1;
   }
   else if(pid == 0){
        exit(execute_wallet(wallet_id, info, buffs));    
   }

   return pid;
}

/* Função que inicia um novo processo Server */
int launch_server(int server_id, struct info_container* info, struct buffers* buffs) {
    pid_t pid = fork();

    if (pid < 0) {  
        perror("Erro ao criar processo Server");
        return -1;
    } 
    else if (pid == 0) {  
        exit(execute_server(server_id, info, buffs)); 
    }

    return pid;  
}

/* Função que espera que um processo com PID process_id termine */
int wait_process(int process_id) {
    int status;
    pid_t result = waitpid(process_id, &status, 0);  

    if (result == -1) {  
        perror("Erro ao esperar pelo processo");
        return -1;
    }

    if (WIFEXITED(status)) { 
        return WEXITSTATUS(status); 
    }

    return -1;  
}
