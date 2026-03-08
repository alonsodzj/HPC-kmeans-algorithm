//  EJEMPLO DE USO DE LA LIBRERÍA
/*
    reader rd;
    std::vector<float> datos = rd.leerDatos("salida.bin");  //obtengo el vector de datos.
    auto coords = rd.getNumCoords();
    auto puntos = rd.getNumPuntos();
*/
#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

class reader{
private:
    int nFilas = 0;
    int nColumnas = 0;
public:
    std::vector<float> leerDatos(const char* nombreArchivo);
    int getNumCoords();
    int getNumPuntos();
};

//versión con mmaping para acelerar la subida de archivos que pesan mucho.
std::vector<float> reader::leerDatos(const char* nombreArchivo)
{
    int fd = open(nombreArchivo, O_RDONLY);
    if (fd == -1) {
        std::cerr << "Error abriendo archivo\n";
        return {};
    }

    posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);

    struct stat sb;
    fstat(fd, &sb);

    void* mapped = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        std::cerr << "Error en mmap\n";
        close(fd);
        return {};
    }

    char* ptr = (char*)mapped;

    nFilas = *(int*)ptr;
    nColumnas = *(int*)(ptr + sizeof(int));

    float* datos = (float*)(ptr + 2*sizeof(int));

    std::vector<float> data(datos, datos + nFilas*nColumnas);

    munmap(mapped, sb.st_size);
    close(fd);

    return data;
}

/*
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
*/

int reader::getNumCoords()
{
    return nColumnas;
}
int reader::getNumPuntos()
{
    return nFilas;
}
