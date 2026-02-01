#ifndef TTP_HEURISTICS_H
#define TTP_HEURISTICS_H

#include "base1.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

// /*
// // HEURÍSTICA A: Tour secuencial + Sin recoger items
// class SequentialNoItems : public TTPHeuristic {
// public:
//     SequentialNoItems(const TTPInstance& inst) : TTPHeuristic(inst) {}
//     
//     string getName() const override {
//         return "Sequential Tour + No Items";
//     }
//     
//     TTPSolution solve() override {
//         TTPSolution sol;
//         sol.tour = createSequentialTour();
//         sol.pickingPlan = createEmptyPickingPlan();
//         evaluateSolution(sol);
//         return sol;
//     }
// };
// */

// /*
// // HEURÍSTICA B: Vecino más cercano + Picking greedy
// class NearestNeighborGreedy : public TTPHeuristic {
// public:
//     NearestNeighborGreedy(const TTPInstance& inst) : TTPHeuristic(inst) {}
//     
//     string getName() const override {
//         return "Nearest Neighbor + Greedy Picking";
//     }
//     
//     TTPSolution solve() override {
//         TTPSolution sol;
//         sol.tour = createNearestNeighborTour(0);
//         sol.pickingPlan = createGreedyPickingPlan(sol.tour);
//         evaluateSolution(sol);
//         return sol;
//     }
// };
// */

// /*
// // HEURÍSTICA C: Tour aleatorio + Picking greedy
// class RandomTourGreedy : public TTPHeuristic {
// public:
//     RandomTourGreedy(const TTPInstance& inst) : TTPHeuristic(inst) {
//         srand(time(0));
//     }
//     
//     string getName() const override {
//         return "Random Tour + Greedy Picking";
//     }
//     
//     TTPSolution solve() override {
//         TTPSolution sol;
//         sol.tour = createRandomTour();
//         sol.pickingPlan = createGreedyPickingPlan(sol.tour);
//         evaluateSolution(sol);
//         return sol;
//     }
// };
// */


// HEURÍSTICA D (ACTIVA): Mejora local con 2-opt en tour

class LocalSearch2Opt : public TTPHeuristic {
private:
    bool improve2Opt(TTPSolution& sol) {
        bool improved = false;
        int n = sol.tour.size();
        
        for (int i = 1; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                // Hacer swap 2-opt
                reverse(sol.tour.begin() + i, sol.tour.begin() + j + 1);
                
                double oldObj = sol.objective;
                evaluateSolution(sol);
                
                if (sol.objective > oldObj) {
                    improved = true;
                } else {
                    // Revertir si no mejoró
                    reverse(sol.tour.begin() + i, sol.tour.begin() + j + 1);
                    sol.objective = oldObj;
                }
            }
        }
        return improved;
    }
    
public:
    LocalSearch2Opt(const TTPInstance& inst) : TTPHeuristic(inst) {}
    
    string getName() const override {
        return "2-Opt Local Search + Greedy Picking";
    }
    
    TTPSolution solve() override {
        TTPSolution sol;
        sol.tour = createNearestNeighborTour(0);
        sol.pickingPlan = createGreedyPickingPlan(sol.tour);
        evaluateSolution(sol);
        
        // Aplicar mejora local
        int iterations = 0;
        while (improve2Opt(sol) && iterations < 100) {
            iterations++;
            // Recalcular picking plan después de cambiar tour
            sol.pickingPlan = createGreedyPickingPlan(sol.tour);
            evaluateSolution(sol);
        }
        
        return sol;
    }
};

// /*
// // HEURÍSTICA E: Picking basado en profit absoluto
// class HighProfitPicking : public TTPHeuristic {
// public:
//     HighProfitPicking(const TTPInstance& inst) : TTPHeuristic(inst) {}
//     
//     string getName() const override {
//         return "Nearest Neighbor + High Profit Picking";
//     }
//     
//     TTPSolution solve() override {
//         TTPSolution sol;
//         sol.tour = createNearestNeighborTour(0);
//         
//         // Crear picking plan basado en profit absoluto
//         vector<pair<int, int>> itemsByProfit; // (profit, index)
//         for (int i = 0; i < instance.num_items; i++) {
//             itemsByProfit.push_back({instance.items[i].profit, i});
//         }
//         sort(itemsByProfit.rbegin(), itemsByProfit.rend());
//         
//         sol.pickingPlan = createEmptyPickingPlan();
//         int currentWeight = 0;
//         
//         for (auto& p : itemsByProfit) {
//             int itemIdx = p.second;
//             if (currentWeight + instance.items[itemIdx].weight <= instance.capacity) {
//                 sol.pickingPlan[itemIdx] = 1;
//                 currentWeight += instance.items[itemIdx].weight;
//             }
//         }
//         
//         evaluateSolution(sol);
//         return sol;
//     }
// };
// */

