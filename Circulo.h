//Circulo.h
template<class Punto>
class Circulo
{
private:
    //atributos y funciones privadas.
	Punto centro;
	float radio;

public:
    //constructor, inicializamos con las llaves ya que es más rápido.
    //a menudo no hacen falta constructores
    Circulo(Punto centro, float radio): centro{centro}, radio{radio} {};
    //constructor vacío
        //por definición creo un punto con las coordenadas [0,0] con el constructor vacío de punto
        //por definición creo un punto con radio 0.
    Circulo(): centro{}, radio{0} {};
    
    ~Circulo(){}
    //constructor copia

    //constructor move


	//Si tenemos referencias o punteros entre los miembros de nuestra clase, hacen falta:
		//Constructor copia
		//Constructor move
		//A menudo constructor vacío ()
		//El destructor (sólo si hay reservas con malloc/new, o polimorfismo)
};