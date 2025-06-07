SOchain
SOchain is a simulated distributed system that represents a simple blockchain network among wallets, servers, and a main application (main). The processes communicate with each other through shared memory buffers, simulating transactions, signatures, and validations.

📁 Project Structure
SOchain/
├── bin/ # Directory where the final executable will be placed
├── inc/ # Header files (.h)
├── obj/ # Compiled object files (.o)
├── src/ # Source code (.c)
├── makefile # Build file
└── README.md # This file

🔧 Compilation
To compile the project, simply run the following command in the terminal at the project root:

bash
Copiar
Editar
make
To clean compiled files (objects and binaries), run:

bash
Copiar
Editar
make clean
After compilation, run the program with:

bash
Copiar
Editar
./bin/sochain
🧠 Dependencies
This project uses:

gcc as the compiler

POSIX shared memory (shm_open, mmap, shm_unlink)

POSIX threads (pthread)

librt library for timing and shared memory support

The operating system used was Windows 11 with WSL, which is compatible with these libraries (Ubuntu recommended).
