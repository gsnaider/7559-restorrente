/*
 * Process.h
 *
 *  Created on: Oct 18, 2016
 *      Author: gaston
 */

#ifndef PROCESSES_PROCESS_H_
#define PROCESSES_PROCESS_H_

#include <string>
#include "../utils/logger/Logger.h"

class SIGINT_ProcessHandler ;

namespace std{

const string processLogId = "Process";

class Process {
protected:
	void inicializarHandler();
	SIGINT_ProcessHandler* handler;

public:
	Process();
	virtual void run() = 0;
	virtual void limpiarRecursos() = 0;
	void finalizar();
	virtual ~Process();
};

}
#endif /* PROCESSES_PROCESS_H_ */
