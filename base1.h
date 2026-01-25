#ifndef TTP_BASE_H
#define TTP_BASE_H

#include "reader.cpp"
#include <vector>
#include <string>
#include <limits>
#include <algorithm>

using namespace std;

// Estructura para almacenar una solución del TTP
struct TTPSolution {
    vector<int> tour;           // orden de visita de ciudades
    vector<int> pickingPlan;    // 1 si se recoge el item i, 0 si no
    double objective;           // valor de la función objetivo
    double profit;              // ganancia total
    double time;                // tiempo total
    int weight;                 // peso total recogido
    
    TTPSolution() : objective(-numeric_limits<double>::infinity()), 
                    profit(0), time(0), weight(0) {}
    
    bool isValid(const TTPInstance& inst) const {
        return weight <= inst.capacity && tour.size() == inst.dimension;
    }
};

// Clase base abstracta para heurísticas
class TTPHeuristic {
protected:
    const TTPInstance& instance;
    
public:
    TTPHeuristic(const TTPInstance& inst) : instance(inst) {}
    virtual ~TTPHeuristic() {}
    
    // Método principal que debe implementar cada heurística
    virtual TTPSolution solve() = 0;
    
    // Nombre de la heurística (para reportes)
    virtual string getName() const = 0;
    
    // Método auxiliar para evaluar una solución
    void evaluateSolution(TTPSolution& sol) {
        sol.profit = 0.0;
        sol.time = 0.0;
        sol.weight = 0;
        
        // Calcular ganancia y peso total
        for (int i = 0; i < instance.num_items; i++) {
            if (sol.pickingPlan[i] == 1) {
                sol.profit += instance.items[i].profit;
                sol.weight += instance.items[i].weight;
            }
        }
        
        // Calcular tiempo total del viaje
        double nu = (instance.max_speed - instance.min_speed) / instance.capacity;
        int currentWeight = 0;
        
        for (int i = 0; i < instance.dimension; i++) {
            int from = sol.tour[i];
            int to = sol.tour[(i + 1) % instance.dimension];
            
            double velocity = instance.max_speed - nu * currentWeight;
            sol.time += instance.distances[from][to] / velocity;
            
            // Actualizar peso después de visitar 'to'
            for (int k = 0; k < instance.num_items; k++) {
                if (sol.pickingPlan[k] == 1 && instance.items[k].node == to) {
                    currentWeight += instance.items[k].weight;
                }
            }
        }
        
        sol.objective = sol.profit - sol.time * instance.renting_ratio;
    }
    
    // Crear tour inicial (por ejemplo, secuencial)
    vector<int> createSequentialTour() {
        vector<int> tour(instance.dimension);
        for (int i = 0; i < instance.dimension; i++) {
            tour[i] = i;
        }
        return tour;
    }
    
    // Crear tour aleatorio
    vector<int> createRandomTour() {
        vector<int> tour = createSequentialTour();
        random_shuffle(tour.begin() + 1, tour.end()); // mantener ciudad 0 al inicio
        return tour;
    }
    
    // Crear tour usando vecino más cercano
    vector<int> createNearestNeighborTour(int start = 0) {
        vector<int> tour;
        vector<bool> visited(instance.dimension, false);
        
        int current = start;
        tour.push_back(current);
        visited[current] = true;
        
        for (int i = 1; i < instance.dimension; i++) {
            double minDist = numeric_limits<double>::infinity();
            int nearest = -1;
            
            for (int j = 0; j < instance.dimension; j++) {
                if (!visited[j] && instance.distances[current][j] < minDist) {
                    minDist = instance.distances[current][j];
                    nearest = j;
                }
            }
            
            tour.push_back(nearest);
            visited[nearest] = true;
            current = nearest;
        }
        
        return tour;
    }
    
    // Picking plan vacío (no recoger nada)
    vector<int> createEmptyPickingPlan() {
        return vector<int>(instance.num_items, 0);
    }
    
    // Picking plan greedy basado en profit/weight ratio
    vector<int> createGreedyPickingPlan(const vector<int>& tour) {
        vector<int> pickingPlan(instance.num_items, 0);
        
        // Crear lista de items ordenados por ratio profit/weight
        vector<pair<double, int>> itemRatios;
        for (int i = 0; i < instance.num_items; i++) {
            double ratio = (double)instance.items[i].profit / instance.items[i].weight;
            itemRatios.push_back({ratio, i});
        }
        sort(itemRatios.rbegin(), itemRatios.rend());
        
        // Seleccionar items mientras haya capacidad
        int currentWeight = 0;
        for (auto& p : itemRatios) {
            int itemIdx = p.second;
            if (currentWeight + instance.items[itemIdx].weight <= instance.capacity) {
                pickingPlan[itemIdx] = 1;
                currentWeight += instance.items[itemIdx].weight;
            }
        }
        
        return pickingPlan;
    }
};

// Clase para gestionar experimentos con múltiples heurísticas
class TTPExperiment {
private:
    const TTPInstance& instance;
    vector<TTPHeuristic*> heuristics;
    
public:
    TTPExperiment(const TTPInstance& inst) : instance(inst) {}
    
    ~TTPExperiment() {
        for (auto h : heuristics) {
            delete h;
        }
    }
    
    void addHeuristic(TTPHeuristic* heuristic) {
        heuristics.push_back(heuristic);
    }
    
    void runAll() {
        cout << "  EJECUTANDO EXPERIMENTOS TTP" << endl;
        cout << "Instancia: " << instance.name << endl;
        cout << "Ciudades: " << instance.dimension << endl;
        cout << "Items: " << instance.num_items << endl;
        cout << "Capacidad: " << instance.capacity << endl;
        
        TTPSolution bestSolution;
        string bestHeuristic;
        
        for (auto heuristic : heuristics) {
            cout << "► Ejecutando: " << heuristic->getName() << endl;
            
            TTPSolution solution = heuristic->solve();
            
            cout << "  Objetivo: " << solution.objective << endl;
            cout << "  Ganancia: " << solution.profit << endl;
            cout << "  Tiempo: " << solution.time << endl;
            cout << "  Peso usado: " << solution.weight << "/" << instance.capacity << endl;
            cout << "  Válida: " << (solution.isValid(instance) ? "Sí" : "No") << endl;
            cout << endl;
            
            if (solution.objective > bestSolution.objective) {
                bestSolution = solution;
                bestHeuristic = heuristic->getName();
            }
        }
        
        cout << "  MEJOR SOLUCIÓN ENCONTRADA" << endl;
        cout << "Heurística: " << bestHeuristic << endl;
        cout << "Objetivo: " << bestSolution.objective << endl;
        cout << "Ganancia: " << bestSolution.profit << endl;
        cout << "Tiempo: " << bestSolution.time << endl;
        cout << "Peso: " << bestSolution.weight << endl;
    }
};

#endif