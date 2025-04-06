# SOchain

**SOchain** é um sistema distribuído simulado que representa uma rede simples de blockchain entre carteiras (wallets), servidores e uma aplicação principal (main). Os processos comunicam entre si através de buffers em memória partilhada, simulando transações, assinaturas e validações.

## 📁 Estrutura do Projeto
SOchain/ ├── bin/ # Diretório onde o executável final será colocado ├── inc/ # Ficheiros de cabeçalho (.h) ├── obj/ # Objetos compilados (.o) ├── src/ # Código-fonte (.c) ├── makefile # Ficheiro de compilação └── README.md # Este ficheiro

------------------------------------------------

## 🔧 Compilação

Para compilar o projeto, basta correr o seguinte comando no terminal, na raiz do projeto:

make

Para limpar os ficheiros compilados (objetos e binários), corra:

make clean

-------------------------------------------

Após a compilação, corre o programa com:

./bin/sochain

-----------------------------------------------

🧠 Dependências
Este projeto usa:

- gcc como compilador

- POSIX shared memory (shm_open, mmap, shm_unlink)

- POSIX threads (pthread)

- Biblioteca librt para suporte a temporização e memória partilhada

O sistema operativo usado foi o Windows11 com WSL que é compatível com estas bibliotecas (Ubuntu recomendado).


