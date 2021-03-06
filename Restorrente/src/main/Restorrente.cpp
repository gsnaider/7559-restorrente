

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


	if (cantMesas <= 0 || cantMozos <= 0 || cantRecepcionistas <= 0 || cantComensales <= 0){
		Logger::log(mainLogId, "Datos invalidos en configuracion.", INFO);
		exit(0);
	}

	Logger::log(mainLogId, "Cantidad de Mozos: " + Logger::intToString(cantMozos), INFO);
	Logger::log(mainLogId, "Cantidad de Recepcionistas: " + Logger::intToString(cantRecepcionistas), INFO);
	Logger::log(mainLogId, "Cantidad de Mesas: " + Logger::intToString(cantMesas), INFO);
	Logger::log(mainLogId, "Cantidad de Comensales: " + Logger::intToString(cantComensales), INFO);
	Logger::log(mainLogId, "nivelDeLog: " + Logger::intToString(nivelDeLog), INFO);

	int comensalesPendientes = cantComensales;
	double perdidas = 0;
	do {
		MainProcess mainProcess(cantRecepcionistas, cantMozos, cantMesas, comensalesPendientes, perdidas, menu);
		mainProcessReturnData mainProcessReturnData = mainProcess.run();
		comensalesPendientes -= mainProcessReturnData.cantComensalesFinalizados;
		perdidas += mainProcessReturnData.perdidas;
	} while(comensalesPendientes > 0);

	Logger::log(mainLogId, "Simulacion finalizada", INFO);


	return 0;
}
