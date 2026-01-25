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
    
    // Agregar heurísticas a probar
    experiment.addHeuristic(new SequentialNoItems(instance));
    experiment.addHeuristic(new NearestNeighborGreedy(instance));
    experiment.addHeuristic(new RandomTourGreedy(instance));
    experiment.addHeuristic(new HighProfitPicking(instance));
    experiment.addHeuristic(new LocalSearch2Opt(instance));
    
    experiment.runAll();
    
    return 0;
}

