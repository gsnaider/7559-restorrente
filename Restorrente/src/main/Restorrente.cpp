

#include <iostream>
#include <string>
#include <vector>

#include "../model/Comida.h"
#include "../model/Menu.h"
#include "../model/Pedido.h"
#include "../model/Plato.h"
#include "../utils/serializer/LlamadoAMozoSerializer.h"
#include "MainProcess.h"



using namespace std;

//TODO Borrar despues.
int testSerializador(){

	Plato plato1("Fideos con tuco", 50);

	Plato plato2("Milanesa con papas fritas", 80);

	Plato plato3("Ensalada mixta", 45.50);

	Pedido pedido(5);

	pedido.agregarPlato(plato1);
	pedido.agregarPlato(plato2);
	pedido.agregarPlato(plato3);

	cout << LlamadoAMozoSerializer::serializar(pedido) << endl;

	cout << LlamadoAMozoSerializer::getTipoLlamado("%1%2%3%Fideos con tuco%50.00%Milanesa con papas fritas%80.00%Ensalada mixta%45.50%") << endl;

	Comida comidaDes = LlamadoAMozoSerializer::deserializarComida("%2%2%3%Fideos con tuco%50.00%Milanesa con papas fritas%80.00%Ensalada mixta%45.50%");

	cout << comidaDes.getMesa() << endl;

	for (unsigned int i = 0; i < comidaDes.getPlatos().size(); i++){
		cout << comidaDes.getPlatos().at(i).getNombre() << endl;
		cout << comidaDes.getPlatos().at(i).getPrecio() << endl;
	}

	return 0;

}


int main() {
	//TODO Reemplazar todos los cout por el log.


/*
	// TODO Leer de archivo de config.
	Parser p ;
	p.parse("src/utils/SetUp.txt");
	int cantMozos = p.getCantMozos();
	int cantRecepcionistas = p.getCantRecepcionistas();
	int cantMesas = p.getCantMesas();
	int cantComensales = p.getCantComensales();
*/


	int cantMozos = 1;
	int cantRecepcionistas = 1;
	int cantMesas = 1;
	int cantComensales = 2;
	Menu* menu = new Menu();

	Plato plato1("Fideos con tuco", 50);
	menu->agregarPlato(plato1);

	Plato plato2("Milanesa con papas fritas", 80);
	menu->agregarPlato(plato2);

	Plato plato3("Ensalada mixta", 45.50);
	menu->agregarPlato(plato3);


	MainProcess mainProcess(cantRecepcionistas, cantMozos, cantMesas, cantComensales, menu);
	mainProcess.run();


	return 0;
}
