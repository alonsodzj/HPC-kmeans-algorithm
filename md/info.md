COMPILACIÓN
mpicxx -O3 -march=znver2 -mtune=znver2        -fopenmp -fopenmp-simd        -ffast-math        -funroll-loops        -std=c++11        main_mpi.cpp        -o kmeans_mpi



EJECUCIÓN
# Opción A: 2 procesos MPI, 6 threads cada uno
OMP_NUM_THREADS=6 mpirun -np 2 ./kmeans_mpi

# Opción B: 3 procesos MPI, 4 threads cada uno
OMP_NUM_THREADS=4 mpirun -np 3 ./kmeans_mpi

# Opción C: 6 procesos MPI, 2 threads cada uno
OMP_NUM_THREADS=2 mpirun -np 6 ./kmeans_mpi

# Opción D: 1 proceso MPI, 12 threads (sin MPI real, solo OpenMP)
OMP_NUM_THREADS=12 mpirun -np 1 ./kmeans_mpi