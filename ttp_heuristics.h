#ifndef TTP_HEURISTICS_H
#define TTP_HEURISTICS_H

#include "base1.h"
#include <cstdlib>
#include <ctime>

// HEURÍSTICA 1: Tour secuencial + Sin recoger items
class SequentialNoItems : public TTPHeuristic {
public:
    SequentialNoItems(const TTPInstance& inst) : TTPHeuristic(inst) {}
    
    string getName() const override {
        return "Sequential Tour + No Items";
    }
    
    TTPSolution solve() override {
        TTPSolution sol;
        sol.tour = createSequentialTour();
        sol.pickingPlan = createEmptyPickingPlan();
        evaluateSolution(sol);
        return sol;
    }
};

// HEURÍSTICA 2: Vecino más cercano + Picking greedy
class NearestNeighborGreedy : public TTPHeuristic {
public:
    NearestNeighborGreedy(const TTPInstance& inst) : TTPHeuristic(inst) {}
    
    string getName() const override {
        return "Nearest Neighbor + Greedy Picking";
    }
    
    TTPSolution solve() override {
        TTPSolution sol;
        sol.tour = createNearestNeighborTour(0);
        sol.pickingPlan = createGreedyPickingPlan(sol.tour);
        evaluateSolution(sol);
        return sol;
    }
};

// HEURÍSTICA 3: Tour aleatorio + Picking greedy
class RandomTourGreedy : public TTPHeuristic {
public:
    RandomTourGreedy(const TTPInstance& inst) : TTPHeuristic(inst) {
        srand(time(0));
    }
    
    string getName() const override {
        return "Random Tour + Greedy Picking";
    }
    
    TTPSolution solve() override {
        TTPSolution sol;
        sol.tour = createRandomTour();
        sol.pickingPlan = createGreedyPickingPlan(sol.tour);
        evaluateSolution(sol);
        return sol;
    }
};

// HEURÍSTICA 4: Mejora local con 2-opt en tour
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

// HEURÍSTICA 5: Picking basado en profit absoluto
class HighProfitPicking : public TTPHeuristic {
public:
    HighProfitPicking(const TTPInstance& inst) : TTPHeuristic(inst) {}
    
    string getName() const override {
        return "Nearest Neighbor + High Profit Picking";
    }
    
    TTPSolution solve() override {
        TTPSolution sol;
        sol.tour = createNearestNeighborTour(0);
        
        // Crear picking plan basado en profit absoluto
        vector<pair<int, int>> itemsByProfit; // (profit, index)
        for (int i = 0; i < instance.num_items; i++) {
            itemsByProfit.push_back({instance.items[i].profit, i});
        }
        sort(itemsByProfit.rbegin(), itemsByProfit.rend());
        
        sol.pickingPlan = createEmptyPickingPlan();
        int currentWeight = 0;
        
        for (auto& p : itemsByProfit) {
            int itemIdx = p.second;
            if (currentWeight + instance.items[itemIdx].weight <= instance.capacity) {
                sol.pickingPlan[itemIdx] = 1;
                currentWeight += instance.items[itemIdx].weight;
            }
        }
        
        evaluateSolution(sol);
        return sol;
    }
};

#endif