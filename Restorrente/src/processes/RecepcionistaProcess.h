/*
 * RecepcionistaProcess.h
 *
 *  Created on: Oct 15, 2016
 *      Author: gaston
 */

#ifndef PROCESSES_RECEPCIONISTAPROCESS_H_
#define PROCESSES_RECEPCIONISTAPROCESS_H_

#include "../utils/ipc/semaphore/Semaforo.h"
#include "../utils/logger/Logger.h"
#include "Process.h"
#include "TiemposEspera.h"

namespace std {

const string recepcionistaLogId = "Recep";

class RecepcionistaProcess : public Process {
private:
	Semaforo* semRecepcionistasLibres;
	Semaforo* semComensalesEnPuerta;

public:
	RecepcionistaProcess(Semaforo* semRecepcionistasLibres, Semaforo* semComensalesEnPuerta);
	void run();
	void limpiarRecursos();
	virtual ~RecepcionistaProcess();
};

} /* namespace std */

#endif /* PROCESSES_RECEPCIONISTAPROCESS_H_ */
