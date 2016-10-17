/*
 * MainProcess.cpp
 *
 *  Created on: Oct 15, 2016
 *      Author: gaston
 */

#include "MainProcess.h"

#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <iostream>

#include "../processes/CocineroProcess.h"
#include "../processes/GrupoComensalesProcess.h"
#include "../processes/MozoProcess.h"
#include "../processes/RecepcionistaProcess.h"
#include "../utils/random/RandomUtil.h"


namespace std {

MainProcess::MainProcess(int cantRecepcionistas, int cantMozos, int cantMesas, int cantComensales, Menu* menu) {
	this->cantRecepcionistas = cantRecepcionistas;
	this->cantMozos = cantMozos;
	this->cantMesas = cantMesas;
	this->cantComensales = cantComensales;
	this->menu = menu;


	semsLlegoComida = new vector<Semaforo*>();
	semsMesaPago = new vector<Semaforo*>();
	semsFacturas = new vector<Semaforo*>();
	semsMesasLibres = new vector<Semaforo*>();
	semsComidaEnMesas = new vector<Semaforo*>();
	shmMesasLibres = new vector<MemoriaCompartida<bool>*>();
	shmComidaEnMesas = new vector<MemoriaCompartida<Comida>*>();
	shmFacturas = new vector<MemoriaCompartida<double>*>();


	inicializarIPCs();
}

void MainProcess::iniciarProcesoCocinero(){
	cout << "DEBUG: Iniciando cocinero"<< endl;
	Logger::log(logId, "Iniciando cocinero", DEBUG);
	pid_t idCocinero = fork();
	if (idCocinero == 0){
		CocineroProcess cocinero;
		cocinero.run();
		exit(0);
	} else {
		this->idCocinero = idCocinero;
	}
}

void MainProcess::iniciarProcesosMozo(){
	for (int i = 0; i < cantMozos; i++){
		cout << "DEBUG: Iniciando mozo " << i << endl;
		Logger::log(logId, "Iniciando mozo "+ Logger::integerToString(i), DEBUG);


		pid_t idMozo = fork();

		if (idMozo == 0){
			MozoProcess mozo(pipeLlamadosAMozos, pipePedidosACocinar, semLlamadosAMozos,
					semsComidaEnMesas, shmComidaEnMesas, semsLlegoComida,
					semsFacturas, shmFacturas, semCajaB, shmCaja, semsMesaPago);
			mozo.run();
			exit(0);
		} else {
			idsMozos.push_back(idMozo);
		}

	}
}

void MainProcess::iniciarProcesosRecepcionista(){
	for (int i = 0; i < cantRecepcionistas; i++){
		cout << "DEBUG: Iniciando recepcionista " << i << endl;
		Logger::log(logId, "Iniciando recepcionista "+ Logger::integerToString(i), DEBUG);
		pid_t idRecepcionista = fork();

		if (idRecepcionista == 0){
			RecepcionistaProcess recepcionista(semRecepcionistasLibres, semComensalesEnPuerta);
			recepcionista.run();
			exit(0);
		} else {
			idsRecepcionistas.push_back(idRecepcionista);
		}
	}
}

void MainProcess::inicializarProcesosRestaurant(){
	iniciarProcesosRecepcionista();
	iniciarProcesosMozo();
	iniciarProcesoCocinero();
}

void MainProcess::inicializarSemaforos(){
	semComensalesEnPuerta = new Semaforo(SEM_COMENSALES_EN_PUERTA_INIT_FILE, 0, 0);
	semRecepcionistasLibres = new Semaforo(SEM_RECEPCIONISTAS_LIBRES_INIT_FILE, 0, 0); //cada recepcionista suma uno al semaforo cuando se inicia.
	semMesasLibres = new Semaforo(SEM_MESAS_LIBRES_INIT_FILE, cantMesas, 0);
	semPersonasLivingB = new Semaforo(SEM_PERSONAS_LIVING_INIT_FILE, 1, 0);
	semCajaB = new Semaforo(SEM_CAJA_INIT_FILE, 1, 0);
	semLlamadosAMozos = new Semaforo(SEM_LLAMADOS_MOZOS_INIT_FILE, 1, 0);

	for(int i = 0; i < cantMesas; i++){
		semsLlegoComida->push_back(new Semaforo(SEMS_LLEGO_COMIDA_INIT_FILE, 0, i));
		semsMesaPago->push_back(new Semaforo(SEMS_MESA_PAGO_INIT_FILE, 0, i));
		semsFacturas->push_back(new Semaforo(SEMS_FACTURA_INIT_FILE, 1, i));
		semsMesasLibres->push_back(new Semaforo(SEMS_MESAS_LIBRES_INIT_FILE, 1, i));
		semsComidaEnMesas->push_back(new Semaforo(SEMS_COMIDA_MESAS_INIT_FILE, 1, i));
	}
}

void MainProcess::inicializarMemoriasCompartidas(){
	semPersonasLivingB->p();
	shmPersonasLiving = new MemoriaCompartida<int>();

	semCajaB->p();
	shmCaja = new MemoriaCompartida<double>();

	for(int i = 0; i < cantMesas; i++){
		semsFacturas->at(i)->p();
		shmFacturas->push_back(new MemoriaCompartida<double>());

		semsMesasLibres->at(i)->p();
		shmMesasLibres->push_back(new MemoriaCompartida<bool>());

		semsComidaEnMesas->at(i)->p();
		shmComidaEnMesas->push_back(new MemoriaCompartida<Comida>());
	}
}

void MainProcess::crearMemoriasCompartidas(){
	shmPersonasLiving->crear(SHM_PERSONAS_LIVING, 0);
	shmPersonasLiving->escribir(0);
	semPersonasLivingB->v();

	shmCaja->crear(SHM_CAJA, 0);
	shmCaja->escribir(0);
	semCajaB->v();

	for(int i = 0; i < cantMesas; i++){
		shmFacturas->at(i)->crear(SHM_FACTURAS, i);
		shmFacturas->at(i)->escribir(0);
		semsFacturas->at(i)->v();

		shmMesasLibres->at(i)->crear(SHM_MESAS_LIBRES, i);
		shmMesasLibres->at(i)->escribir(true);
		semsMesasLibres->at(i)->v();

		shmComidaEnMesas->at(i)->crear(SHM_COMIDA_MESAS, i);
		semsComidaEnMesas->at(i)->v();


	}
}

void MainProcess::inicializarPipesFifos(){
	pipeLlamadosAMozos = new Pipe();
	pipePedidosACocinar = new Pipe();
}


void MainProcess::inicializarIPCs(){
	inicializarSemaforos();
	inicializarMemoriasCompartidas();
	inicializarPipesFifos();

}

void MainProcess::finalizarProcesosRestaurant(){

	for (unsigned int i = 0; i < idsRecepcionistas.size(); i++){
		kill(idsRecepcionistas[i], 9);
	}

	for (unsigned int i = 0; i < idsMozos.size(); i++){
		kill(idsMozos[i], 9);
	}

	kill(idCocinero, 9);


}

void MainProcess::inicializarComensalesComensales(){
	for (int i = 0; i < cantComensales; i++){

		cout << "DEBUG: Iniciando comensal " << i << endl;
		Logger::log(logId, "Iniciando comensal "+ Logger::integerToString(i), DEBUG);
		pid_t idComensal = fork();

		if (idComensal == 0){
			int cantPersonas = RandomUtil::randomInt(MAX_PERSONAS_POR_GRUPO);

			GrupoComensalesProcess grupoComensalesProcess(cantPersonas, semRecepcionistasLibres, semComensalesEnPuerta,
					semPersonasLivingB, shmPersonasLiving, semMesasLibres,
					semsMesasLibres, shmMesasLibres,
					pipeLlamadosAMozos, semsLlegoComida, semsComidaEnMesas,
					shmComidaEnMesas, semsMesaPago, menu);
			grupoComensalesProcess.run();
			exit(0);
		} else {
			idsComensales.push_back(idComensal);
		}

	}

}

void MainProcess::run(){

	inicializarProcesosRestaurant();
	inicializarComensalesComensales();

	crearMemoriasCompartidas();

	// Estoy asumiendo que los unicos que pueden terminar "por su cuenta" son los comensales cuando se van.
	for (int i = 0; i < cantComensales; i++){
		pid_t idHijo = wait(NULL);
		cout << "DEBUG: Termino proceso hijo: " << idHijo << endl;
		Logger::log(logId, "Termino proceso hijo "+ Logger::integerToString(idHijo), DEBUG);
	}

	finalizarProcesosRestaurant();

}

void MainProcess::eliminarSemaforos(){

	semComensalesEnPuerta->eliminar();
	delete semComensalesEnPuerta;

	semRecepcionistasLibres->eliminar();
	delete semRecepcionistasLibres;

	semMesasLibres->eliminar();
	delete semMesasLibres;

	semPersonasLivingB->eliminar();
	delete semPersonasLivingB;

	semCajaB->eliminar();
	delete semCajaB;

	semLlamadosAMozos->eliminar();
	delete semLlamadosAMozos;

	for(int i = 0; i < cantMesas; i++){
		semsLlegoComida->at(i)->eliminar();
		delete semsLlegoComida->at(i);

		semsMesaPago->at(i)->eliminar();
		delete semsMesaPago->at(i);

		semsFacturas->at(i)->eliminar();
		delete semsFacturas->at(i);

		semsMesasLibres->at(i)->eliminar();
		delete semsMesasLibres->at(i);

		semsComidaEnMesas->at(i)->eliminar();
		delete semsComidaEnMesas->at(i);

	}
}

void MainProcess::eliminarMemoriasCompartidas(){

	shmPersonasLiving->liberar();
	delete shmPersonasLiving;

	shmCaja->liberar();
	delete shmCaja;

	for(int i = 0; i < cantMesas; i++){

		shmFacturas->at(i)->liberar();
		delete shmFacturas->at(i);

		shmMesasLibres->at(i)->liberar();
		delete shmMesasLibres->at(i);

		shmComidaEnMesas->at(i)->liberar();
		delete shmComidaEnMesas->at(i);

	}
}

void MainProcess::eliminarPipesFifos(){
	delete pipeLlamadosAMozos;
	delete pipePedidosACocinar;
}


void MainProcess::eliminarIPCs(){
	eliminarPipesFifos();
	eliminarMemoriasCompartidas();
	eliminarSemaforos();
}

MainProcess::~MainProcess() {

	eliminarIPCs();

	delete semsLlegoComida;
	delete semsMesaPago;
	delete semsFacturas;
	delete semsMesasLibres;
	delete semsComidaEnMesas;
	delete shmMesasLibres;
	delete shmComidaEnMesas;
	delete shmFacturas;

	delete menu;

}

} /* namespace std */
