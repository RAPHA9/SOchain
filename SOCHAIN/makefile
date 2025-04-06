# Compilador
CC = gcc
CFLAGS = -Wall -Wextra -Iinc

# Diretórios
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Arquivos
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
EXEC = $(BIN_DIR)/sochain

# Bibliotecas necessárias
LIBS = -lrt -pthread

# Regra principal
all: $(EXEC)

# Criar diretórios se não existirem
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Criar o executável
$(EXEC): $(OBJ_DIR) $(BIN_DIR) $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LIBS)

# Compilar arquivos .c para .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
