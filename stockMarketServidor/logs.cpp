#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>

void escribirLog(const std::string& mensaje) {
    std::ofstream archivoLog("data/logs.txt", std::ios::app);
    if (!archivoLog.is_open()) return;

    std::time_t ahora = std::time(nullptr);
    std::tm* tiempoLocal = std::localtime(&ahora);

    archivoLog << "["
               << std::put_time(tiempoLocal, "%Y-%m-%d %H:%M:%S")
               << "] " << mensaje << std::endl;

    archivoLog.close();
}
