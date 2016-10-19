/*
 * Process.h
 *
 *  Created on: Oct 18, 2016
 *      Author: gaston
 */

#ifndef PROCESSES_PROCESS_H_
#define PROCESSES_PROCESS_H_

namespace std {

class Process {
public:
	Process();
	virtual void run() = 0;
	virtual void limpiarRecursos() = 0;
	virtual ~Process();
};

} /* namespace std */

#endif /* PROCESSES_PROCESS_H_ */
