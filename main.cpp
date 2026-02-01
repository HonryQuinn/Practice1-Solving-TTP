#include "reader.cpp"
#include "base1.h"
#include "ttp_heuristics.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <archivo_ttp>" << endl;
        return 1;
    }
    
    // Leer instancia del problema
    TTPInstance instance;
    if (!readTTPFile(argv[1], instance)) {
        return 1;
    }
    
    // Mostrar información de la instancia
    printInstanceInfo(instance);
    
    // Crear experimento
    TTPExperiment experiment(instance);
    
    
    // Heurística D: 2-Opt Local Search (original)
    experiment.addHeuristic(new LocalSearch2Opt(instance));
    
    // Nueva heurística probabilística con diferentes temperaturas
    // Temperature = 0.3 (más determinístico, similar a NN clásico)
    experiment.addHeuristic(new ProbabilisticNearestNeighbor2Opt(instance, 0.3));
    
    // Temperature = 0.5 (balance, valor por defecto)
    experiment.addHeuristic(new ProbabilisticNearestNeighbor2Opt(instance, 0.5));
    
    // Temperature = 1.0 (más aleatorio, mayor exploración)
    experiment.addHeuristic(new ProbabilisticNearestNeighbor2Opt(instance, 1.0));
    
    // Temperature = 2.0 (muy aleatorio, máxima exploración)
    experiment.addHeuristic(new ProbabilisticNearestNeighbor2Opt(instance, 2.0));
    
    // experiment.addHeuristic(new SequentialNoItems(instance));
    // experiment.addHeuristic(new NearestNeighborGreedy(instance));
    // experiment.addHeuristic(new RandomTourGreedy(instance));
    // experiment.addHeuristic(new HighProfitPicking(instance));
    
    experiment.runAll();
    
    return 0;
}