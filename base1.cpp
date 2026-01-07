#include <iostream>
#include <vector>

using namespace std;

int main() {

    // número de ciudades
    int n;
    cin >> n;
    
    // matriz de distancias D[i][j]
    vector<vector<double>> D(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            double dist;
            cin >> dist;
            D[i][j] = dist;
            D[j][i] = dist; 
        }
    }
    
    // número de objetos
    int m;
    cin >> m;
    
    // atributos de los objetos
    vector<int> w(m);      // w[k] = peso del objeto k
    vector<int> b(m);      // b[k] = beneficio del objeto k
    vector<int> a(m);      // a[k] = ciudad donde está el objeto k
    
    //datos de cada objeto
    for (int k = 0; k < m; k++) {
        cout << "Objeto " << k << ":\n";
        cout << "  Peso: ";
        cin >> w[k];
        cout << "  Beneficio: ";
        cin >> b[k];
        cout << "  Ciudad (0 a " << n-1 << "): ";
        cin >> a[k];
    }
    
    // parámetros de la mochila y velocidad
    int Q;          // capacidad máxima de la mochila
    double v_max;   // velocidad máxima (mochila vacía)
    double v_min;   // velocidad mínima (mochila llena)
    double R;       // costo de alquiler por unidad de tiempo
    
    cin >> Q;
    cin >> v_max;
    cin >> v_min;
    cin >> R;
    
    // Coeficiente de reducción de velocidad
    double v = (v_max - v_min) / Q;
    
    vector<int> x(n);

    vector<int> z(m, 0); 
    
    
    
    return 0;
}