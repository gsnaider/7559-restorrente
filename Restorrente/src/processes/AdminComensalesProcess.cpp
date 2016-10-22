/*
 * CreadorComensalesProcess.cpp
 *
 *  Created on: Oct 18, 2016
 *      Author: gaston
 */

#include "AdminComensalesProcess.h"

#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <string>

#include "../utils/ipc/signal/SignalHandler.h"

namespace std {

AdminComensalesProcess::AdminComensalesProcess(int cantComensales, Menu menu, Semaforo* semRecepcionistasLibres, Semaforo* semComensalesEnPuerta,
			Semaforo* semPersonasLivingB, MemoriaCompartida<int>* shmPersonasLiving, Semaforo* semMesasLibres,
			vector<Semaforo>* semsMesasLibres, vector<MemoriaCompartida<bool>>* shmMesasLibres,
			Pipe* pipeLlamadosAMozos, vector<Semaforo>* semsLlegoComida, vector<Semaforo>* semsMesaPago){

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

	this->menu = menu;

	inicializarHandler();

}

void AdminComensalesProcess::inicializarHandler(){
	SignalHandler::getInstance()->registrarHandler(SIGINT, &sigintHandler);
}

int AdminComensalesProcess::run(){

	Logger::log(adminComensalesLogId, "Comenzando inicializacion de comensales", INFO);
	Logger::log(adminComensalesLogId, "Cantidad de comensales a inicializar: " + Logger::intToString(cantComensales) , INFO);

	vector<pid_t> idsComensales;

	int comensalesCreados = 0;

	bool corteLuz = false;
	while (comensalesCreados < cantComensales && !corteLuz){

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
			Logger::log(adminComensalesLogId, "Se inicializo un nuevo comensal con id: " + Logger::intToString(idComensal), INFO);
			idsComensales.push_back(idComensal);
			comensalesCreados++;
		}

		if (TiemposEspera::tiempos){
			sleep(RandomUtil::randomInt(TiemposEspera::TIEMPO_RANDOM_ENTRE_COMENSALES));
		}
		corteLuz = (sigintHandler.getGracefulQuit() == 1);
		if (corteLuz){
			Logger::log(adminComensalesLogId, "Corte de luz: abortando creacion de comensales. Comensales creados hasta el momento: " + Logger::intToString(comensalesCreados) , DEBUG);
			Logger::log(adminComensalesLogId, "Corte de luz: Se van los comensales que estaban en el restaurant" , INFO);
		}

	}

	if(corteLuz){

		Logger::log(adminComensalesLogId, "Reenviando senial a los comensales" , DEBUG);
		for (unsigned int i = 0; i < idsComensales.size(); i++){
			kill(idsComensales[i], SIGINT);
		}

	} else {

		Logger::log(adminComensalesLogId, "Todos los comensales fueron inicializados: "  + Logger::intToString(comensalesCreados) , DEBUG);
		Logger::log(adminComensalesLogId, "Esperando finalizacion de comensales" , DEBUG);


		int comensalesTerminados = 0;

		while ((comensalesTerminados < comensalesCreados) && (!corteLuz)){
			pid_t id = wait(NULL);
			corteLuz = (sigintHandler.getGracefulQuit() == 1);
			if (corteLuz){
				Logger::log(adminComensalesLogId, "Corte de luz: abortando comensales. " , DEBUG);
				Logger::log(adminComensalesLogId, "Corte de luz: Se van los comensales que estaban en el restaurant" , INFO);
			}else{
				Logger::log(adminComensalesLogId, "Termino el comensal "+ Logger::intToString(id), DEBUG);
				comensalesTerminados++;
			}
		}

		if(corteLuz){
			Logger::log(adminComensalesLogId, "Reenviando senial a los comensales" , DEBUG);
			for (unsigned int i = 0; i < idsComensales.size(); i++){
				kill(idsComensales[i], SIGINT);
			}
		}
	}

	Logger::log(adminComensalesLogId, "Devolviendo cantidad de comensales inicializados: " + Logger::intToString(comensalesCreados) , DEBUG);

	return comensalesCreados;

}

AdminComensalesProcess::~AdminComensalesProcess() {
	// TODO Auto-generated destructor stub
}

} /* namespace std */
