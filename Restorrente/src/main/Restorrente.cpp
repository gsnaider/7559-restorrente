

#include <iostream>
#include <string>
#include <vector>

#include "../model/Comida.h"
#include "../model/Menu.h"
#include "../model/Pedido.h"
#include "../model/Plato.h"
#include "../utils/serializer/LlamadoAMozoSerializer.h"
#include "../utils/Parser.h"
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



	Parser p = Parser();
	//Direccion para usar en eclipse
	//p.parsearDocumento("SetUp.xml");
	p.parsearDocumento("../SetUp.xml");


	int nivelDeLog = p.getNivelDeLog();
	int cantMozos = p.gentCantMozos();
	int cantRecepcionistas = p.getCantRecepcionistas();
	int cantMesas = p.getCantMesas();
	int cantComensales = p.getCantComensales();

	cout << "Cantidad de Mozos: " << cantMozos << endl;
	cout << "Cantidad de Recepcionistas: " << cantRecepcionistas << endl;
	cout << "Cantidad de Mesas: " << cantMesas << endl;
	cout << "Cantidad de Comensales: " << cantComensales << endl;
	cout << "nivelDeLog: " << nivelDeLog << endl;
	Menu* menu = p.getMenu();
/*
	cout << menu->getPlatoRandom().getNombre() <<endl;
	cout << menu->getPlatoRandom().getNombre() <<endl;
	cout << menu->getPlatoRandom().getNombre() <<endl;
*/
/*
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

*/
	LOG_MODE mode;
	if(nivelDeLog == 0) mode = DEBUG;
	else if(nivelDeLog == 1) mode = ERROR;
	else mode = INFO;

	Logger::setMode(mode);

	Logger::log("Restorrente main", "Llamo a mainProcess", DEBUG);
	MainProcess mainProcess(cantRecepcionistas, cantMozos, cantMesas, cantComensales, menu);

	Logger::log("Restorrente main", "Corro el mainProcess", DEBUG);
	mainProcess.run();


	return 0;
}
