#include <iostream>
#include <vector>
#include <random>   // For random number generation
#include <chrono>   // For seeding the random number generator

// Define Map as a vector of vectors of integers.
// You can change 'int' to whatever type best represents your cells (e.g., char, bool).
using Map = std::vector<std::vector<int>>;

/**
 * @brief Prints the map (matrix) to the console.
 * @param map The map to print.
 */
void printMap(const Map& map) {
    std::cout << "--- Current Map ---" << std::endl;
    for (const auto& row : map) {
        for (int cell : row) {
            // Adapt this to represent your cells meaningfully (e.g., ' ' for empty, '#' for occupied).
            std::cout << cell << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "-------------------" << std::endl;
}

/**
 * @brief Function to implement the Cellular Automata logic.
 * It should take a map and return the updated map after one iteration.
 * @param currentMap The map in its current state.
 * @param W Width of the map.
 * @param H Height of the map.
 * @param R Radius of the neighbor window (e.g., 1 for 3x3, 2 for 5x5).
 * @param U Threshold to decide if the current cell becomes 1 or 0.
 * @return The map after applying the cellular automata rules.
 */
Map cellularAutomata(Map currentMap, int W, int H, int R, double U) {
    // Primera pasada: calcular el nuevo estado y guardarlo en el bit 1
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            int count = 0;
            for (int dx = -R; dx <= R; ++dx) {
                for (int dy = -R; dy <= R; ++dy) {
                    int ni = i + dx;
                    int nj = j + dy;
                    if (ni >= 0 && ni < H && nj >= 0 && nj < W) {
                        count += currentMap[ni][nj] & 1;  // Leer solo el bit 0 (valor original)
                    } else {
                        count += 1; // Bordes se consideran como 1
                    }
                }
            }

            int total = (2 * R + 1) * (2 * R + 1);
            double ratio = static_cast<double>(count) / total;
            int newVal = (ratio >= U) ? 1 : 0;

            // Guardar el nuevo valor en el bit 1 (sin tocar el valor original en el bit 0)
            currentMap[i][j] |= (newVal << 1);
        }
    }

    // Segunda pasada: actualizar el estado definitivo (bit 1 -> bit 0)
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            currentMap[i][j] = (currentMap[i][j] >> 1) & 1; // Solo conservar el nuevo valor
        }
    }

    return currentMap;
}

/**
 * @brief Function to implement the Drunk Agent logic.
 * It should take a map and parameters controlling the agent's behavior,
 * then return the updated map after the agent performs its actions.
 *
 * @param currentMap The map in its current state.
 * @param W Width of the map.
 * @param H Height of the map.
 * @param J The number of times the agent "walks" (initiates a path).
 * @param I The number of steps the agent takes per "walk".
 * @param roomSizeX Max width of rooms the agent can generate.
 * @param roomSizeY Max height of rooms the agent can generate.
 * @param probGenerateRoom Probability (0.0 to 1.0) of generating a room at each step.
 * @param probIncreaseRoom If no room is generated, this value increases probGenerateRoom.
 * @param probChangeDirection Probability (0.0 to 1.0) of changing direction at each step.
 * @param probIncreaseChange If direction is not changed, this value increases probChangeDirection.
 * @param agentX Current X position of the agent (updated by reference).
 * @param agentY Current Y position of the agent (updated by reference).
 * @return The map after the agent's movements and actions.
 */

