#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <thread>
#include <ctime>

using namespace std;

class Process {
public:
    int pid;
    int sizeKb;
    int pages;

    Process() : pid(0), sizeKb(0), pages(0) {}
    Process(int id, int size, int pageSize) : pid(id), sizeKb(size) {
        pages = (size + pageSize - 1) / pageSize;
    }
};

// Numero random entre min y max 
int randomInt(int min, int max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

void memoryInitializer(int &physicalPages, int &virtualPages, int &pageSize) {
    int physicalMemoryMb;
    cout << "Ingrese el tamaño de la memoria física (MB): ";
    cin >> physicalMemoryMb;

    cout << "Ingrese el tamaño de cada página (KB): ";
    cin >> pageSize;

    int physicalMemoryKb = physicalMemoryMb * 1024;
    int virtualMemoryKb = randomInt(physicalMemoryKb * 1.5, physicalMemoryKb * 4.5);

    physicalPages = physicalMemoryKb / pageSize;
    virtualPages = virtualMemoryKb / pageSize;

    cout << "Memoria física: " << physicalMemoryKb << " KB (" << physicalPages << " páginas)\n";
    cout << "Memoria virtual: " << virtualMemoryKb << " KB (" << virtualPages << " páginas)\n";
}

// void de la simulacionn
void pagSim() {
    int physicalPages, virtualPages, pageSize;
    memoryInitializer(physicalPages, virtualPages, pageSize);

    list<pair<int, int>> ram;          //usamos un list para la ram y el swap ya que es un poco mas rapido de insertar y eliminar
    list<pair<int, int>> swapSpace;
    unordered_map<int, Process> processes;
    int pidCounter = 1;

    int processMinSize, processMaxSize;
    cout << "Ingrese el tamaño mínimo de los procesos (KB): ";
    cin >> processMinSize;
    cout << "Ingrese el tamaño máximo de los procesos (KB): ";
    cin >> processMaxSize;

    time_t startTime = time(nullptr);

    while (true) {
        time_t currentTime = time(nullptr);
        double elapsedTime = difftime(currentTime, startTime);

        // Creacion de un proceso cada 2 segundos
        if (static_cast<int>(elapsedTime) % 2 == 0) {
            int processSize = randomInt(processMinSize, processMaxSize); // Tamaño
            Process newProcess(pidCounter, processSize, pageSize); // Creacion del proceso
            processes.emplace(pidCounter, newProcess); // Agregado del proceso a la lista de procesos
            cout << "\nCreando proceso " << pidCounter << " con tamaño " << processSize
                 << " KB (" << newProcess.pages << " páginas)\n";

            // Carga de las paginas a la memoria si hay espacio o si no lo carga en swap
            for (int page = 0; page < newProcess.pages; ++page) {
                if (ram.size() < physicalPages) {
                    ram.push_back({pidCounter, page});
                    cout << "-> Página (" << pidCounter << ", " << page << ") cargada en RAM.\n";
                } else if (swapSpace.size() < virtualPages - physicalPages) {
                    swapSpace.push_back({pidCounter, page});
                    cout << "-> Página (" << pidCounter << ", " << page << ") cargada en Swap.\n";
                } else {
                    cout << "\nMemoria llena! Terminando simulación.\n";
                    return;
                }
            }
            pidCounter++;
            this_thread::sleep_for(chrono::seconds(2));
        }

        // Eventos pasados 30 segundos
        if (elapsedTime >= 1 && (static_cast<int>(elapsedTime) - 30) % 5 == 0) {
            // ELiminacion de un random cada 5 segundos
            if (!processes.empty()) {
                int pidToKill = randomInt(1, pidCounter - 1);
                while (processes.find(pidToKill) == processes.end()) {
                    pidToKill = randomInt(1, pidCounter - 1);
                }
                cout << "\nTerminando proceso " << pidToKill << "\n";

                auto removePages = [&](list<pair<int, int>> &memory) {
                    memory.remove_if([pidToKill](pair<int, int> page) {
                        return page.first == pidToKill;
                    });
                };

                removePages(ram);
                removePages(swapSpace);
                processes.erase(pidToKill);
            }

            // Acceso a una direccion virtual random cada 5 segundos
            if (!processes.empty()) {
                int randomPid = randomInt(1, pidCounter - 1);
                while (processes.find(randomPid) == processes.end()) {
                    randomPid = randomInt(1, pidCounter - 1);
                }
                auto &process = processes[randomPid];
                int randomPage = randomInt(0, process.pages - 1);
                int virtualAddress = randomPage * pageSize * 1024 + randomInt(0, pageSize * 1024 - 1);

                cout << "\nAccediendo a dirección virtual " << virtualAddress << " del proceso " << randomPid << "\n";
                cout << "Correspondiente a página " << randomPage << "\n";

                auto pagePair = make_pair(randomPid, randomPage);
                auto it = find(ram.begin(), ram.end(), pagePair);
                if (it != ram.end()) {
                    cout << "¡Página encontrada en RAM!\n";
                } else {
                    cout << "Page fault! Realizando swapping...\n";

                    // Si es que detecto que la pagina esta en swap se carga a ram
                    if (ram.size() < physicalPages) {
                        ram.push_back(pagePair);
                        swapSpace.remove(pagePair);
                        cout << "-> Página " << randomPage << " del proceso " << randomPid << " cargada en RAM.\n";
                    } else {
                        // Se cambia por la primera pagina de la lista de ram (FIFO)
                        auto pageToSwap = ram.front();
                        ram.pop_front();

                        if (swapSpace.size() < virtualPages - physicalPages) {
                            swapSpace.push_back(pageToSwap);
                            cout << "-> Swapping página (" << pageToSwap.first << ", " << pageToSwap.second << ") de RAM a Swap.\n";
                        } else {
                            cout << "\nSwap lleno! Terminando simulación.\n";
                            return;
                        }

                        // cARGA la pagina a ram
                        ram.push_back(pagePair);
                        swapSpace.remove(pagePair);
                        cout << "-> Página (" << randomPid << ", " << randomPage << ") cargada en RAM.\n";
                    }
                }
            }
            this_thread::sleep_for(chrono::seconds(5));
        }

        // Chequeo si la memoria tanto fisica como virtual estan llenas y si lo estan termina la simulacion
        if (ram.size() == physicalPages && swapSpace.size() == virtualPages - physicalPages) {
            cout << "\n¡Memoria llena! Terminando simulación.\n";
            return;
        }

        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

int main() {
    pagSim();
    return 0;
}
