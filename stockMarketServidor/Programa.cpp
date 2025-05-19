
#ifndef PROGRAMA_CPP_
#define PROGRAMA_CPP_

#include "Programa.h"

bool mostrarMenuPrincipal(SOCKET comm_socket, Usuario usuario) {
    char recvBuff[512];
    int bytes;
    bool sesionActiva = true;

    while (sesionActiva) {
        std::string subMenu = "\n=== MENU PRINCIPAL ===\n1) Consulta\n2) Orden\n3) Cuenta\n4) Salir\nSeleccione una opción: ";
        send(comm_socket, subMenu.c_str(), subMenu.size(), 0);

        bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
        if (bytes <= 0) {
            escribirLog("Conexión perdida en menú principal con usuario: " + usuario.getEmail());
            return false;
        }
        recvBuff[bytes] = '\0';
        std::string subOption(recvBuff);

        if (subOption == "1") {
            std::string msg = "Opción seleccionada: Consulta\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
        } else if (subOption == "2") {
            std::string msg = "Opción seleccionada: Orden\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
        } else if (subOption == "3") {
            std::string msg = "Accediendo a gestión de cuenta...\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);

            MenuCuenta menuCuenta(comm_socket, usuario.getEmail());
            if (menuCuenta.mostrarMenu()) {
                return true;
            }
        } else if (subOption == "4") {
            std::string msg = "Adiós. Gracias por usar nuestro servicio.\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            escribirLog("Usuario " + usuario.getEmail() + " cerró sesión");
            return true;
        } else {
            std::string msg = "Opción inválida. Intente de nuevo.\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
        }
    }

    return false;
}


#endif
