/*
 * MainProcess.h
 *
 *  Created on: Oct 15, 2016
 *      Author: gaston
 */

#ifndef PROCESSES_MAINPROCESS_H_
#define PROCESSES_MAINPROCESS_H_

#include <sched.h>
#include <string>
#include <vector>

//#include "../model/Comida.h"
#include "../model/Menu.h"
#include "../utils/ipc/pipe/Pipe.h"
#include "../utils/ipc/semaphore/Semaforo.h"
#include "../utils/ipc/shared-memory/MemoriaCompartida.h"
#include "../utils/ipc/signal/SIGINT_Handler.h"
#include "../utils/ipc/signal/SIGUSR1_Handler.h"
//#include "../utils/logger/Logger.h"
#include "../processes/TiemposEspera.h"



namespace std {

const string mainLogId = "Main";


const string SEM_COMENSALES_EN_PUERTA_INIT_FILE = "../ipc-init-files/sem_comensales_en_puerta.txt";
const string SEM_RECEPCIONISTAS_LIBRES_INIT_FILE = "../ipc-init-files/sem_recepcionistas_libres.txt";
const string SEM_MESAS_LIBRES_INIT_FILE = "../ipc-init-files/sem_mesas_libres.txt";
const string SEM_PERSONAS_LIVING_INIT_FILE = "../ipc-init-files/sem_personas_living.txt";
const string SEM_CAJA_INIT_FILE = "../ipc-init-files/sem_caja.txt";
const string SEM_LLAMADOS_MOZOS_INIT_FILE = "../ipc-init-files/sem_llamados_mozos.txt";
const string SEMS_LLEGO_COMIDA_INIT_FILE = "../ipc-init-files/sems_llego_comida.txt";
const string SEMS_MESA_PAGO_INIT_FILE = "../ipc-init-files/sems_mesa_pago.txt";
const string SEMS_FACTURA_INIT_FILE = "../ipc-init-files/sems_factura.txt";
const string SEMS_MESAS_LIBRES_INIT_FILE = "../ipc-init-files/sems_mesas_libres.txt";

const string SHM_PERSONAS_LIVING = "../ipc-init-files/shm_personas_living.txt";
const string SHM_CAJA = "../ipc-init-files/shm_caja.txt";
const string SHM_FACTURAS = "../ipc-init-files/shm_facturas.txt";
const string SHM_MESAS_LIBRES = "../ipc-init-files/shm_mesas_libres.txt";


struct mainProcessReturnData {
	int cantComensalesFinalizados;
	double perdidas;
};

class MainProcess {

private:

	int cantMozos;
	int cantRecepcionistas;
	int cantMesas;
	int cantComensales;
	Menu menu;

	double perdidas;

	vector<pid_t> idsRecepcionistas;
	vector<pid_t> idsMozos;
	vector<pid_t> idsComensales;
	pid_t idAdminComensales;
	pid_t idCocinero;
	pid_t idGerente;


	Semaforo semComensalesEnPuerta;
	Semaforo semRecepcionistasLibres;
	Semaforo semMesasLibres;
	Semaforo semPersonasLivingB;
	Semaforo semCajaB;
	Semaforo semLlamadosAMozos;
	vector<Semaforo> semsLlegoComida;
	vector<Semaforo> semsMesaPago;
	vector<Semaforo> semsFacturas;
	vector<Semaforo> semsMesasLibres;

	MemoriaCompartida<int> shmPersonasLiving;
	MemoriaCompartida<double> shmCaja;

	vector<MemoriaCompartida<bool>> shmMesasLibres;
	vector<MemoriaCompartida<double>> shmFacturas;

	Pipe pipeLlamadosAMozos;
	Pipe pipePedidosACocinar;

	SIGINT_Handler sigintHandler;
	SIGUSR1_Handler sigusr1Handler;

	void inicializarIPCs();
	void inicializarSemaforos();
	void inicializarMemoriasCompartidas();
	void crearMemoriasCompartidas();
	void inicializarPipesFifos();
	void inicializarSigintHandler();
	void inicializarSigusr1Handler();


	void inicializarProcesosRestaurant();
	void iniciarProcesosMozo();
	void iniciarProcesosRecepcionista();
	void iniciarProcesoCocinero();

	void iniciarProcesoGerente();


	void iniciarSimulacion();

	void inicializarProcesosComensales();

	void finalizarProcesosRestaurant();
	int finalizarComensales();

	void eliminarIPCs();
	void eliminarSemaforos();
	void eliminarMemoriasCompartidas();
	void eliminarPipesFifos();

	int handleCorteLuz();
	void acumularPerdidas();



public:
	MainProcess(int cantRecepcionistas, int cantMozos, int cantMesas, int cantComensales, int perdidas, Menu menu);

	/**
	 * Devuelve la cantidad de comensales que pasaron por el restaurant.
	 */
	mainProcessReturnData run();

	virtual ~MainProcess();
};

} /* namespace std */

#endif /* PROCESSES_MAINPROCESS_H_ */
