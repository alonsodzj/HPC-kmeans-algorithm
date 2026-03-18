# K-Means MPI+OpenMP - Decisiones de Diseño

## Índice
1. [Arquitectura General](#arquitectura-general)
2. [Decisiones de Implementación](#decisiones-de-implementación)
3. [Optimizaciones Aplicadas](#optimizaciones-aplicadas)
4. [Análisis de Rendimiento](#análisis-de-rendimiento)
5. [Limitaciones y Mejoras Futuras](#limitaciones-y-mejoras-futuras)

## Arquitectura General

### Paralelismo Híbrido MPI + OpenMP

**Decisión:** Combinar MPI (memoria distribuida) con OpenMP (memoria compartida)

**Justificación:**
- **MPI:** Permite escalabilidad horizontal entre nodos de un cluster
- **OpenMP:** Aprovecha los múltiples cores dentro de cada nodo
- **Resultado:** Mejor aprovechamiento del hardware en entornos HPC

**Alternativas descartadas:**
- **Solo MPI:** Desperdicia cores dentro de cada nodo (1 proceso por nodo subutiliza CPUs)
- **Solo OpenMP:** No escala más allá de un único nodo (limitado por memoria compartida)
- **Pthreads:** Más verboso y complejo, OpenMP es estándar en HPC

**Patrón de comunicación:** Maestro-trabajador
- Rank 0 actualiza centroides
- Broadcast distribuye centroides a todos los procesos
- Reduce agrega estadísticas globales

---

## Decisiones de Implementación

### 1. Layout de Datos

**Formato:** Vector plano (row-major)
```cpp
// Punto 0: [dim0, dim1, ..., dimN]
// Punto 1: [dim0, dim1, ..., dimN]
datos_local = [p0_d0, p0_d1, ..., p0_dN, p1_d0, p1_d1, ...]
```

**Justificación:**
- Localidad espacial en caché
- Acceso secuencial beneficia prefetching
- Compatible con vectorización SIMD

**Alternativa descartada:** Column-major (mejor para operaciones por dimensión, peor para nuestro caso)

---

### 2. Cálculo de Distancias

**Decisión:** Distancia euclidiana al cuadrado (sin `sqrt`)
```cpp
dist = Σ(punto[d] - centro[d])²  // SIN sqrt final
```

**Justificación:**
- `sqrt()` es monótona: si d1² < d2², entonces d1 < d2
- Evita ~k llamadas a `sqrt()` por punto (función muy costosa)
- **Impacto:** Reducción de 15-25% en tiempo de cómputo

**Trade-off:** Los valores de distancia no son interpretables (pero solo comparamos)

---

### 3. Sincronización de Centroides

**Decisión:** Broadcast condicional (solo después de primera iteración)
```cpp
if (iteraciones > 0) {
    MPI_Bcast(centroides.data(), ...);
}
```

**Justificación:**
- En iteración 0, todos los procesos ya tienen los centroides iniciales
- Evita comunicación innecesaria de `k × d × 4 bytes`

**Ejemplo:** k=100, d=50 → ahorra ~20KB de transferencia

---

### 4. Estrategia de Reducción MPI

**Decisión:** Dos `MPI_Reduce` separados en lugar de estructura custom
```cpp
MPI_Reduce(suma_local.data(), ...);     // Sumas parciales
MPI_Reduce(conteo_local.data(), ...);   // Conteos
```

**Justificación:**
- Más simple y portable
- Overhead de dos llamadas es despreciable vs cómputo total
- No requiere definir `MPI_Datatype` personalizado

**Alternativa descartada:** Struct custom con `MPI_Type_create_struct`
- **Pros:** Una sola comunicación
- **Contras:** Más complejo, ganancia marginal (<1% típicamente)

---

### 5. Convergencia Global

**Decisión:** `MPI_Allreduce` para contador de desplazados
```cpp
MPI_Allreduce(&desplazados_local, &desplazados_total, 1, MPI_INT, MPI_SUM, ...);
```

**Justificación:**
- Todos los procesos necesitan saber si converger
- `Allreduce` = `Reduce` + `Bcast` en una sola operación optimizada

**Alternativa:** `Reduce` + `Bcast` (2 comunicaciones en lugar de 1)

---

### 6. Criterio de Convergencia

**Decisión:** Tasa relativa de cambio
```cpp
float tasa_cambio = desplazados_total / n_filas_total;
if (tasa_cambio <= 0.001f)  // 0.1% de puntos cambian
    converge();
```

**Justificación:**
- Escala bien con datasets de cualquier tamaño
- Umbral relativo es más robusto que absoluto

**Alternativas descartadas:**
- **Umbral absoluto** (ej: <100 puntos): No escala con tamaño de dataset
- **Cambio en centroides**: Más costoso de calcular, menos intuitivo

---

### 7. Parámetros de Configuración

**Decisión:** Usar `#define` en lugar de parámetros de función
```cpp
#define MAX_ITERACIONES 2000
#define UMBRAL_CONVERGENCIA 0.001f
```

**Justificación:**
- **Simplicidad:** Interface de función más limpia
- **Constantes en tiempo de compilación:** Permite optimizaciones del compilador
- **Valores estándar:** 2000 iteraciones y 0.1% son apropiados para la mayoría de casos

**Alternativas descartadas:**
- **Parámetros de función:** Mayor flexibilidad pero interface más compleja
- **Variables globales:** Menos clara la intención, posibles problemas en linking

**Trade-off:** 
- Menor flexibilidad en runtime
- Requiere recompilación para cambiar valores
- Apropiado para parámetros que raramente cambian

---

## Optimizaciones Aplicadas

### 1. Pre-asignación de Memoria

**Optimización:** Declarar buffers fuera del bucle principal
```cpp
// ANTES (mal): 
while (...) {
    std::vector<float> suma_local(...);  // 2000 mallocs
}

// DESPUÉS (bien):
std::vector<float> suma_local(...);  // 1 malloc
while (...) {
    std::fill(suma_local.begin(), ...);
}
```

**Impacto:** Elimina ~2000 allocaciones/deallocaciones por ejecución

---

### 2. Reducción de Cálculos de Índices

**Optimización:** Punteros base calculados una vez
```cpp
// ANTES (malo):
for (size_t d = 0; d < n_columnas; ++d) {
    suma_local[mejor_centroide * n_columnas + d] += ...;  // Calcula índice base cada vez
}

// DESPUÉS (bueno):
float* suma_dest = &suma_local[mejor_centroide * n_columnas];  // Una vez
for (size_t d = 0; d < n_columnas; ++d) {
    suma_dest[d] += ...;  // Acceso directo
}
```

**Impacto:** Reduce cálculos de índices de O(k×d) a O(1) por punto

---

### 3. Vectorización SIMD

**Optimización:** Directivas `#pragma omp simd` en bucles críticos
```cpp
#pragma omp simd reduction(+:dist)
for (size_t d = 0; d < n_columnas; ++d) {
    float diff = punto[d] - centro[d];
    dist += diff * diff;
}
```

**Justificación:**
- Permite al compilador generar instrucciones vectoriales (AVX, AVX2, AVX-512)
- Procesa 4-16 elementos en paralelo por instrucción

**Impacto:** Speedup de 4-8× en el bucle de distancias (80% del tiempo total)

**Requisitos:**
- Compilador con soporte OpenMP 4.0+
- Flags: `-fopenmp-simd` (GCC/Clang) o `/Qopenmp-simd` (Intel)
- Mejor con `-march=native` para optimizar a arquitectura específica

---

### 4. Array Reductions (OpenMP 4.5+)

**Optimización:** Reducción automática de arrays completos
```cpp
#pragma omp parallel for reduction(+:suma_local[:suma_local.size()])
```

**Justificación:**
- Evita race conditions sin `#pragma omp critical` (que serializa)
- El compilador gestiona copias privadas y merge automático

**Alternativa descartada:** Reducción manual
```cpp
// Cada thread tiene su propio array
// Al final, merge manual de todos los arrays
// Más memoria, más complejidad
```

---

### 5. Multiplicación por Inverso

**Optimización:** `x * (1/n)` en lugar de `x / n`
```cpp
float inv_conteo = 1.0f / (float)conteo_global[i];
for (...) {
    centroides[...] = suma_global[...] * inv_conteo;  // Multiplicación
}
```

**Justificación:**
- Multiplicación es ~2× más rápida que división en CPU moderna
- Calcula el inverso una vez, reutiliza d veces

**Nota:** Compiladores modernos con `-O3` suelen hacer esta optimización automáticamente

---

## Análisis de Rendimiento

### Complejidad Temporal

**Por iteración:**
- **Secuencial:** O(n × k × d)
  - n: número de puntos
  - k: número de clusters
  - d: dimensionalidad

**Con MPI (p procesos):**
- **Cómputo:** O((n/p) × k × d)
- **Comunicación:** O(k × d) broadcast + O(k × d) reduce
- **Speedup ideal:** ~p (limitado por comunicación)

**Con OpenMP (t threads por proceso):**
- **Cómputo:** O((n/p×t) × k × d)
- **Speedup ideal:** ~p×t

---

### Cuellos de Botella

1. **Cálculo de distancias (80-90% del tiempo)**
   - **Mitigado:** Vectorización SIMD + OpenMP
   - **Mejora típica:** 10-20× vs versión secuencial

2. **Comunicación MPI (5-15% del tiempo)**
   - **Factor limitante:** Cuando k×d es muy grande (>1MB)
   - **Mitigación:** Reducir k, usar algoritmos distribuidos avanzados

3. **Convergencia lenta (número de iteraciones)**
   - **Depende de:** Calidad de inicialización, complejidad del dataset
   - **Mitigación:** k-means++, mini-batch k-means

---

### Escalabilidad

**Strong scaling (dataset fijo, aumentar procesos):**
- **Ideal hasta:** ~n/(k×d×100) procesos
- **Límite:** Comunicación domina cuando cada proceso tiene muy pocos puntos

**Weak scaling (dataset proporcional a procesos):**
- **Ideal:** Escala linealmente
- **Práctica:** Eficiencia >90% hasta cientos de nodos

**Ejemplo:**
- Dataset: 10M puntos, k=100, d=50
- 1 nodo (16 cores): ~30 segundos
- 10 nodos (160 cores): ~4 segundos (speedup 7.5×)
- 100 nodos: limitado por comunicación

---

## Limitaciones y Mejoras Futuras

### Limitaciones Actuales

1. **Centroides vacíos**
   - **Problema:** Si un centroide no recibe puntos, mantiene posición anterior
   - **Impacto:** Puede causar convergencia subóptima
   - **Frecuencia:** Raro en práctica con buena inicialización
   - **Solución futura:** Reinicializar con punto aleatorio o split de centroide más grande

2. **Inicialización no optimizada**
   - **Actual:** Asume centroides iniciales dados externamente
   - **Problema:** Random initialization puede requerir muchas iteraciones
   - **Solución futura:** Implementar k-means++ distribuido

3. **Sin balance de carga dinámico**
   - **Asume:** Distribución uniforme de puntos entre procesos
   - **Problema:** Si un proceso tiene más puntos, limita velocidad total
   - **Solución futura:** Redistribución adaptativa

4. **Parámetros fijos en tiempo de compilación**
   - **Limitación:** `MAX_ITERACIONES` y `UMBRAL_CONVERGENCIA` requieren recompilación para cambiar
   - **Justificación:** Valores estándar apropiados para mayoría de casos
   - **Workaround:** Modificar `#define` y recompilar si necesario

---

### Optimizaciones Avanzadas Posibles

#### 1. Triangle Inequality Pruning
**Idea:** Usar desigualdad triangular para evitar cálculos de distancia
```cpp
// Si |d(p, c1) - d(c1, c2)| > d(p, c_mejor), entonces c2 no puede ser mejor
// Ahorra ~50% de cálculos de distancia en práctica
```

**Complejidad:** Requiere matriz de distancias entre centroides O(k²)

---

#### 2. Elkan's Algorithm
**Idea:** Mantener bounds superiores e inferiores de distancias

**Beneficio:** Reduce cálculos de O(nkd) a O(nk + nd) en muchos casos

**Trade-off:** Más memoria (O(nk)), complejidad de implementación

---

#### 3. Mini-Batch K-Means
**Idea:** Cada iteración procesa solo un subset aleatorio de puntos
```cpp
size_t batch_size = std::min(n_filas_local, 1000);
// Procesar solo batch_size puntos aleatorios
```

**Beneficio:** Converge más rápido (menos iteraciones)

**Trade-off:** Calidad de clusters ligeramente inferior

---

#### 4. GPU Acceleration
**Idea:** Mover cálculo de distancias a GPU con CUDA/OpenCL

**Speedup esperado:** 10-100× para datasets grandes (n > 100K)

**Requisitos:**
- GPUs en cada nodo
- Transferencia CPU ↔ GPU debe amortizarse
- Complejidad de implementación alta

---

#### 5. Comunicación Asíncrona
**Idea:** Solapar comunicación con cómputo
```cpp
MPI_Isend(...);  // Non-blocking
// Computar mientras se envía
MPI_Wait(...);
```

**Beneficio:** Reduce tiempo de comunicación efectivo

**Aplicable cuando:** Cómputo y comunicación son comparables en tiempo

---

#### 6. K-Means++ Initialization
**Idea:** Seleccionar centroides iniciales con probabilidad proporcional a distancia²

**Beneficio:** 
- Mejor calidad de clusters
- Menos iteraciones necesarias (típicamente 5-10× menos)

**Desafío:** Implementación distribuida requiere O(k) passes sobre datos

---

## Configuración Recomendada

### Compilación
```bash
# GCC/Clang
mpicxx -O3 -march=native -fopenmp -fopenmp-simd kmeans.cpp

# Intel Compiler (mejor vectorización)
mpiicpc -O3 -xHost -qopenmp -qopt-report=5 kmeans.cpp
```

puedo probar a ejecutar de esta manera
export OMP_NUM_THREADS=4
mpirun -np 2 --map-by node:PE=4 ./main_mpi

export OMP_NUM_THREADS=4
mpirun -np 2 --map-by node:PE=4 --bind-to none ./main_mpi

**Flags importantes:**
- `-O3`: Optimizaciones agresivas
- `-march=native` / `-xHost`: Optimiza para CPU específica
- `-fopenmp-simd`: Habilita vectorización SIMD
- `-qopt-report=5` (Intel): Reporte de vectorización

---

### Ejecución
```bash
# 4 nodos, 8 procesos MPI por nodo, 4 threads OpenMP por proceso
mpirun -np 32 --map-by node:PE=4 ./kmeans

# Variables de entorno
export OMP_NUM_THREADS=4
export OMP_PROC_BIND=close
export OMP_PLACES=cores
```

**Guía de configuración:**
- **Procesos MPI:** 1-2 por socket CPU
- **Threads OpenMP:** Cores por proceso (típicamente 4-16)
- **Total cores usados:** MPI_processes × OMP_threads

---

### Modificar Parámetros

Para cambiar los parámetros del algoritmo, editar el header:
```cpp
// En kmeans_mpi.hpp
#define MAX_ITERACIONES 5000      // Aumentar para datasets complejos
#define UMBRAL_CONVERGENCIA 0.0001f  // Reducir para mayor precisión
```

Luego recompilar:
```bash
make clean && make
```

---

### Análisis de Rendimiento
```bash
# Profiling con Intel VTune
amplxe-cl -collect hotspots -- mpirun -np 4 ./kmeans

# Tracing MPI con TAU
mpirun -np 4 tau_exec ./kmeans

# Verificar vectorización
gcc -O3 -march=native -fopenmp -fopt-info-vec-optimized kmeans.cpp





mpicxx -O3 -march=znver2 -mtune=znver2        -fopenmp -fopenmp-simd        -ffast-math        -funroll-loops        -std=c++11        main_mpi.cpp        -o kmeans_mpi

mpirun -np 4 --bind-to core --map-by core ./kmeans_mpi