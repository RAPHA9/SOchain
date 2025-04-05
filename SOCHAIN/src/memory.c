#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> 
#include <sys/mman.h> 
#include <string.h> 


void* allocate_dynamic_memory(int size) {

    void* ptr = calloc(size, 1);
    
    if(ptr == NULL){
        exit(1);
    }
    return ptr;
}

void* create_shared_memory(char* name, int size){
    int ret;
    uid_t user_id = getuid();
    char memory_id[256]; 
    //concatena o user_id com o name
    snprintf(memory_id, sizeof(memory_id), "%s_%d", name, user_id);

    //cria o identificador
    int fd = shm_open(memory_id, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1){
    perror("Erro ao criar a memória compartilhada");
    exit(1);
    }
    //define o tamanho da memória
    ret = ftruncate(fd, size);
    if (ret == -1){
    perror("Erro ao definir o tamanho da memória compartilhada");
     exit(2);
    }
    // mapeia a memória partilhada 
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("shm-mmap");
        exit(3);
    }
    // preenche a zona de memória com o valor 0
    memset(ptr, 0, size);
    return ptr;
}

void deallocate_dynamic_memory(void* ptr){
    free(ptr);
}

void destroy_shared_memory(char* name, void* ptr, int size){
    int ret;
    ret = munmap(ptr, size);
    if (ret == -1){
        perror("Erro (munmap)");
        exit(1);
    }
    uid_t user_id = getuid();
    char memory_id[256]; 
    //concatena o user_id com o name
    snprintf(memory_id, sizeof(memory_id), "%s_%d", name, user_id);

    ret = shm_unlink(memory_id);
    if (ret == -1){
        perror("Erro (shm_unlink)");
        exit(2);
       }
    }

void write_main_wallets_buffer(struct ra_buffer* buffer, int buffer_size, struct transaction* tx){
    
    for(int i=0; i < buffer_size; i++){
        if(buffer->ptrs[i] == 0){
            buffer->buffer[i] = *tx;
            //marca a posição como ocupada
            buffer->ptrs[i]= 1;
            return;
        }
    }
    // Se não houver nenhuma posição livre, não escreve nada e a transação é perdida
}

void write_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx){
    //verificar se o está cheio
    if((buffer-> ptrs-> in+1) % buffer_size == buffer->ptrs->out){
        return;
    }

    //escreve a transação
    buffer->buffer[buffer->ptrs->in] = * tx;

    //atualiza o apontador de escrita
    buffer-> ptrs-> in = (buffer->ptrs->in + 1) % buffer_size;
}

void write_servers_main_buffer(struct ra_buffer* buffer, int buffer_size, struct transaction* tx){
    for(int i=0; i < buffer_size; i++){
        if(buffer->ptrs[i] == 0){
            buffer->buffer[i] = *tx;
            //marca a posição como ocupada
            buffer->ptrs[i]= 1;
            return;
        }
    }
    // Se não houver nenhuma posição livre, não escreve nada e a transação é perdida
}

void read_main_wallets_buffer(struct ra_buffer* buffer, int wallet_id, int buffer_size, struct transaction* tx){
    for(int i=0; i < buffer_size; i++){
        if(buffer->ptrs[i] == 1 && buffer->buffer[i].dest_id == wallet_id){
            //lê transação
            *tx = buffer->buffer[i];
            
            //marca a posição como livre
             buffer->ptrs[i]= 0;
            return;
        }
    }
    //Se não houver nenhuma transação disponível, afeta tx->id com o valor -1
    tx->id = -1;
}

void read_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx){
    //verifica se está vazio
    if(buffer-> ptrs-> in == buffer->ptrs->out){
         //Se não houver nenhuma transação disponível, afeta tx->id com o valor -1
        tx->id = -1;
        return;
    }
     //lê transação
    *tx = buffer->buffer[buffer->ptrs->out];

    //atualiza o apontador de leitura
    buffer->ptrs->out = (buffer->ptrs->out + 1) % buffer_size;
       
}

void read_servers_main_buffer(struct ra_buffer* buffer, int tx_id, int buffer_size, struct transaction* tx){
   for(int i=0; i < buffer_size; i++){
        if(buffer->ptrs[i] == 1 && buffer->buffer[i].id == tx_id){
            //lê transação
            *tx = buffer->buffer[i];
            
            //marca a posição como livre
             buffer->ptrs[i]= 0;
            return;
        }
    }
    //Se não houver nenhuma transação disponível, afeta tx->id com o valor -1
    tx->id = -1;
}
