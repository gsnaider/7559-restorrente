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

#include "../processes/AdminComensalesProcess.h"
#include "../processes/CocineroProcess.h"
#include "../processes/MozoProcess.h"
#include "../processes/RecepcionistaProcess.h"
#include "../processes/GerenteProcess.h"
#include "../utils/ipc/signal/SignalHandler.h"


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
void MainProcess::iniciarProcesoGerente(){
	Logger::log(mainLogId, "Iniciando gerente", DEBUG);
	pid_t idGerente = fork();
	if (idGerente == 0){
		GerenteProcess gerente(&semCajaB, &shmCaja,
				&semPersonasLivingB, &shmPersonasLiving, perdidas);
		gerente.run();
		exit(0);
	} else {
		this->idCocinero = idCocinero;
	}
}


void MainProcess::iniciarProcesosMozo(){
	for (int i = 0; i < cantMozos; i++){
		Logger::log(mainLogId, "Iniciando mozo "+ Logger::intToString(i), DEBUG);


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
		Logger::log(mainLogId, "Iniciando recepcionista "+ Logger::intToString(i), DEBUG);
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

	Logger::log(mainLogId, "Comenzando inicializacion de memorias compartidas ", DEBUG);
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

	Logger::log(mainLogId, "Fin de inicializacion de memorias compartidas ", DEBUG);
}

void MainProcess::inicializarPipesFifos(){

}


void MainProcess::inicializarIPCs(){
	inicializarSemaforos();
	inicializarMemoriasCompartidas();
	inicializarPipesFifos();

}

void MainProcess::finalizarProcesosRestaurant(){
	Logger::log(mainLogId, "Finalizando procesos del restaurant", DEBUG);

	for (unsigned int i = 0; i < idsRecepcionistas.size(); i++){
		Logger::log(mainLogId, "Finalizando recepcionista " + Logger::intToString(idsRecepcionistas[i]), DEBUG);
		kill(idsRecepcionistas[i], SIGINT);
		waitpid(idsRecepcionistas[i],NULL,0);
	}

	for (unsigned int i = 0; i < idsMozos.size(); i++){
		Logger::log(mainLogId, "Finalizando mozo " + Logger::intToString(idsMozos[i]), DEBUG);
		kill(idsMozos[i], SIGINT);
		waitpid(idsMozos[i],NULL,0);
	}

	Logger::log(mainLogId, "Finalizando cocinero" + Logger::intToString(idCocinero), DEBUG);
	kill(idCocinero, SIGINT);
	waitpid(idCocinero,NULL,0);

}

int MainProcess::finalizarComensales(){
	Logger::log(mainLogId, "Finalizando procesos de comensales", DEBUG);
	kill(idAdminComensales, SIGINT);
	int cantComensalesFinalizados;
	int response;
	waitpid(idAdminComensales, &response, 0);
	if (WIFEXITED(response)){
		cantComensalesFinalizados = WEXITSTATUS(response);
	}

	Logger::log(mainLogId, "Cantidad de comensales finalizados: " + Logger::intToString(cantComensalesFinalizados), DEBUG);
	return cantComensalesFinalizados;
}

void MainProcess::inicializarProcesosComensales(){
	pid_t idAdminComensales = fork();

	if (idAdminComensales == 0){
		AdminComensalesProcess adminComensales(cantComensales, menu, &semRecepcionistasLibres, &semComensalesEnPuerta,
				&semPersonasLivingB, &shmPersonasLiving, &semMesasLibres,
				&semsMesasLibres, &shmMesasLibres,
				&pipeLlamadosAMozos, &semsLlegoComida, &semsMesaPago);
		int comensalesFinalizados = adminComensales.run();

		Logger::log(adminComensalesLogId, "Exit con cantidad de comensales finalizados: " + Logger::intToString(comensalesFinalizados) , DEBUG);
		exit(comensalesFinalizados);
	} else {
		this->idAdminComensales = idAdminComensales;
	}
}

void MainProcess::inicializarSigintHandler(){
	SignalHandler::getInstance()->registrarHandler(SIGINT, &sigintHandler);
}

void MainProcess::acumularPerdidas(){

	Logger::log(mainLogId, "Acumulando perdidas por corte de luz", INFO);
	for (int mesa = 0; mesa < cantMesas; mesa++){
		semsFacturas.at(mesa).p();
		perdidas += shmFacturas.at(mesa).leer();
		semsFacturas.at(mesa).v();
	}
	Logger::log(mainLogId, "Perdidas acumuladas: $" + Logger::doubleToString(perdidas), INFO);
}


int MainProcess::handleCorteLuz(){
	Logger::log(mainLogId, "CORTE DE LUZ", INFO);
	acumularPerdidas();
	int comensalesFinalizados = finalizarComensales();
	finalizarProcesosRestaurant();
	eliminarIPCs();

	Logger::log(mainLogId, "ESPERANDO QUE VUELVA LA LUZ", INFO);
	sleep(30); //Durmiendo hasta q vuevle la luz

	return comensalesFinalizados;
}


void MainProcess::iniciarSimulacion(){
	inicializarProcesosRestaurant();
	inicializarProcesosComensales();
	crearMemoriasCompartidas();
	inicializarSigintHandler();
}

int MainProcess::run(){
	iniciarSimulacion();
	Logger::log(mainLogId, "Simulacion iniciada", DEBUG);

	int comensalesFinalizados = 0;
	int response;
	waitpid(idAdminComensales, &response, 0);
	bool corteLuz = (sigintHandler.getGracefulQuit() == 1);
	if (corteLuz){
		iniciarProcesoGerente();
		//comensalesFinalizados = handleCorteLuz();
	}else {

		//Leyendo respuesta del AdminComensales
		if (WIFEXITED(response)){
			comensalesFinalizados = WEXITSTATUS(response);
		}
		Logger::log(mainLogId, "Cantidad de comensales finalizados: " + Logger::intToString(comensalesFinalizados), DEBUG);

		finalizarProcesosRestaurant();
		eliminarIPCs();
	}

	return comensalesFinalizados;

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
	Logger::log(mainLogId, "Eliminando IPCs", DEBUG);
	eliminarPipesFifos();
	eliminarMemoriasCompartidas();
	eliminarSemaforos();
}

MainProcess::~MainProcess() {


}

} /* namespace std */
