

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

int main() {
	//TODO Reemplazar todos los cout por el log.

	Parser p = Parser();
	p.parsearDocumento("../SetUp.xml");

	//Direccion para usar en eclipse
	//p.parsearDocumento("SetUp.xml");

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
	Menu menu = p.getMenu();

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
