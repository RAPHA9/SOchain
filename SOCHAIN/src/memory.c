#include "memory.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>
#include <errno.h>

/* Função que reserva uma zona de memória dinâmica, inicializando-a a 0 */
void* allocate_dynamic_memory(int size) {
    void *ptr = calloc(1, size);
    if (!ptr) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

/* Função que cria e mapeia uma zona de memória partilhada.
 * Concatena o UID do utilizador ao nome para tornar único.
 */
void* create_shared_memory(char* name, int size) {
    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "%s_%d", name, getuid());
    
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    
    if (ftruncate(shm_fd, size) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    
    void* ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    
    // A memória já é mapeada com ftruncate, mas podemos garantir que está a zero.
    memset(ptr, 0, size);
    
    return ptr;
}

/* Liberta uma zona de memória dinâmica previamente alocada */
void deallocate_dynamic_memory(void* ptr) {
    free(ptr);
}

/* Remove uma zona de memória partilhada previamente criada */
void destroy_shared_memory(char* name, void* ptr, int size) {
    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "%s_%d", name, getuid());
    
    if (munmap(ptr, size) == -1) {
        perror("munmap");
    }
    
    if (shm_unlink(shm_name) == -1) {
        perror("shm_unlink");
    }
}

/* Função auxiliar para copiar uma transação */
static void copy_transaction(struct transaction *dest, const struct transaction *src) {
    dest->id = src->id;
    dest->src_id = src->src_id;
    dest->dest_id = src->dest_id;
    dest->amount = src->amount;
    dest->wallet_signature = src->wallet_signature;
    dest->server_signature = src->server_signature;
}

/* Escreve uma transação no buffer RA (Main -> Wallets).
 * Utiliza o int apontado por buffer->ptrs como índice de escrita.
 */
void write_main_wallets_buffer(struct ra_buffer* buffer, int buffer_size, struct transaction* tx) {
    if (*(buffer->ptrs) < buffer_size) {
        buffer->buffer[*(buffer->ptrs)] = *tx;
        (*(buffer->ptrs))++;
    } else {
        
        fprintf(stderr, "write_main_wallets_buffer: Buffer cheio. Transação %d perdida.\n", tx->id);
    }
}

/* Escreve uma transação no buffer circular (Wallets -> Servers).
 * Utiliza os índices 'in' e 'out' para controlar o buffer.
 */
void write_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx) {
    int next_in = (buffer->ptrs->in + 1) % buffer_size;
    // Se o próximo índice for igual a out, o buffer está cheio
    if (next_in == buffer->ptrs->out) {
        fprintf(stderr, "write_wallets_servers_buffer: Buffer circular cheio. Transação %d perdida.\n", tx->id);
        return;
    }
    buffer->buffer[buffer->ptrs->in] = *tx;
    buffer->ptrs->in = next_in;
}

/* Escreve uma transação no buffer RA (Servers -> Main).
 * Utiliza o int apontado por buffer->ptrs como índice de escrita.
 */
void write_servers_main_buffer(struct ra_buffer* buffer, int buffer_size, struct transaction* tx) {
    if (*(buffer->ptrs) < buffer_size) {
        buffer->buffer[*(buffer->ptrs)] = *tx;
        (*(buffer->ptrs))++;
    } else {
        // Buffer cheio; transação perdida
        fprintf(stderr, "write_servers_main_buffer: Buffer cheio. Transação %d perdida.\n", tx->id);
    }
}

/* Lê uma transação do buffer RA (Main -> Wallets) para a carteira com id 'wallet_id'.
 * Procura uma transação cuja origem (src_id) corresponda ao wallet_id.
 * Se encontrada, copia a transação para 'tx' e remove-a do buffer (deslocando os restantes).
 * Se não encontrar, define tx->id = -1.
 */
void read_main_wallets_buffer(struct ra_buffer* buffer, int wallet_id, int buffer_size, struct transaction* tx) {
    int found = 0;
    int count = *(buffer->ptrs);
    for (int i = 0; i < count; i++) {
        if (buffer->buffer[i].src_id == wallet_id) {
            copy_transaction(tx, &buffer->buffer[i]);
            found = 1;
           
            for (int j = i; j < count - 1; j++) {
                buffer->buffer[j] = buffer->buffer[j+1];
            }
            (*(buffer->ptrs))--;
            break;
        }
    }
    if (!found) {
        tx->id = -1;
    }
}

/* Lê uma transação do buffer circular (Wallets -> Servers).
 * Se o buffer não estiver vazio (in != out), copia a transação na posição 'out' e atualiza o índice.
 * Caso contrário, define tx->id = -1.
 */
void read_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx) {
    if (buffer->ptrs->in == buffer->ptrs->out) {
        // Buffer vazio
        tx->id = -1;
    } else {
        copy_transaction(tx, &buffer->buffer[buffer->ptrs->out]);
        buffer->ptrs->out = (buffer->ptrs->out + 1) % buffer_size;
    }
}

/* Lê uma transação do buffer RA (Servers -> Main) identificada pelo tx_id.
 * Procura a transação com o id igual a tx_id; se encontrada, copia para 'tx' e remove-a do buffer.
 * Caso não seja encontrada, define tx->id = -1.
 */
void read_servers_main_buffer(struct ra_buffer* buffer, int tx_id, int buffer_size, struct transaction* tx) {
    int found = 0;
    int count = *(buffer->ptrs);
    for (int i = 0; i < count; i++) {
        if (buffer->buffer[i].id == tx_id) {
            copy_transaction(tx, &buffer->buffer[i]);
            found = 1;
            
            for (int j = i; j < count - 1; j++) {
                buffer->buffer[j] = buffer->buffer[j+1];
            }
            (*(buffer->ptrs))--;
            break;
        }
    }
    if (!found) {
        tx->id = -1;
    }
}
