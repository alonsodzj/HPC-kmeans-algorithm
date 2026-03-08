#include <vector>
#include <random>
#include <iostream>
#include <cstdio>
#include <iomanip>

#define DOMAIN_SIZE 200.0f
#define CLUSTER_STDDEV 30.0f

std::vector<float> randomCenter(int dims, std::mt19937 &gen)
{
    std::uniform_real_distribution<float> dist(-DOMAIN_SIZE, DOMAIN_SIZE);

    std::vector<float> c(dims);
    for (int i = 0; i < dims; i++)
        c[i] = dist(gen);

    return c;
}

std::vector<float> randomPointAround(const std::vector<float>& center,
                                      float stddev,
                                      std::mt19937 &gen)
{
    std::normal_distribution<float> dist(0.0f, stddev);

    int n = center.size();
    std::vector<float> p(n);

    for (int i = 0; i < n; i++)
        p[i] = center[i] + dist(gen);

    return p;
}

int main()
{
    int dimensiones;
    int puntosTotales;
    int clusters;

    std::cout << "Numero de coordenadas: ";
    std::cin >> dimensiones;

    std::cout << "Numero total de puntos: ";
    std::cin >> puntosTotales;

    std::cout << "Numero de clusters reales: ";
    std::cin >> clusters;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<std::vector<float>> data;
    data.reserve(puntosTotales);

    // generar centros reales
    std::vector<std::vector<float>> centers;
    for (int i = 0; i < clusters; i++)
        centers.push_back(randomCenter(dimensiones, gen));

    int puntosPorCluster = puntosTotales / clusters;

    // generar puntos alrededor de cada centro
    for (int c = 0; c < clusters; c++)
    {
        for (int i = 0; i < puntosPorCluster; i++)
            data.push_back(randomPointAround(centers[c], CLUSTER_STDDEV, gen));
    }

    FILE* f = fopen("data/salida.bin", "wb");

    if (!f)
    {
        std::cout << "Error al abrir archivo\n";
        return 1;
    }

    int nFilas = data.size();
    int nCol = dimensiones;

    fwrite(&nFilas, sizeof(int), 1, f);
    fwrite(&nCol, sizeof(int), 1, f);

    for (auto &p : data)
        fwrite(p.data(), sizeof(float), nCol, f);

    fclose(f);

    std::cout << "Dataset generado correctamente\n";
}