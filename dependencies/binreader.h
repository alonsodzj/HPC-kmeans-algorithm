//  EJEMPLO DE USO DE LA LIBRERÍA
/*
    reader rd;
    std::vector<float> datos = rd.leerDatos("salida.bin");  //obtengo el vector de datos.
    auto coords = rd.getNumCoords();
    auto puntos = rd.getNumPuntos();
*/

#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

class reader{
private:
    int nFilas = 0;
    int nColumnas = 0;
public:
    std::vector<float> leerDatos(const char* nombreArchivo);
    int getNumCoords();
    int getNumPuntos();
};


std::vector<float> reader::leerDatos(const char* nombreArchivo)
{
    //creo mi vector unidimiensional para optimizar rendimiento
    std::vector<float> data; 

    //intento abrir el fichero
    std::ifstream file(nombreArchivo, std::ios::binary);
    if (!file) {
        std::cerr << "Error al abrir el archivo.\n";
        return data;
    }

    // Leer cabecera
    file.read(reinterpret_cast<char*>(&nFilas), sizeof(int));
    file.read(reinterpret_cast<char*>(&nColumnas), sizeof(int));
    if (!file) {
        std::cerr << "Error leyendo cabecera.\n";
        return data;
    }

    // Reservar memoria (número de elementos, NO bytes)
    data.resize(nFilas * nColumnas);

    // Leer todos los floats de una sola vez
    file.read(reinterpret_cast<char*>(data.data()), nFilas * nColumnas * sizeof(float));
    if (!file) {
        std::cerr << "Error leyendo datos.\n";
        data.clear();
    }
    return data;
}

int reader::getNumCoords()
{
    return nColumnas;
}
int reader::getNumPuntos()
{
    return nFilas;
}
