#ifndef PROGRAMA_CPP_
#define PROGRAMA_CPP_

#include "Programa.h"
#include <algorithm>
#include <string>
#include <winsock2.h>
#include "MenuCuenta.h"
#include "logs.h"
#include "Usuario.h"
#include "funcionesBD.h"

bool mostrarMenuPrincipal(SOCKET comm_socket, Usuario usuario) {
    char recvBuff[512];
    int bytes;
    bool sesionActiva = true;

    try {
        escribirLog("Usuario " + usuario.getEmail() + " accedió al menú principal");
    } catch (const std::exception& e) {}

    while (sesionActiva) {
        std::string subMenu = "\n=== MENU PRINCIPAL ===\n1) Consulta\n2) Orden\n3) Cuenta\n4) Salir\nSeleccione una opción: ";

        int sendResult = send(comm_socket, subMenu.c_str(), subMenu.size(), 0);
        if (sendResult == SOCKET_ERROR) {
            return false;
        }

        bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
        if (bytes <= 0) {
            try {
                escribirLog("Conexión perdida en menú principal con usuario: " + usuario.getEmail());
            } catch (const std::exception& e) {}
            return false;
        }

        recvBuff[bytes] = '\0';
        std::string subOption = limpiarInput(recvBuff);
        std::string debugMsg = "DEBUG: Recibido [" + subOption + "] (longitud: " + std::to_string(subOption.length()) + ")\n";

        if (subOption == "1") {
            std::string msg = "Opción seleccionada: Consulta\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
        } else if (subOption == "2") {
            std::string msg = "Opción seleccionada: Orden\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
        } else if (subOption == "3") {
            std::string msg = "Accediendo a gestión de cuenta...\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);

            try {
                MenuCuenta menuCuenta(comm_socket, usuario.getEmail());
                bool salirCompleto = menuCuenta.mostrarMenu();
                if (salirCompleto) {
                    return true;
                }
            } catch (const std::exception& e) {
                std::string errorMsg = "Error en el menú de cuenta: " + std::string(e.what()) + "\n";
                send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
            }
        } else if (subOption == "4") {
            std::string msg = "Adiós. Gracias por usar nuestro servicio.\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            try {
                escribirLog("Usuario " + usuario.getEmail() + " cerró sesión");
            } catch (const std::exception& e) {}
            return true;
        } else {
            std::string msg = "Opción inválida. Intente de nuevo.\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
        }
    }

    return false;
}

#endif
