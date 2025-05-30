#include "menuConsulta.h"
#include "Accion.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <winsock2.h>

std::vector<Accion> cargarAccionesDesdeFichero(const std::string& ruta) {
    std::vector<Accion> acciones;
    std::ifstream fichero(ruta);

    if (!fichero.is_open()) {

        return acciones;
    }

    std::string linea;
    while (std::getline(fichero, linea)) {
        std::stringstream ss(linea);
        std::string ticker, nombre, mercado;

        if (std::getline(ss, ticker, ',') && std::getline(ss, nombre, ',') && std::getline(ss, mercado)) {
            Accion a(ticker, nombre, mercado);
            acciones.emplace_back(ticker, nombre, mercado);

        }
    }

    fichero.close();
    return acciones;
}

std::string recibirLinea(SOCKET comm_socket) {
    char buffer[512];
    int bytes = recv(comm_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) return "";

    buffer[bytes] = '\0';
    std::string linea(buffer);


    linea.erase(std::remove(linea.begin(), linea.end(), '\r'), linea.end());
    linea.erase(std::remove(linea.begin(), linea.end(), '\n'), linea.end());
    return linea;
}


void enviarTexto(SOCKET comm_socket, const std::string& texto) {
    send(comm_socket, texto.c_str(), texto.size(), 0);
}

void mostrarMenuConsulta(SOCKET comm_socket) {
    std::vector<Accion> acciones = cargarAccionesDesdeFichero("data/stock_info.txt");
    if (acciones.empty()) {
        enviarTexto(comm_socket, "No se pudo cargar la información de acciones.\n");
        return;
    }

    bool salir = false;
    while (!salir) {
        std::string menu = "\n--- MENU CONSULTA ---\n"
                           "1) Consultar nombre acción (por ticker)\n"
                           "2) Consultar ticker acción (por nombre)\n"
                           "3) Volver al menú principal\n"
                           "Selecciona opción: ";
        enviarTexto(comm_socket, menu);

        std::string opcion = recibirLinea(comm_socket);
        if (opcion.empty()) {
            salir = true;
            break;
        }

        if (opcion == "1") {
            enviarTexto(comm_socket, "Introduce ticker: ");
            std::string tickerBuscado = recibirLinea(comm_socket);
            if (tickerBuscado.empty()) {
                enviarTexto(comm_socket, "Entrada vacía. Volviendo al menú consulta.\n");
                continue;
            }

            bool encontrado = false;
            for (const auto& acc : acciones) {
                if (acc.getTicker() == tickerBuscado) {
                    enviarTexto(comm_socket, "El ticker " + tickerBuscado + " corresponde a: " + acc.getNombre() + "\n");
                    encontrado = true;
                    break;
                }
            }
            if (!encontrado) {
                enviarTexto(comm_socket, "Error: No se encontró ninguna acción con ese ticker.\n");
            }

        } else if (opcion == "2") {
            enviarTexto(comm_socket, "Introduce nombre acción: ");
            std::string nombreBuscado = recibirLinea(comm_socket);
            if (nombreBuscado.empty()) {
                enviarTexto(comm_socket, "Entrada vacía. Volviendo al menú consulta.\n");
                continue;
            }

            bool encontrado = false;
            for (const auto& acc : acciones) {
                if (acc.getNombre() == nombreBuscado) {
                    enviarTexto(comm_socket, "La acción \"" + nombreBuscado + "\" tiene ticker: " + acc.getTicker() + "\n");
                    encontrado = true;
                    break;
                }
            }
            if (!encontrado) {
                enviarTexto(comm_socket, "Error: No se encontró ninguna acción con ese nombre.\n");
            }

        } else if (opcion == "3") {
        	salir = true;

        } else {
            enviarTexto(comm_socket, "Opción inválida. Intenta de nuevo.\n");
        }
    }
}
