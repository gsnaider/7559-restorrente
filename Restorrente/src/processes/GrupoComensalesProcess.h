/*
 * GrupoComensalesProcess.h
 *
 *  Created on: Oct 15, 2016
 *      Author: gaston
 */

#ifndef PROCESSES_GRUPOCOMENSALESPROCESS_H_
#define PROCESSES_GRUPOCOMENSALESPROCESS_H_

#include "../utils/ipc/shared-memory/MemoriaCompartida.h"
#include "../utils/ipc/semaphore/Semaforo.h"
#include "../utils/ipc/pipe/Pipe.h"
#include "../model/Menu.h"
#include "../model/Pedido.h"
#include "RecepcionistaProcess.h"
#include "../main/MainProcess.h"


namespace std {

const string comensalLogId = "Comens";


const double PROBABILIDAD_IRSE = 0.25;
const int TIEMPO_COMER = 30;

class GrupoComensalesProcess {
private:

	int cantPersonas;
	int mesa;

	Semaforo* semRecepcionistasLibres;
	Semaforo* semComensalesEnPuerta;
	Semaforo* semMesasLibres;

	Semaforo* semPersonasLivingB;
	MemoriaCompartida<int>* shmPersonasLiving;

	Pipe* pipeLlamadosAMozos;

	vector<Semaforo>* semsLlegoComida;
	vector<Semaforo>* semsMesaPago;

	vector<Semaforo>* semsMesasLibres;
	vector<MemoriaCompartida<bool>>* shmMesasLibres;

	Menu menu;

	void llegar();
	void comer();
	void pagar();
	void irse();

	int obtenerNumeroMesa();

	Pedido generarPedido();

	void inicializarMemoriasCompartidas();

	void liberarMemoriasCompartidas();


public:
	GrupoComensalesProcess(int cantPersonas, Semaforo* semRecepcionistasLibres, Semaforo* semComensalesEnPuerta,
			Semaforo* semPersonasLivingB, MemoriaCompartida<int>* shmPersonasLiving, Semaforo* semMesasLibres,
			vector<Semaforo>* semsMesasLibres, vector<MemoriaCompartida<bool>>* shmMesasLibres,
			Pipe* pipeLlamadosAMozos, vector<Semaforo>* semsLlegoComida, vector<Semaforo>* semsMesaPago, Menu menu);

	void run();
	virtual ~GrupoComensalesProcess();
};

} /* namespace std */

#endif /* PROCESSES_GRUPOCOMENSALESPROCESS_H_ */
