# Guía de Práctica: Algoritmo K-medias y Análisis Estadístico (MPI + OpenMP)

Este documento detalla el desarrollo de una aplicación de alto rendimiento para el agrupamiento de datos mediante el algoritmo **k-medias** y el cálculo de estadísticas descriptivas, combinando paradigmas de **memoria distribuida (MPI)** y **memoria compartida (OpenMP)**.

## 1. Objetivos de la Práctica

- Implementar el algoritmo de agrupamiento **k-medias**.
- Calcular estadísticas por columna: **Media, Mínimo, Máximo y Varianza**.
- Optimizar el rendimiento mediante una estrategia híbrida:
  - **OpenMPI:** Para la comunicación entre nodos (memoria distribuida).
  - **OpenMP:** Para la paralelización interna en cada nodo (memoria compartida).

## 2. Formato de los Datos de Entrada

El programa debe procesar archivos binarios con la siguiente estructura:

| Tipo de Dato  | Descripción                          | Tamaño      |
| **UINT32**    | Número de filas                      | 4 bytes     |
| **UINT32**    | Número de columnas                   | 4 bytes     |
| **REAL32**    | Valores de la matriz (fila por fila) | 4 bytes c/u |

> **Nota sobre lectura de archivos:**  
> Para archivos binarios, se recomienda usar `fread` en C o el método `read()` de `ifstream` en C++, ya que permiten volcar grandes bloques de memoria de forma eficiente, a diferencia de `>>` (operador de extracción) que es para texto.

---

## 3. Algoritmo K-medias

### A. Inicialización

- **Distribución inicial:** Se dividen los datos de forma equitativa basándose en su índice.
- **Cálculo de centroides iniciales:** Cada nodo calcula el centroide del grupo de puntos que le ha sido asignado inicialmente.

### B. Proceso Iterativo

1. **Actualización de Grupos:** Cada nodo calcula la distancia de sus puntos a todos los centroides actuales. El punto se asigna al centroide con la **mínima distancia** (se puede usar la distancia al cuadrado para ahorrar el cálculo de la raíz cuadrada).

2. **Comunicación:** Si un punto cambia de grupo, debe notificarse o enviarse al nodo responsable. Todos los nodos deben conocer la ubicación/valor de los nuevos centroides para la siguiente iteración.

3. **Actualización de Centroides:** El centroide se recalcula como el valor medio de cada columna de todos los puntos pertenecientes a ese grupo.

### C. Criterios de Finalización

El algoritmo se detiene si se cumple cualquiera de estas condiciones:

- **Convergencia:** El número total de puntos que cambian de grupo es **menor al 5%** del total de datos.
- **Límite de iteraciones:** Se alcanzan las **2000 iteraciones**.

---

## 4. Análisis Estadístico de Datos

Además del clustering, se deben calcular de forma paralela las siguientes métricas para cada columna (coordenada):

- **Mínimo/Máximo:** Valor menor y mayor de la columna.
- **Media:** 
- **Varianza:** 

## 5. Estrategia de Paralelización (Híbrida)

Para obtener el máximo rendimiento, se deben distinguir dos niveles de grano:

### Grano Grueso (OpenMPI)

- **División de datos:** Repartir el volumen total de filas entre los nodos disponibles.
- **Sincronización:** Uso de barreras o funciones colectivas (como `MPI_Allreduce` o `MPI_Bcast`) para compartir los centroides actualizados y los contadores de puntos desplazados.

### Grano Fino (OpenMP)

- **Cálculo de distancias:** Paralelizar el bucle interno que calcula la distancia de cada punto a los centroides utilizando `#pragma omp parallel for`.
- **Reducciones:** Utilizar cláusulas `reduction` para calcular mínimos, máximos y sumas locales antes de la comunicación MPI.


## 6. Ejemplo de Ejecución (1000 filas, 3 columnas, 4 nodos)

1. **Carga:**         Cada nodo recibe 250 filas (0-249 el nodo 0, 250-499 el nodo 1, etc.).
2. **Cálculo Local:** Cada nodo calcula el centroide de sus 250 puntos.
3. **Broadcast:**     Los nodos comparten sus centroides para que todos tengan la lista completa.
4. **Reasignación:**  Cada nodo verifica si sus puntos están más cerca de un centroide ajeno.
5. **Iteración:**     Se repite hasta cumplir el criterio de parada.


# Seleccionar y justificar las estrategias de paralelización.(Grano fino y grano grueso)
- Metodología híbrida entre OPENMP Y MPI


# conjunto de centroides -> media de cada coordenada
# conjutno de grupos de puntos que intercambian puntos entre sus iteraciones.