// ============================================================================
// NUEVA HEURÍSTICA: Probabilistic Nearest Neighbor + 2-Opt Local Search
// ============================================================================
// 
// Esta heurística mejora el Nearest Neighbor tradicional usando selección
// probabilística basada en las distancias. En lugar de siempre elegir la
// ciudad más cercana, hay una probabilidad controlada de elegir ciudades
// más lejanas, lo que permite explorar diferentes regiones del espacio de
// soluciones y potencialmente encontrar mejores tours.
//
// Parámetros clave:
// - temperature: Controla la "aleatoriedad" de la selección
//   * temperature alto (>1.0): Más probabilidad de elegir ciudades lejanas
//   * temperature bajo (<1.0): Más determinístico (similar a NN clásico)
//   * temperature = 0.5 (por defecto): Balance razonable
//
// Fórmula de probabilidad (Softmax con temperatura):
//   probability(ciudad_i) = exp(-distance_i / temperature) / Σ(exp(-distance_j / temperature))
//
// Ciudades más cercanas tienen mayor probabilidad, pero las lejanas también
// tienen chance, permitiendo escapar de óptimos locales.
// ============================================================================

class ProbabilisticNearestNeighbor2Opt : public TTPHeuristic {
private:
    double temperature;  // Parámetro para controlar la aleatoriedad
    
    // Crear tour usando selección probabilística basada en distancias
    vector<int> createProbabilisticNearestNeighborTour(int start = 0) {
        vector<int> tour;
        vector<bool> visited(instance.dimension, false);
        
        int current = start;
        tour.push_back(current);
        visited[current] = true;
        
        for (int i = 1; i < instance.dimension; i++) {
            // Recopilar ciudades no visitadas y sus distancias
            vector<int> candidates;
            vector<double> distances;
            
            for (int j = 0; j < instance.dimension; j++) {
                if (!visited[j]) {
                    candidates.push_back(j);
                    distances.push_back(instance.distances[current][j]);
                }
            }
            
            // Calcular probabilidades usando Softmax con temperatura
            // P(i) = exp(-dist_i / T) / Σ exp(-dist_j / T)
            // Distancias más cortas → mayor probabilidad
            vector<double> probabilities;
            double sumExp = 0.0;
            
            for (double dist : distances) {
                double expValue = exp(-dist / temperature);
                probabilities.push_back(expValue);
                sumExp += expValue;
            }
            
            // Normalizar probabilidades
            for (double& prob : probabilities) {
                prob /= sumExp;
            }
            
            // Selección por ruleta (roulette wheel selection)
            double randValue = ((double)rand() / RAND_MAX);
            double cumulative = 0.0;
            int selectedIdx = 0;
            
            for (int k = 0; k < probabilities.size(); k++) {
                cumulative += probabilities[k];
                if (randValue <= cumulative) {
                    selectedIdx = k;
                    break;
                }
            }
            
            int nextCity = candidates[selectedIdx];
            tour.push_back(nextCity);
            visited[nextCity] = true;
            current = nextCity;
        }
        
        return tour;
    }
    
    // Mejora 2-opt (igual que LocalSearch2Opt)
    bool improve2Opt(TTPSolution& sol) {
        bool improved = false;
        int n = sol.tour.size();
        
        for (int i = 1; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                reverse(sol.tour.begin() + i, sol.tour.begin() + j + 1);
                
                double oldObj = sol.objective;
                evaluateSolution(sol);
                
                if (sol.objective > oldObj) {
                    improved = true;
                } else {
                    reverse(sol.tour.begin() + i, sol.tour.begin() + j + 1);
                    sol.objective = oldObj;
                }
            }
        }
        return improved;
    }
    
public:
    // Constructor con temperatura ajustable
    ProbabilisticNearestNeighbor2Opt(const TTPInstance& inst, double temp = 0.5) 
        : TTPHeuristic(inst), temperature(temp) {
        srand(time(0));
    }
    
    string getName() const override {
        return "Probabilistic Nearest Neighbor + 2-Opt (T=" + 
               to_string(temperature) + ")";
    }
    
    TTPSolution solve() override {
        TTPSolution sol;
        
        // FASE 1: Construcción probabilística del tour
        sol.tour = createProbabilisticNearestNeighborTour(0);
        
        // FASE 2: Picking greedy inicial
        sol.pickingPlan = createGreedyPickingPlan(sol.tour);
        evaluateSolution(sol);
        
        // FASE 3: Mejora con 2-opt
        int iterations = 0;
        while (improve2Opt(sol) && iterations < 100) {
            iterations++;
            // Recalcular picking después de mejorar tour
            sol.pickingPlan = createGreedyPickingPlan(sol.tour);
            evaluateSolution(sol);
        }
        
        return sol;
    }
    
    double getTemperature() const {
        return temperature;
    }
};

#endif