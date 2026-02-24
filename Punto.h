//Punto.h
#include <vector>
//la clave es que puedo tener puntos de las dimensiones que quiera.
//en el caso de que tenga puntos de muchas dimensiones se compilará la platilla específica.
template<int nDimensiones>
class Punto 
{
private:
    //atributos y funcinoes privadas
    //este es codigo que pone el
    std::vector<float> coords(nDimensiones); //creo un vector de floats con el numero de dimensiones
	
public:
    Punto() //entiendo que esto hace el constructor vacío.
    double coords[nCoords]; //array se guarda en la pila
    vector operator+(vector v)
    {
        // #pragma unroll // el compilador lo que hace es eliminar el bucle para mejorar el rendimiento
        //pondría las cuatro instrucciones de suma seguidas
        /*
        v.coords[i]+=coords[i];
        v.coords[i]+=coords[i];
        v.coords[i]+=coords[i];
        v.coords[i]+=coords[i];
        ... y así de esta manera
        */
        //se intentan evitar los bucles cortos para evitar los problemas derivados de los saltos en ensamblador
        //tenemos la copia de la dirección de la instruccióna la que saltar en caché.
        for(int i = 0; i< nCoords ; i++)
        {
            v.coords[i]+=coords[i];
        }
        return v;
    }

    template<int I = 0>
    std::vector operator+=(const std::vector& v){
        v.coords[I]+=coords[I];
        return template operator+=<I+1>(v);
    }
    template<> //para hacer una especialización
    //ahora especifico que caos quiero
    void operator+=<nCorods-1>(const vector& v){
        v.coords[I]+=coords[I];
        return template operator+=<I+1>(v);
    }
    //en tiempo de compilación se compila la plantilla pero no las funciones porque las usaremos en
    //tiempo de ejecución.

    //esta clase no se compila hasta que nCoords tenga un valor

};