/*
 * CocineroProcess.h
 *
 *  Created on: Oct 15, 2016
 *      Author: gaston
 */

#ifndef PROCESSES_COCINEROPROCESS_H_
#define PROCESSES_COCINEROPROCESS_H_

#include <vector>

#include "../model/Pedido.h"
#include "../model/Comida.h"
#include "../utils/ipc/semaphore/Semaforo.h"
#include "../utils/ipc/pipe/Pipe.h"
#include "../utils/ipc/shared-memory/MemoriaCompartida.h"

namespace std {

class CocineroProcess {

private:

	const int TIEMPO_COCINA = 20;

	Pipe* pipePedidosACocinar;

	vector<Semaforo*>* semsFacturas;
	vector<MemoriaCompartida<double>*>* shmFacturas;

	Pipe* pipeLlamadosAMozos;

	void inicializarMemoriasCompartidas();

	Comida cocinar(Pedido pedido);

	string leerPedido(int tamanioPedido);

	int leerTamanioPedido();

public:
	CocineroProcess(Pipe* pipePedidosACocinar, vector<Semaforo*>* semsFacturas,
			vector<MemoriaCompartida<double>*>* shmFacturas,
			Pipe* pipeLlamadosAMozos);
	void run();
	virtual ~CocineroProcess();
};

} /* namespace std */

#endif /* PROCESSES_COCINEROPROCESS_H_ */
