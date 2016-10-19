/*
 * SIGUSR1_Handler.h
 *
 *  Created on: Oct 18, 2016
 *      Author: gaston
 */

#ifndef UTILS_IPC_SIGNAL_SIGUSR1_HANDLER_H_
#define UTILS_IPC_SIGNAL_SIGUSR1_HANDLER_H_


#include <signal.h>
#include <assert.h>

#include "EventHandler.h"

class SIGUSR1_Handler : public EventHandler {

	private:
		sig_atomic_t gracefulQuit;

	public:

		SIGUSR1_Handler () : gracefulQuit(0) {
		}

		~SIGUSR1_Handler () {
		}

		virtual int handleSignal ( int signum ) {
			assert ( signum == SIGUSR1 );
			this->gracefulQuit = 1;
			return 0;
		}

		sig_atomic_t getGracefulQuit () const {
			return this->gracefulQuit;
		}

};




#endif /* UTILS_IPC_SIGNAL_SIGUSR1_HANDLER_H_ */
