/*
 * SIGINT_ProcessHandler.h
 *
 *  Created on: Oct 18, 2016
 *      Author: gaston
 */

#ifndef UTILS_IPC_SIGNAL_SIGINT_PROCESSHANDLER_H_
#define UTILS_IPC_SIGNAL_SIGINT_PROCESSHANDLER_H_


#include <assert.h>
#include <csignal>
#include <cstdlib>

#include "../../../processes/Process.h"
#include "EventHandler.h"

using namespace std;

class SIGINT_ProcessHandler : public EventHandler {

	private:
		Process* process;
		sig_atomic_t gracefulQuit;

	public:

		SIGINT_ProcessHandler (Process* process) : gracefulQuit(0) {
			this->process = process;
		}

		~SIGINT_ProcessHandler () {
		}

		virtual int handleSignal ( int signum ) {
			assert ( signum == SIGINT );
			this->process->limpiarRecursos();
			this->process->finalizar();
			exit(0);
			return 0;
		}

		sig_atomic_t getGracefulQuit () const {
			return this->gracefulQuit;
		}

};


#endif /* UTILS_IPC_SIGNAL_SIGINT_PROCESSHANDLER_H_ */
