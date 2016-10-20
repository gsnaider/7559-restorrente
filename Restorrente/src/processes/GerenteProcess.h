/*
 * GerenteProcess.h
 *
 *  Created on: Oct 19, 2016
 *      Author: juampa_94
 */

#ifndef GERENTEPROCESS_H_
#define GERENTEPROCESS_H_

#include <string>

#include "../utils/ipc/shared-memory/MemoriaCompartida.h"
#include "Process.h"
#include "../utils/ipc/semaphore/Semaforo.h"


using namespace std;


class GerenteProcess: public std::Process {

private:
	const string gerenteLogId = "Gerente";

	Semaforo* semCajaB;
	MemoriaCompartida<double>* shmCaja;
	Semaforo* semPersonasLivingB;
	MemoriaCompartida<int>* shmPersonasLiving;
	double perdidas;

	void inicializarMemoriasCompartidas();
	void liberarMemoriasCompartidas();
	double consultarDineroEnCaja();
	double consutlarDineroPerdido();
	int consultarGenteEnLiving();



public:
	GerenteProcess(Semaforo* semCajaB,
			MemoriaCompartida<double>* shmCaja,
			Semaforo* semPersonasLivingB,
			MemoriaCompartida<int>* shmPersonasLiving,
			double perdidas);

	virtual ~GerenteProcess();
	void run();
	void limpiarRecursos();


};

#endif /* GERENTEPROCESS_H_ */
