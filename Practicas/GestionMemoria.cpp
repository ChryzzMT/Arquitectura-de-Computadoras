#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

#define TAMANO 1000
#define TAMANO2 100

int main() {

    int arreglo[TAMANO];
    int segundo[TAMANO2];


    for (int i = 0; i < TAMANO; i++) {
        arreglo[i] = i + 1;
    }

    ofstream archivoSalida("datos.bin", ios::binary);

    if (!archivoSalida) {
        cout << "Error al abrir el archivo para escritura." << endl;
        return 1;
    }
    
    archivoSalida.write(
        reinterpret_cast<char*>(arreglo),
        TAMANO * sizeof(int)
    );

    archivoSalida.close();
    
    memset(arreglo, 0, TAMANO * sizeof(int));

    ifstream archivoEntrada("datos.bin", ios::binary);

    if (!archivoEntrada) {
        cout << "Error al abrir el archivo para lectura." << endl;
        return 1;
    }

    archivoEntrada.read(
        reinterpret_cast<char*>(arreglo),
        TAMANO * sizeof(int)
    );

    archivoEntrada.close();
    
    cout << "Primeros 5 elementos antes del traslado:" << endl;

    for (int i = 0; i < 5; i++) {
        cout << "arreglo[" << i << "] = " << arreglo[i] << endl;
    }

    cout << "\nUltimos 5 elementos antes del traslado:" << endl;

    for (int i = TAMANO - 5; i < TAMANO; i++) {
        cout << "arreglo[" << i << "] = " << arreglo[i] << endl;
    }

    memcpy(
        segundo,
        &arreglo[500],
        TAMANO2 * sizeof(int)
    );

    memset(
        &arreglo[500],
        0,
        TAMANO2 * sizeof(int)
    );

    cout << "\nElementos transferidos al segundo arreglo:" << endl;

    for (int i = 0; i < TAMANO2; i++) {
        cout << "segundo[" << i << "] = " << segundo[i] << endl;
    }

    bool correcto = true;
    for (int i = 500; i < 600; i++) {
        if (arreglo[i] != 0) {
            correcto = false;
            break;
        }
    }

    cout << "\nValidacion:" << endl;

    if (correcto) {
        cout << "Correcto: los indices 500 al 599 "
             << "del arreglo original contienen ceros." << endl;
    } else {
        cout << "Error: algunos elementos no fueron "
             << "establecidos en cero." << endl;
    }

    return 0;
}


    return 0;
}
