/*
 * Logger.cpp
 *
 *  Created on: Oct 16, 2016
 *      Author: juampa_94
 */
#include "Logger.h"
#include <fstream>
#include <ctime>
#include <sys/time.h>
#include "errno.h"
#include "string.h"
#include "ipc/lock/LockFile.h"
#include <iostream>

static Logger* instance = 0;

void Logger::setMode(LOG_MODE mode){
	getInstance()->mode = mode;
}

Logger::Logger()
{
    log_path = "log.txt";
    mode = INFO; //Por default se esta creando en INFO
    mode_symbols[0] = "D.\t";
    mode_symbols[1] = "E.\t";
    mode_symbols[2] = "I.\t";

    log_timestamp();
}

Logger::~Logger()
{
    if(instancia != NULL)
        delete instancia;
    instancia = NULL;
}

void Logger::log_timestamp(){
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,80,"%d-%m-%Y %H:%M:%S",timeinfo);
    std::string timestamp(buffer);

    std::string out_final = "/---------------------- " + timestamp + " ----------------------/" + "\n";

    LockFile log("log.txt");
    log.tomarLock();
    log.escribir(static_cast<const void*>(out_final.c_str()),out_final.length());
    log.liberarLock();
}

std::string Logger::timeHMSmu(){
    struct timeval tv;
    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64], buf[64];

    gettimeofday(&tv, NULL);
    nowtime = tv.tv_sec;
    nowtm = localtime(&nowtime);
    strftime(tmbuf, sizeof tmbuf, "%H:%M:%S", nowtm);
    snprintf(buf, sizeof(buf), "%s.%06d", tmbuf, tv.tv_usec);
    std::string str(buf);
    return str;
}

std::string integerToString ( pid_t number )
{
   std::ostringstream ss;
   ss << number;
   return ss.str();
}



void Logger::_log(std::string name, std::string comment, LOG_MODE comment_mode){
	if(comment_mode >= mode){
        std::string out_final = this->timeHMSmu() + "\t" +
                                name + "\t" +
                                integerToString(getpid()) + "\t" +
                                mode_symbols[comment_mode] + comment + "\n";

        LockFile log("log.txt");
        log.tomarLock();
        log.escribir(static_cast<const void*>(out_final.c_str()),out_final.length());
        log.liberarLock();
    }
}

Logger* Logger::getInstance(){
    if(!instance){
        instance = new Logger;
    }
    return instance;
}

void Logger::log(std::string name, std::string comment, LOG_MODE mode){
    Logger* l = Logger::getInstance();
    l->_log(name, comment, mode);
}

void Logger::logErrno(std::string name){
    char buf[256];
    char const* str = strerror_r(errno, buf, 256);
    std::string msg(str);

    Logger::log(name, msg, ERROR);
}

