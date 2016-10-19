/*
 * RecepcionistaProcess.cpp
 *
 *  Created on: Oct 15, 2016
 *      Author: gaston
 */

#include "RecepcionistaProcess.h"

#include <unistd.h>
#include <iostream>

namespace std {

RecepcionistaProcess::RecepcionistaProcess(Semaforo* semRecepcionistasLibres, Semaforo* semComensalesEnPuerta) {
	this->semRecepcionistasLibres = semRecepcionistasLibres;
	this->semComensalesEnPuerta = semComensalesEnPuerta;

	inicializarHandler();
}

void RecepcionistaProcess::run(){

	Logger::log(recepcionistaLogId, "Iniciando recepcionista ", DEBUG);


	//TODO Ver si hay mejor forma que while(true).
	while (true){
		semRecepcionistasLibres->v();
		Logger::log(recepcionistaLogId, "Recepcionista esperando comensales. ", INFO);

		semComensalesEnPuerta->p();
		Logger::log(recepcionistaLogId, "Recepcionista atendiendo grupo de comensales. ", INFO);

		sleep(TIEMPO_ANTENDIENDO);
	}


}

void RecepcionistaProcess::limpiarRecursos(){
	Logger::log(recepcionistaLogId, "Limpiando recursos", DEBUG);
}

RecepcionistaProcess::~RecepcionistaProcess() {
	// TODO Auto-generated destructor stub
}

} /* namespace std */
