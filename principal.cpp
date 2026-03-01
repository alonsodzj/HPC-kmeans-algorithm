//o grupos aleatorios o centroides aleatorios

//si elejimos centroides tenemos que elegir de los puntos que hay.
//el criterio de inicialización más fácil es el de grupos aleatorios.



//principal.c
#include "Circulo.h"

#include <omp.h>
int main()
{
	Punto<3> p{3,4,5}; //lista de inicialización: sólo se puede usar si son justo
						//los miembros de la clase. TODOS
	Circulo<Punto<3>> c{p,7};
}