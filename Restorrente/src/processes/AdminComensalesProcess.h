/*
 * CreadorComensalesProcess.h
 *
 *  Created on: Oct 18, 2016
 *      Author: gaston
 */

#ifndef PROCESSES_ADMINCOMENSALESPROCESS_H_
#define PROCESSES_ADMINCOMENSALESPROCESS_H_

#include <sched.h>
#include <vector>

#include "../model/Menu.h"
#include "../utils/ipc/semaphore/Semaforo.h"
#include "../utils/ipc/shared-memory/MemoriaCompartida.h"
#include "../utils/ipc/signal/SIGINT_Handler.h"
#include "../utils/ipc/pipe/Pipe.h"
#include "../utils/random/RandomUtil.h"
#include "../utils/logger/Logger.h"
#include "GrupoComensalesProcess.h"

namespace std {
class Menu;
} /* namespace std */

namespace std {

const string adminComensalesLogId = "AdmComen";

class AdminComensalesProcess {
private:

	const int MAX_PERSONAS_POR_GRUPO = 5;

	int cantComensales;
	Menu menu;

	Semaforo* semRecepcionistasLibres;
	Semaforo* semComensalesEnPuerta;
	Semaforo* semMesasLibres;

	Semaforo* semPersonasLivingB;
	MemoriaCompartida<int>* shmPersonasLiving;

	Pipe* pipeLlamadosAMozos;

	vector<Semaforo>* semsLlegoComida;
	vector<Semaforo>* semsMesaPago;

	vector<Semaforo>* semsMesasLibres;
	vector<MemoriaCompartida<bool>>* shmMesasLibres;

	SIGINT_Handler* sigintHandler;


public:
	AdminComensalesProcess(int cantComensales, Menu menu, Semaforo* semRecepcionistasLibres, Semaforo* semComensalesEnPuerta,
			Semaforo* semPersonasLivingB, MemoriaCompartida<int>* shmPersonasLiving, Semaforo* semMesasLibres,
			vector<Semaforo>* semsMesasLibres, vector<MemoriaCompartida<bool>>* shmMesasLibres,
			Pipe* pipeLlamadosAMozos, vector<Semaforo>* semsLlegoComida, vector<Semaforo>* semsMesaPago, SIGINT_Handler* sigintHandler);
	int iniciarComensales();
	virtual ~AdminComensalesProcess();
};

} /* namespace std */

#endif /* PROCESSES_ADMINCOMENSALESPROCESS_H_ */
