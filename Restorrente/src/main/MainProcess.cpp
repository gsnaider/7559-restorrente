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

MainProcess::MainProcess(int cantRecepcionistas, int cantMozos, int cantMesas, int cantComensales, Menu menu):
			semComensalesEnPuerta(SEM_COMENSALES_EN_PUERTA_INIT_FILE, 0, 0),
			semRecepcionistasLibres(SEM_RECEPCIONISTAS_LIBRES_INIT_FILE, 0, 0), //cada recepcionista suma uno al semaforo cuando se inicia.
			semMesasLibres(SEM_MESAS_LIBRES_INIT_FILE, cantMesas, 0),
			semPersonasLivingB(SEM_PERSONAS_LIVING_INIT_FILE, 1, 0),
			semCajaB(SEM_CAJA_INIT_FILE, 1, 0),
			semLlamadosAMozos(SEM_LLAMADOS_MOZOS_INIT_FILE, 1, 0)

		{


	this->cantRecepcionistas = cantRecepcionistas;
	this->cantMozos = cantMozos;
	this->cantMesas = cantMesas;
	this->cantComensales = cantComensales;
	this->menu = menu;


	inicializarIPCs();
}

void MainProcess::iniciarProcesoCocinero(){
	cout << "DEBUG: Iniciando cocinero"<< endl;
	Logger::log(mainLogId, "Iniciando cocinero", DEBUG);
	pid_t idCocinero = fork();
	if (idCocinero == 0){
		CocineroProcess cocinero(&pipePedidosACocinar, &semsFacturas,
				&shmFacturas, &pipeLlamadosAMozos);
		cocinero.run();
		exit(0);
	} else {
		this->idCocinero = idCocinero;
	}
}

void MainProcess::iniciarProcesosMozo(){
	for (int i = 0; i < cantMozos; i++){
		cout << "DEBUG: Iniciando mozo " << i << endl;
		Logger::log(mainLogId, "Iniciando mozo "+ Logger::integerToString(i), DEBUG);


		pid_t idMozo = fork();

		if (idMozo == 0){
			MozoProcess mozo(&pipeLlamadosAMozos, &pipePedidosACocinar, &semLlamadosAMozos,
					&semsLlegoComida, &semsFacturas, &shmFacturas, &semCajaB, &shmCaja, &semsMesaPago);
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
		Logger::log(mainLogId, "Iniciando recepcionista "+ Logger::integerToString(i), DEBUG);
		pid_t idRecepcionista = fork();

		if (idRecepcionista == 0){
			RecepcionistaProcess recepcionista(&semRecepcionistasLibres, &semComensalesEnPuerta);
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

	for(int i = 0; i < cantMesas; i++){
		semsLlegoComida.push_back(Semaforo(SEMS_LLEGO_COMIDA_INIT_FILE, 0, i));
		semsMesaPago.push_back(Semaforo(SEMS_MESA_PAGO_INIT_FILE, 0, i));
		semsFacturas.push_back(Semaforo(SEMS_FACTURA_INIT_FILE, 1, i));
		semsMesasLibres.push_back(Semaforo(SEMS_MESAS_LIBRES_INIT_FILE, 1, i));
	}
}

void MainProcess::inicializarMemoriasCompartidas(){

	// Bloqueo para primero crear los procesos, y despues asignarles los valores iniciales sin que ningun proceso hijo acceda antes.
	semPersonasLivingB.p();
	semCajaB.p();

	for(int i = 0; i < cantMesas; i++){
		semsFacturas.at(i).p();
		shmFacturas.push_back(MemoriaCompartida<double>());

		semsMesasLibres.at(i).p();
		shmMesasLibres.push_back(MemoriaCompartida<bool>());
	}
}

void MainProcess::crearMemoriasCompartidas(){

	cout << getpid() << " " << "DEBUG: Main: Comenzando inicializacion de memorias compartidas." << endl;

	shmPersonasLiving.crear(SHM_PERSONAS_LIVING, 0);
	shmPersonasLiving.escribir(0);
	semPersonasLivingB.v();

	shmCaja.crear(SHM_CAJA, 0);
	shmCaja.escribir(0);
	semCajaB.v();

	for(int i = 0; i < cantMesas; i++){
		shmFacturas.at(i).crear(SHM_FACTURAS, i);
		shmFacturas.at(i).escribir(0);
		semsFacturas.at(i).v();

		shmMesasLibres.at(i).crear(SHM_MESAS_LIBRES, i);
		shmMesasLibres.at(i).escribir(true);
		semsMesasLibres.at(i).v();


	}

	cout << getpid() << " " << "DEBUG: Main: Fin de inicializacion de memorias compartidas." << endl;
}

void MainProcess::inicializarPipesFifos(){

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
		Logger::log(mainLogId, "Iniciando comensal "+ Logger::integerToString(i), DEBUG);
		pid_t idComensal = fork();

		if (idComensal == 0){
			int cantPersonas = RandomUtil::randomInt(MAX_PERSONAS_POR_GRUPO) + 1;

			GrupoComensalesProcess grupoComensalesProcess(cantPersonas, &semRecepcionistasLibres, &semComensalesEnPuerta,
					&semPersonasLivingB, &shmPersonasLiving, &semMesasLibres,
					&semsMesasLibres, &shmMesasLibres,
					&pipeLlamadosAMozos, &semsLlegoComida, &semsMesaPago, menu);
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
		Logger::log(mainLogId, "Termino proceso hijo "+ Logger::integerToString(idHijo), DEBUG);
	}

	finalizarProcesosRestaurant();

}

void MainProcess::eliminarSemaforos(){

	semComensalesEnPuerta.eliminar();

	semRecepcionistasLibres.eliminar();

	semMesasLibres.eliminar();

	semPersonasLivingB.eliminar();

	semCajaB.eliminar();

	semLlamadosAMozos.eliminar();

	for(int i = 0; i < cantMesas; i++){
		semsLlegoComida.at(i).eliminar();

		semsMesaPago.at(i).eliminar();

		semsFacturas.at(i).eliminar();

		semsMesasLibres.at(i).eliminar();

	}
}

void MainProcess::eliminarMemoriasCompartidas(){

	shmPersonasLiving.liberar();

	shmCaja.liberar();

	for(int i = 0; i < cantMesas; i++){

		shmFacturas.at(i).liberar();

		shmMesasLibres.at(i).liberar();

	}
}

void MainProcess::eliminarPipesFifos(){
}


void MainProcess::eliminarIPCs(){
	eliminarPipesFifos();
	eliminarMemoriasCompartidas();
	eliminarSemaforos();
}

MainProcess::~MainProcess() {

	eliminarIPCs();


}

} /* namespace std */
