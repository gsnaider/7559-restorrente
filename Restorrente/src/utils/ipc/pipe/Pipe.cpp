#include "Pipe.h"

#include <errno.h>
#include <cstring>
#include <iostream>
#include "../../logger/Logger.h"


Pipe :: Pipe() : lectura(true), escritura(true) {
	int resultado = pipe ( this->descriptores );
	if(resultado < 0){
		std::string stringError = strerror(errno);
		Logger::log(pipeLogId, "Error creacion pipe: "+ stringError, ERROR);
	}
	/*fcntl ( this->descriptors[0],F_SETFL,O_NONBLOCK );
	fcntl ( this->descriptors[1],F_SETFL,O_NONBLOCK );*/
}

Pipe::~Pipe() {
}

void Pipe :: setearModo ( const int modo ) {
	if ( modo == LECTURA ) {
		int resultado = close ( this->descriptores[1] );
		if(resultado < 0){
			string stringError = strerror(errno);
			Logger::log(pipeLogId, "Error close pipe: "+ stringError, ERROR);
		}
		this->escritura = false;

	} else if ( modo == ESCRITURA ) {
		int resultado = close ( this->descriptores[0] );
		if(resultado < 0){
			string stringError = strerror(errno);
			Logger::log(pipeLogId, "Error close pipe: "+ stringError, ERROR);
		}
		this->lectura = false;
	}
}

ssize_t Pipe :: escribir ( const void* dato,int datoSize ) {
	if ( this->lectura == true ) {
		int resultado = close ( this->descriptores[0] );
		if(resultado < 0){
			string stringError = strerror(errno);
			Logger::log(pipeLogId, "Error close pipe: "+ stringError, ERROR);
		}
		this->lectura = false;
	}

	ssize_t bytesLeidos =  write ( this->descriptores[1],dato,datoSize );
	if (bytesLeidos < 0){
		std::string stringError = strerror(errno);
		Logger::log(pipeLogId, "Error escritura pipe: "+ stringError, ERROR);
	}
	return bytesLeidos;
}

ssize_t Pipe :: leer ( void* buffer,const int buffSize ) {
	if ( this->escritura == true ) {
		int resultado = close ( this->descriptores[1] );
		if(resultado < 0){
			std::string stringError = strerror(errno);
			Logger::log(pipeLogId, "Error close pipe: "+ stringError, ERROR);
		}
		this->escritura = false;
	}

	ssize_t bytesLeidos = read ( this->descriptores[0],buffer,buffSize );
	if (bytesLeidos < 0){
		std::string stringError = strerror(errno);
		Logger::log(pipeLogId, "Error lectura pipe: "+ stringError, ERROR);
	}
	return bytesLeidos;
}

int Pipe :: getFdLectura () const {
	if ( this->lectura == true )
		return this->descriptores[0];
	else
		return -1;
}

int Pipe :: getFdEscritura () const {
	if ( this->escritura == true )
		return this->descriptores[1];
	else
		return -1;
}

void Pipe :: cerrar () {
	if ( this->lectura == true ) {
		int resultado = close ( this->descriptores[0] );
		if(resultado < 0){
			std::string stringError = strerror(errno);
			Logger::log(pipeLogId, "Error close pipe: "+ stringError, ERROR);
		}
		this->lectura = false;
	}

	if ( this->escritura == true ) {
		int resultado = close ( this->descriptores[1] );
		if(resultado < 0){
			std::string stringError = strerror(errno);
			Logger::log(pipeLogId, "Error close pipe: "+ stringError, ERROR);
		}
		this->escritura = false;
	}
}
