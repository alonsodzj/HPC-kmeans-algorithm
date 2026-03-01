//función que se va a encargar de leer los datos que he guardado dentro de mi binario con fwrite.


#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

std::vector<std::vector<float>> leerDatosBinarios(const char* nombreArchivo)
{
    std::vector<std::vector<float>> data;

    std::ifstream file(nombreArchivo, std::ios::binary);
    if (!file) {
        std::cerr << "Error al abrir el archivo.\n";
        return data;
    }

    // leo la cabecera para saber el tamaño en filas y columnas.
    int nFilas = 0;
    int nColumnas = 0;
    file.read(reinterpret_cast<char*>(&nFilas), sizeof(int)); //reinterpret cast sirve para decir oye cógeme estos bytes y hazmelos este tipo de dato.
    if (!file) {
        std::cerr << "Error leyendo número de filas.\n";
        return data;
    }

    file.read(reinterpret_cast<char*>(&nColumnas), sizeof(int));
    if (!file) {
        std::cerr << "Error leyendo número de columnas.\n";
        return data;
    }

    // Reservar memoria
    data.resize(nFilas, std::vector<float>(nColumnas));

    // Leer cada fila
    for (int i = 0; i < nFilas; ++i) {
        file.read(reinterpret_cast<char*>(data[i].data()), nColumnas * sizeof(float));
        if (!file) {
            std::cerr << "Error leyendo fila " << i << "\n";
            data.clear();
            return data;
        }
    }

    return data;
}

//función que me devuelve el número de coordenadas.
int getNumCoords(std::vector<std::vector<float>>& datos)
{
return datos[0].size();
}

//hay que considerar que fread avisa al compilador de que no tenemos en cuenta que se lean los daos correctamente.
int main()
{
    auto datos = leerDatosBinarios("salida.bin");
    std::cout << std::fixed << std::setprecision(9);
    for (size_t i = 0; i < datos.size(); ++i) {
        for (size_t j = 0; j < datos[i].size(); ++j) {
            std::cout << datos[i][j] << "\t";
        }
        std::cout << "\n";
    }
}