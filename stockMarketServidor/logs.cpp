#include "logs.h"
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <direct.h>
#include <cerrno>
#include <cstring>

void escribirLog(const std::string& mensaje) {

    if (_mkdir("data") != 0 && errno != EEXIST) {
        std::cerr << "Error al crear el directorio 'data': " << strerror(errno) << std::endl;

        std::ofstream archivoLog("logs.txt", std::ios::app);
        if (!archivoLog.is_open()) {
            std::cerr << "No se pudo abrir el archivo de logs" << std::endl;
            return;
        }

        std::time_t ahora = std::time(nullptr);
        std::tm* tiempoLocal = std::localtime(&ahora);

        archivoLog << "["
                  << std::put_time(tiempoLocal, "%Y-%m-%d %H:%M:%S")
                  << "] " << mensaje << std::endl;

        archivoLog.close();
        return;
    }

    std::ofstream archivoLog("data/logs.txt", std::ios::app);
    if (!archivoLog.is_open()) {
        std::cerr << "No se pudo abrir el archivo de logs en 'data/logs.txt'" << std::endl;


        archivoLog.open("logs.txt", std::ios::app);
        if (!archivoLog.is_open()) {
            std::cerr << "Tampoco se pudo abrir el archivo de logs en el directorio actual" << std::endl;
            return;
        }
    }

    std::time_t ahora = std::time(nullptr);
    std::tm* tiempoLocal = std::localtime(&ahora);

    archivoLog << "["
               << std::put_time(tiempoLocal, "%Y-%m-%d %H:%M:%S")
               << "] " << mensaje << std::endl;

    archivoLog.close();
}