Map drunkAgent(const Map& currentMap, int W, int H, int J, int I, int roomSizeX, int roomSizeY,
               double probGenerateRoom, double probIncreaseRoom,
               double probChangeDirection, double probIncreaseChange,
               int& agentX, int& agentY) {
    Map newMap = currentMap; // The new map is a copy of the current one

    // Inicialización del generador de números aleatorios
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<double> chance(0.0, 1.0); // Para decisiones probabilísticas
    std::uniform_int_distribution<int> dirDist(0, 3); // Para escoger direcciones aleatorias

    int dx = 0, dy = 1; // Dirección inicial: hacia la derecha

    for (int j = 0; j < J; ++j) {
        
        for (int i = 0; i < I; ++i) {
            // Marcar la posicion actual del agente en el mapa
            if (agentX >= 0 && agentX < H && agentY >= 0 && agentY < W)
                newMap[agentX][agentY] = 1;

            // Calcular nueva posicion
            int newX = agentX + dx;
            int newY = agentY + dy;

            // Si la nueva posicion es valida, mover al agente
            if (newX >= 0 && newX < H && newY >= 0 && newY < W) {
                agentX = newX;
                agentY = newY;
            } else {
                // Si se choca con el borde del mapa, cambiar dirección aleatoriamente
                int dir = dirDist(rng);
                dx = (dir == 0) ? -1 : (dir == 1) ? 1 : 0;
                dy = (dir == 2) ? -1 : (dir == 3) ? 1 : 0;
                continue; // Saltar al siguiente paso sin intentar moverse mas
            }

            // Cambiar direccion con cierta probabilidad
            if (chance(rng) < probChangeDirection) {
                int dir = dirDist(rng);
                dx = (dir == 0) ? -1 : (dir == 1) ? 1 : 0;
                dy = (dir == 2) ? -1 : (dir == 3) ? 1 : 0;
                probChangeDirection = 0.2; // Reiniciar probabilidad de cambio
            } else {
                probChangeDirection += probIncreaseChange; // Incrementar probabilidad de cambio
            }
        }

        // Intentar generar una habitación con cierta probabilidad
        if (chance(rng) < probGenerateRoom) {
            int halfX = roomSizeX / 2;
            int halfY = roomSizeY / 2;
            // Dibujar una habitación centrada en la posicion del agente
            for (int dxRoom = -halfX; dxRoom <= halfX; ++dxRoom) {
                for (int dyRoom = -halfY; dyRoom <= halfY; ++dyRoom) {
                    int rx = agentX + dxRoom;
                    int ry = agentY + dyRoom;
                    // Verificar que este dentro del mapa
                    if (rx >= 0 && rx < H && ry >= 0 && ry < W) {
                        newMap[rx][ry] = 1;
                    }
                }
            }
            probGenerateRoom = 0.1; // Reiniciar probabilidad de generar habitacion
        } else {
            probGenerateRoom += probIncreaseRoom; // Incrementar probabilidad
        }
    }

    return newMap; // Devolver el nuevo mapa generado
}


int main() {
    std::cout << "--- CELLULAR AUTOMATA AND DRUNK AGENT SIMULATION ---" << std::endl;

    // --- Initial Map Configuration ---
    int mapRows = 10;
    int mapCols = 20;
    Map myMap(mapRows, std::vector<int>(mapCols, 0)); // Map initialized with zeros

    // TODO: IMPLEMENTATION GOES HERE: Initialize the map with some pattern or initial state.
    // For example, you might set some cells to 1 for the cellular automata
    // or place the drunk agent at a specific position.

    // Drunk Agent's initial position
    int drunkAgentX = mapRows / 2;
    int drunkAgentY = mapCols / 2;
    // If your agent modifies the map at start, you could do it here:
    // myMap[drunkAgentX][drunkAgentY] = 2; // Assuming '2' represents the agent

    std::cout << "\nInitial map state:" << std::endl;
    printMap(myMap);

    // --- Simulation Parameters ---
    int numIterations = 5; // Number of simulation steps

    // Cellular Automata Parameters
    int ca_W = mapCols;
    int ca_H = mapRows;
    int ca_R = 1;      // Radius of neighbor window
    double ca_U = 0.5; // Threshold

    // Drunk Agent Parameters
    int da_W = mapCols;
    int da_H = mapRows;
    int da_J = 5;      // Number of "walks"
    int da_I = 10;     // Steps per walk
    int da_roomSizeX = 5;
    int da_roomSizeY = 3;
    double da_probGenerateRoom = 0.1;
    double da_probIncreaseRoom = 0.05;
    double da_probChangeDirection = 0.2;
    double da_probIncreaseChange = 0.03;


    // --- Main Simulation Loop ---
    for (int iteration = 0; iteration < numIterations; ++iteration) {
        std::cout << "\n--- Iteration " << iteration + 1 << " ---" << std::endl;

        // TODO: IMPLEMENTATION GOES HERE: Call the Cellular Automata and/or Drunk Agent functions.
        // The order of calls will depend on how you want them to interact.

        // Example: First the cellular automata, then the agent
        myMap = cellularAutomata(myMap, ca_W, ca_H, ca_R, ca_U);
        myMap = drunkAgent(myMap, da_W, da_H, da_J, da_I, da_roomSizeX, da_roomSizeY,
                           da_probGenerateRoom, da_probIncreaseRoom,
                           da_probChangeDirection, da_probIncreaseChange,
                           drunkAgentX, drunkAgentY);

        printMap(myMap);

        // You can add a delay to visualize the simulation step by step
        // #include <thread> // For std::this_thread::sleep_for
        // #include <chrono> // For std::chrono::milliseconds
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "\n--- Simulation Finished ---" << std::endl;
    return 0;
}