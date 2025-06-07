# SOchain

**SOchain** is a simulated distributed system that represents a simple blockchain network among wallets, servers, and a main application (main). The processes communicate with each other through shared memory buffers, simulating transactions, signatures, and validations.

## 📁 Structure
SOchain/ ├── bin/  ├── inc/ (.h) ├── obj/ (.o) ├── src/ (.c) ├── makefile └── README.md 

------------------------------------------------

## 🔧 

make


make clean

-------------------------------------------

./bin/sochain

-----------------------------------------------

🧠 Dependencies

- gcc 

- POSIX shared memory (shm_open, mmap, shm_unlink)

- POSIX threads (pthread)

The operating system used was Windows 11 with WSL, which is compatible with these libraries (Ubuntu recommended).
