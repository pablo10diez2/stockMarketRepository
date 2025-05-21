#include "logs.h"
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <direct.h> // For Windows directory functions
#include <cerrno>   // For errno
#include <cstring>  // For strerror

void escribirLog(const std::string& mensaje) {
    // Asegurarse de que el directorio "data" existe
    // Usando funciones de Windows en lugar de std::filesystem
    if (_mkdir("data") != 0 && errno != EEXIST) {
        std::cerr << "Error al crear el directorio 'data': " << strerror(errno) << std::endl;
        // Intentar usar el directorio actual como alternativa
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
    // Removed the console output that was here

    // Intentar abrir el archivo en la carpeta data
    std::ofstream archivoLog("data/logs.txt", std::ios::app);
    if (!archivoLog.is_open()) {
        std::cerr << "No se pudo abrir el archivo de logs en 'data/logs.txt'" << std::endl;

        // Intentar con ruta alternativa en el directorio actual
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
