/*
 * RecepcionistaProcess.h
 *
 *  Created on: Oct 15, 2016
 *      Author: gaston
 */

#ifndef PROCESSES_RECEPCIONISTAPROCESS_H_
#define PROCESSES_RECEPCIONISTAPROCESS_H_

#include "../utils/ipc/semaphore/Semaforo.h"
#include "../utils/Logger.h"

namespace std {

const string recepcionistaLogId = "Recep";

const int TIEMPO_ANTENDIENDO = 10;

class RecepcionistaProcess {
private:
	Semaforo* semRecepcionistasLibres;
	Semaforo* semComensalesEnPuerta;

public:
	RecepcionistaProcess(Semaforo* semRecepcionistasLibres, Semaforo* semComensalesEnPuerta);
	void run();
	virtual ~RecepcionistaProcess();
};

} /* namespace std */

#endif /* PROCESSES_RECEPCIONISTAPROCESS_H_ */
