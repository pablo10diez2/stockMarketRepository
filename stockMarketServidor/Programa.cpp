#ifndef PROGRAMA_CPP_
#define PROGRAMA_CPP_

#include "Programa.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <winsock2.h>
#include "MenuCuenta.h"
#include "menuConsulta.h"
#include "logs.h"
#include "Usuario.h"
#include "MenuOrden.h"

bool mostrarMenuPrincipal(SOCKET comm_socket, Usuario usuario) {
    char recvBuff[512];
    int bytes;
    bool sesionActiva = true;


    std::cout << "Entrando en mostrarMenuPrincipal para usuario: " << usuario.getEmail() << std::endl;
    try {
        escribirLog("Usuario " + usuario.getEmail() + " accedió al menú principal");
    } catch (const std::exception& e) {
        std::cout << "Error al escribir log: " << e.what() << std::endl;
    }

    while (sesionActiva) {
        std::string subMenu = "\n=== MENU PRINCIPAL ===\n1) Consulta\n2) Orden\n3) Cuenta\n4) Salir\nSeleccione una opción: ";


        std::cout << "\nMostrando al cliente:\n" << subMenu << std::endl;

        int sendResult = send(comm_socket, subMenu.c_str(), subMenu.size(), 0);
        if (sendResult == SOCKET_ERROR) {
            std::cout << "Error al enviar menú principal: " << WSAGetLastError() << std::endl;
            return false;
        }

        std::cout << "Esperando respuesta del cliente..." << std::endl;
        bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
        if (bytes <= 0) {
            std::cout << "Cliente desconectado durante menú principal. Error: " << WSAGetLastError() << std::endl;
            try {
                escribirLog("Conexión perdida en menú principal con usuario: " + usuario.getEmail());
            } catch (const std::exception& e) {
                std::cout << "Error al escribir log: " << e.what() << std::endl;
            }
            return false;
        }

        recvBuff[bytes] = '\0';
        std::string subOption(recvBuff);

        subOption.erase(std::remove(subOption.begin(), subOption.end(), '\r'), subOption.end());
        subOption.erase(std::remove(subOption.begin(), subOption.end(), '\n'), subOption.end());

        std::cout << "Opción seleccionada: '" << subOption << "'" << std::endl;

        if (subOption == "1") {
            std::string msg = "Opción seleccionada: Consulta\n";
            std::cout << "Enviando: " << msg;
            send(comm_socket, msg.c_str(), msg.size(), 0);

            mostrarMenuConsulta(comm_socket);

        } else if (subOption == "2") {
            std::string msg = "Opción seleccionada: Orden\n";
            std::cout << "Enviando: " << msg;
            send(comm_socket, msg.c_str(), msg.size(), 0);

            Sleep(100);

            try {
                MenuOrden menuOrden(comm_socket, usuario);
                bool salirCompleto = menuOrden.mostrarMenu();
                if (salirCompleto) {
                    std::cout << "Usuario eligió salir completamente desde el menú de órdenes" << std::endl;
                    return true;
                }
            } catch (const std::exception& e) {
                std::cout << "Excepción en MenuOrden: " << e.what() << std::endl;
                std::string errorMsg = "Error en el menú de órdenes: " + std::string(e.what()) + "\n";
                send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
            }
        } else if (subOption == "3") {
            std::cout << "Usuario seleccionó opción 3: Cuenta" << std::endl;
            std::string msg = "Accediendo a gestión de cuenta...\n";
            std::cout << "Enviando: " << msg;
            send(comm_socket, msg.c_str(), msg.size(), 0);


            Sleep(100);

            try {
                std::cout << "Creando instancia de MenuCuenta para: " << usuario.getEmail() << std::endl;
                MenuCuenta menuCuenta(comm_socket, usuario.getEmail());
                std::cout << "Llamando a menuCuenta.mostrarMenu()" << std::endl;

                bool salirCompleto = menuCuenta.mostrarMenu();
                std::cout << "Regresando de MenuCuenta. salirCompleto=" << (salirCompleto ? "true" : "false") << std::endl;

                if (salirCompleto) {
                    std::cout << "Usuario eligió salir completamente desde el menú de cuenta" << std::endl;
                    return true;
                }
            } catch (const std::exception& e) {
                std::cout << "Excepción en MenuCuenta: " << e.what() << std::endl;
                std::string errorMsg = "Error en el menú de cuenta: " + std::string(e.what()) + "\n";
                send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
            }
        } else if (subOption == "4") {
            std::string msg = "Adiós. Gracias por usar nuestro servicio.\n";
            std::cout << "Enviando: " << msg;
            send(comm_socket, msg.c_str(), msg.size(), 0);
            try {
                escribirLog("Usuario " + usuario.getEmail() + " cerró sesión");
            } catch (const std::exception& e) {
                std::cout << "Error al escribir log: " << e.what() << std::endl;
            }
            return true;
        } else {
            std::string msg = "Opción inválida. Intente de nuevo.\n";
            std::cout << "Enviando: " << msg;
            send(comm_socket, msg.c_str(), msg.size(), 0);
        }
    }

    return false;
}

#endif
