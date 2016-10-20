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
#include "Process.h"

namespace std {

const string cocineroLogId = "Cocin";


class CocineroProcess : public Process {

private:

	const int TIEMPO_COCINA = 2;

	Pipe* pipePedidosACocinar;

	vector<Semaforo>* semsFacturas;
	vector<MemoriaCompartida<double>>* shmFacturas;

	Pipe* pipeLlamadosAMozos;

	void inicializarMemoriasCompartidas();
	void liberarMemoriasCompartidas();

	void facturar(int mesa, Plato plato);

	Comida cocinar(Pedido pedido);

	string leerPedido(int tamanioPedido);

	int leerTamanioPedido();

public:
	CocineroProcess(Pipe* pipePedidosACocinar, vector<Semaforo>* semsFacturas,
			vector<MemoriaCompartida<double>>* shmFacturas,
			Pipe* pipeLlamadosAMozos);
	void run();
	void limpiarRecursos();
	virtual ~CocineroProcess();
};

} /* namespace std */

#endif /* PROCESSES_COCINEROPROCESS_H_ */
