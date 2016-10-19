/*
 * Process.cpp
 *
 *  Created on: Oct 18, 2016
 *      Author: gaston
 */

#include "Process.h"

#include "../utils/ipc/signal/SignalHandler.h"
#include "../utils/ipc/signal/SIGINT_ProcessHandler.h"

namespace std {

Process::Process() {
	handler = new SIGINT_ProcessHandler(this);
}


void Process::inicializarHandler(){
	SignalHandler::getInstance()->registrarHandler( SIGINT, handler);
}

void Process::finalizar(){
	Logger::log(processLogId, "Finalizando proceso", DEBUG);
	delete handler;
	exit(0);
}

Process::~Process() {
	// TODO Auto-generated destructor stub
}

} /* namespace std */
