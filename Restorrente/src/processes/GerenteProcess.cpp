/*
 * GerenteProcess.cpp
 *
 *  Created on: Oct 19, 2016
 *      Author: juampa_94
 */

#include "GerenteProcess.h"

#include <iostream>

#include "../main/MainProcess.h"
#include "../utils/logger/Logger.h"

using namespace std;

GerenteProcess::GerenteProcess(Semaforo* semCajaB, MemoriaCompartida<double>* shmCaja,
		Semaforo* semPersonasLivingB, MemoriaCompartida<int>* shmPersonasLiving,
		double perdidas) {

	this->semCajaB = semCajaB;
	this->shmCaja = shmCaja;
	this->semPersonasLivingB = semPersonasLivingB;
	this->shmPersonasLiving = shmPersonasLiving;
	this->perdidas = perdidas;

	inicializarMemoriasCompartidas();


}
void GerenteProcess::inicializarMemoriasCompartidas(){

	Logger::log(gerenteLogId, "Inicializo las memorias comaprtidas", DEBUG);
	this->shmCaja->crear(SHM_CAJA, 0);
	this->shmPersonasLiving->crear(SHM_PERSONAS_LIVING,0);
}


GerenteProcess::~GerenteProcess() {
	// TODO Auto-generated destructor stub
}

double GerenteProcess::consultarDineroEnCaja(){

	Logger::log(gerenteLogId, "Accediendo a shmCaja por semaforo", DEBUG);
	semCajaB->p();

	double dineroEnCaja = shmCaja->leer();
	Logger::log(gerenteLogId, "Ya pude leer de la caja", DEBUG);

	semCajaB->v();
	Logger::log(gerenteLogId, "Libero el semaforo de la caja", DEBUG);

	return dineroEnCaja;

}

double GerenteProcess::consutlarDineroPerdido(){

	return perdidas;

}

int GerenteProcess::consultarGenteEnLiving(){

	Logger::log(gerenteLogId, "Accediendo a shared memory perosnasLiving", DEBUG);
	semPersonasLivingB->p();

	int genteEsperando = shmPersonasLiving->leer();
	Logger::log(gerenteLogId, "Ya lei la gente esperando ", DEBUG);
	semPersonasLivingB->v();
	Logger::log(gerenteLogId, "Ya libero semaforo del Living", DEBUG);

	return genteEsperando;
}

void GerenteProcess::liberarMemoriasCompartidas(){

	Logger::log(gerenteLogId, "Liberando shmCaja", DEBUG);
	this->shmCaja->liberar();
	Logger::log(gerenteLogId, "Liberando shmPersonasLiving", DEBUG);
	this->shmPersonasLiving->liberar();
}

void GerenteProcess::limpiarRecursos(){
	Logger::log(gerenteLogId, "Limpiando recursos", DEBUG);
	liberarMemoriasCompartidas();
}
void GerenteProcess::run(){
	bool condicion = true;
	while(condicion){
		cout << "Bienvenido al proceso Gerente" << endl;
		cout << "Ingrese alguna de las siguientes opciónes: " << endl;

		cout <<"1 - Para consultar dinero en caja " << endl;
		cout <<"2 - Para consultar dinero perdido por cortes de luz" << endl;
		cout <<"3 - Para consultar grupos de personas esperando en  el living" << endl;
		cout <<"4 - Para salir" << endl;

		string opcion;
		cin >> opcion;

		if(opcion == "1"){
			cout << "Selecciono 1" << endl;
			double dineroEnCaja = consultarDineroEnCaja();
			cout << "El dinero actual en la caja es " << dineroEnCaja << endl;
		}
		else if(opcion == "2"){
			cout << "Selecciono 2" << endl;
			double dineroPerdido = consutlarDineroPerdido();
			cout << "Se perdio hasta el momento " << dineroPerdido << " por cortes de luz" << endl;
		}
		else if(opcion == "3"){
			cout << "Selecciono 3" << endl;
			int genteEnLiving = consultarGenteEnLiving();
			cout << "Actualmente hay " << genteEnLiving << " grupos de comensales esperando en el Living" << endl;
		}
		else if(opcion == "4"){
			cout << "Selecciono 4. Fin del Gerente" << endl;
			condicion = false;
		}
		else
			cout << "Ingreso opcion invalida" << endl;
	}

}

