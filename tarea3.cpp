#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <thread>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Clase que representa al proceso
class Proceso {
public:
    int pid;
    int size_kb;
    int pages;

    Proceso() : pid(0), size_kb(0), pages(0) {}
    Proceso(int id, int size, int page_size) : pid(id), size_kb(size) {
        pages = (size + page_size - 1) / page_size;
    }
};

// Numero random entre min y max 
int randomInt(int min, int max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

// Parametros de inicializacion de la memoria (donde se pide los tamaños y se asignan los valores en kb dsps)
void MemoryInitializer(int &physical_pages, int &virtual_pages, int &page_size) {
    int physical_memory_mb;
    cout << "Ingrese el tamaño de la memoria física (MB): ";
    cin >> physical_memory_mb;

    cout << "Ingrese el tamaño de cada página (KB): ";
    cin >> page_size;

    int physical_memory_kb = physical_memory_mb * 1024;
    int virtual_memory_kb = randomInt(physical_memory_kb * 1.5, physical_memory_kb * 4.5);

    physical_pages = physical_memory_kb / page_size;
    virtual_pages = virtual_memory_kb / page_size;

    cout << "Memoria física: " << physical_memory_kb << " KB (" << physical_pages << " páginas)\n";
    cout << "Memoria virtual: " << virtual_memory_kb << " KB (" << virtual_pages << " páginas)\n";
}

// simulacion de la paginacion utilizando FIFO
void PagSim() {
    int physical_pages, virtual_pages, page_size;
    MemoryInitializer(physical_pages, virtual_pages, page_size);

    list<pair<int, int>> ram;          // RAM usamos list para que sea un poco mas optima la insercion y eliminacion
    list<pair<int, int>> swap_space;   // Swap
    unordered_map<int, Proceso> processes; // Procesos
    int pid_counter = 1;

    int process_min_size, process_max_size;
    cout << "Ingrese el tamaño mínimo de los procesos (KB): ";
    cin >> process_min_size;
    cout << "Ingrese el tamaño máximo de los procesos (KB): ";
    cin >> process_max_size;

    auto start_time = steady_clock::now();

    while (true) {
        auto current_time = steady_clock::now();
        auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();

        // Creacion de un proceso cada 2 segundos
        if (elapsed_time % 2 == 0) {
            int process_size = randomInt(process_min_size, process_max_size); // Tamaño
            Proceso new_process(pid_counter, process_size, page_size); // Creacion del proceso
            processes.emplace(pid_counter, new_process); // Agregado del proceso a la lista de procesos
            cout << "\nCreando proceso " << pid_counter << " con tamaño " << process_size
                 << " KB (" << new_process.pages << " páginas)\n";

            // Carga de las paginas a la memoria si hay espacio o si no lo carga en swap
            for (int page = 0; page < new_process.pages; ++page) {
                if (ram.size() < physical_pages) {
                    ram.push_back({pid_counter, page});
                    cout << "-> Página (" << pid_counter << ", " << page << ") cargada en RAM.\n";
                } else if (swap_space.size() < virtual_pages - physical_pages) {
                    swap_space.push_back({pid_counter, page});
                    cout << "-> Página (" << pid_counter << ", " << page << ") cargada en Swap.\n";
                } else {
                    cout << "\n¡Memoria llena! Terminando simulación.\n";
                    return;
                }
            }
            pid_counter++;
            this_thread::sleep_for(chrono::seconds(2));
        }

        // Eventos pasados 30 segundos
        if (elapsed_time >= 1 && (elapsed_time - 30) % 5 == 0) {
            // ELiminacion de un random cada 5 segundos
            if (!processes.empty()) {
                int pid_to_kill = randomInt(1, pid_counter - 1);
                while (processes.find(pid_to_kill) == processes.end()) {
                    pid_to_kill = randomInt(1, pid_counter - 1);
                }
                cout << "\nTerminando proceso " << pid_to_kill << "\n";

                auto remove_pages = [&](list<pair<int, int>> &memory) {
                    memory.remove_if([pid_to_kill](pair<int, int> page) {
                        return page.first == pid_to_kill;
                    });
                };

                remove_pages(ram);
                remove_pages(swap_space);
                processes.erase(pid_to_kill);
            }

            // Acceso a una direccion virtual random cada 5 segundos
            if (!processes.empty()) {
                int random_pid = randomInt(1, pid_counter - 1);
                while (processes.find(random_pid) == processes.end()) {
                    random_pid = randomInt(1, pid_counter - 1);
                }
                auto &process = processes[random_pid];
                int random_page = randomInt(0, process.pages - 1);
                int virtual_address = random_page * page_size * 1024 + randomInt(0, page_size * 1024 - 1);

                cout << "\nAccediendo a dirección virtual " << virtual_address << " del proceso " << random_pid << "\n";
                cout << "Correspondiente a página " << random_page << "\n";

                auto page_pair = make_pair(random_pid, random_page);
                auto it = find(ram.begin(), ram.end(), page_pair);
                if (it != ram.end()) {
                    cout << "¡Página encontrada en RAM!\n";
                } else {
                    cout << "¡Page fault! Realizando swapping...\n";

                    // Si es que detecto que la pagina esta en swap se carga a ram
                    if (ram.size() < physical_pages) {
                        ram.push_back(page_pair);
                        swap_space.remove(page_pair);
                        cout << "-> Página " << random_page << " del proceso " << random_pid << " cargada en RAM.\n";
                    } else {
                        // Se cambia por la primera pagina de la lista de ram (FIFO)
                        auto page_to_swap = ram.front();
                        ram.pop_front();

                        if (swap_space.size() < virtual_pages - physical_pages) {
                            swap_space.push_back(page_to_swap);
                            cout << "-> Swapping página (" << page_to_swap.first << ", " << page_to_swap.second << ") de RAM a Swap.\n";
                        } else {
                            cout << "\n¡Swap lleno! Terminando simulación.\n";
                            return;
                        }

                        // cARGA la pagina a ram
                        ram.push_back(page_pair);
                        swap_space.remove(page_pair);
                        cout << "-> Página (" << random_pid << ", " << random_page << ") cargada en RAM.\n";
                    }
                }
            }
            this_thread::sleep_for(chrono::seconds(5));
        }

        // Chequeo si la memoria tanto fisica como virtual estan llenas y si lo estan termina la simulacion
        if (ram.size() == physical_pages && swap_space.size() == virtual_pages - physical_pages) {
            cout << "\n¡Memoria llena! Terminando simulación.\n";
            return;
        }

        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

int main() {
    PagSim();
    return 0;
}
