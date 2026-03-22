#pragma once

#include <vector>
#include <iomanip>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//este es un tipo de lectura AoS en el que voy almacenando los puntos en orden no toda una coordenada y luego la siguiente

//versión con mmaping para acelerar la lectura
void leerDatos(const char* nombre_archivo, std::vector<float>& vector_datos, size_t& n_filas, size_t& n_columnas)
{
    int fd = open(nombre_archivo, O_RDONLY);
    if (fd == -1) {
        perror("Error abriendo archivo");
        return; // Salir si hay error
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("Error en fstat");
        close(fd);
        return;
    }

    // Mapeo del archivo en memoria
    void* mapped = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("Error en mmap");
        close(fd);
        return;
    }

    // Opcional: Indicar lectura secuencial para optimizar el kernel
    posix_fadvise(fd, 0, sb.st_size, POSIX_FADV_SEQUENTIAL);

    char* ptr = (char*)mapped;

    //Leo las dimensiones de mi dataset
    n_filas = *(int*)ptr;
    n_columnas = *(int*)(ptr + sizeof(int));

    //Apunto al inicio de los datos
    float* datos_raw = (float*)(ptr + 2 * sizeof(size_t));

    // 3. ASIGNAR los datos al vector de referencia (usando assign para eficiencia)
    size_t total_elementos = n_filas * n_columnas;
    vector_datos.assign(datos_raw, datos_raw + total_elementos);

    // Limpieza
    munmap(mapped, sb.st_size);
    close(fd);
}