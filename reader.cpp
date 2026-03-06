//  EJEMPLO DE USO DE LA LIBRERÍA
/*
    reader rd;
    std::vector<float> datos = rd.leerDatos("salida.bin");  //obtengo el vector de datos.
    auto coords = rd.getNumCoords();
    auto puntos = rd.getNumPuntos();
*/
#include "dependencies/binreader.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

int main() {
    reader rd;

    const char* archivo = "salida.bin";

    // Leer los datos y actualizar filas/columnas en la clase
    std::vector<float> datos = rd.leerDatos(archivo);

    // Verificamos que se hayan leído datos
    if (datos.empty()) {
        std::cerr << "No se pudieron leer los datos del archivo.\n";
        return 1;
    }

    // Imprimir información de filas y columnas
    std::cout << "Número de filas: " << rd.getNumPuntos() << "\n";
    std::cout << "Número de columnas: " << rd.getNumCoords() << "\n";

    // Imprimir los datos en forma de tabla
    std::cout << "Datos:\n";
    int filas = rd.getNumPuntos();
    int cols  = rd.getNumCoords();
    for (int i = 0; i < filas; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << std::fixed << std::setprecision(2) 
                      << datos[i * cols + j] << " ";
        }
        std::cout << "\n";
    }

    return 0;
}
