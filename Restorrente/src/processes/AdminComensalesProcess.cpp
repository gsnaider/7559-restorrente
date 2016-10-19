/*
 * CreadorComensalesProcess.cpp
 *
 *  Created on: Oct 18, 2016
 *      Author: gaston
 */

#include "AdminComensalesProcess.h"

#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <string>

#include "../main/MainProcess.h"

namespace std {

AdminComensalesProcess::AdminComensalesProcess(int cantComensales, Menu menu, Semaforo* semRecepcionistasLibres, Semaforo* semComensalesEnPuerta,
			Semaforo* semPersonasLivingB, MemoriaCompartida<int>* shmPersonasLiving, Semaforo* semMesasLibres,
			vector<Semaforo>* semsMesasLibres, vector<MemoriaCompartida<bool>>* shmMesasLibres,
			Pipe* pipeLlamadosAMozos, vector<Semaforo>* semsLlegoComida, vector<Semaforo>* semsMesaPago, SIGINT_Handler* sigintHandler){

	this->cantComensales = cantComensales;
	this->semComensalesEnPuerta = semComensalesEnPuerta;
	this->semRecepcionistasLibres = semRecepcionistasLibres;
	this->semPersonasLivingB = semPersonasLivingB;
	this->semMesasLibres = semMesasLibres;

	this->shmPersonasLiving = shmPersonasLiving;


	this->pipeLlamadosAMozos = pipeLlamadosAMozos;

	this->semsLlegoComida = semsLlegoComida;
	this->semsMesaPago = semsMesaPago;

	this->semsMesasLibres = semsMesasLibres;
	this->shmMesasLibres = shmMesasLibres;

	this->sigintHandler = sigintHandler;
	this->menu = menu;
}

int AdminComensalesProcess::iniciarComensales(){

	Logger::log(adminComensalesLogId, "Comenzando inicializacion de comensales", DEBUG);
	Logger::log(adminComensalesLogId, "Cantidad de comensales a inicializar: " + Logger::intToString(cantComensales) , DEBUG);


	int comensalesCreados = 0;

	int i = 0;
	bool corteLuz = false;
	while (i < cantComensales && !corteLuz){

		sleep(RandomUtil::randomInt(10));

		pid_t idComensal = fork();

		if (idComensal == 0) {
			int cantPersonas = RandomUtil::randomInt(MAX_PERSONAS_POR_GRUPO) + 1;
			GrupoComensalesProcess grupoComensalesProcess(cantPersonas, semRecepcionistasLibres,
					semComensalesEnPuerta, semPersonasLivingB,
					shmPersonasLiving, semMesasLibres, semsMesasLibres,
					shmMesasLibres, pipeLlamadosAMozos, semsLlegoComida,
					semsMesaPago, menu);
			grupoComensalesProcess.run();
			exit(0);
		} else {
			comensalesCreados++;
		}

		corteLuz = (sigintHandler->getGracefulQuit() == 1);
		if (corteLuz){
			Logger::log(adminComensalesLogId, "Corte de luz: abortando creacion de comensales. Comensales creados hasta el momento: " + Logger::intToString(comensalesCreados) , DEBUG);
		}
		i++;
	}

	int comensalesTerminados = 0;

	while (comensalesTerminados < cantComensales){

			pid_t id = wait(NULL);
			Logger::log(adminComensalesLogId, "Termino el comensal "+ Logger::intToString(id), DEBUG);
			comensalesTerminados++;
	}

	return comensalesCreados;

}

AdminComensalesProcess::~AdminComensalesProcess() {
	// TODO Auto-generated destructor stub
}

} /* namespace std */
