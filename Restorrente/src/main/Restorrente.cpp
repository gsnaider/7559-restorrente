

#include <iostream>
#include <string>
#include <vector>

#include "../model/Comida.h"
#include "../model/Menu.h"
#include "../model/Pedido.h"
#include "../model/Plato.h"
#include "../utils/parser/Parser.h"
#include "../utils/serializer/LlamadoAMozoSerializer.h"
#include "MainProcess.h"



using namespace std;

int main() {

	Parser p = Parser();
	p.parsearDocumento("../SetUp.xml");

	//Direccion para usar en eclipse
	//p.parsearDocumento("SetUp.xml");

	int nivelDeLog = p.getNivelDeLog();
	int cantMozos = p.gentCantMozos();
	int cantRecepcionistas = p.getCantRecepcionistas();
	int cantMesas = p.getCantMesas();
	int cantComensales = p.getCantComensales();
	Menu menu = p.getMenu();

	LOG_MODE mode;
	if(nivelDeLog == 0) mode = DEBUG;
	else if(nivelDeLog == 1) mode = ERROR;
	else mode = INFO;

	Logger::setMode(mode);

	Logger::log(mainLogId, "Cantidad de Mozos: " + Logger::intToString(cantMozos), INFO);
	Logger::log(mainLogId, "Cantidad de Recepcionistas: " + Logger::intToString(cantRecepcionistas), INFO);
	Logger::log(mainLogId, "Cantidad de Mesas: " + Logger::intToString(cantMesas), INFO);
	Logger::log(mainLogId, "Cantidad de Comensales: " + Logger::intToString(cantComensales), INFO);
	Logger::log(mainLogId, "nivelDeLog: " + Logger::intToString(nivelDeLog), INFO);

	MainProcess mainProcess(cantRecepcionistas, cantMozos, cantMesas, cantComensales, menu);

	mainProcess.run();


	return 0;
}
