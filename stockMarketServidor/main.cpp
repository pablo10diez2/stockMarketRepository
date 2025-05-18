#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include "funcionesBD.h"
#include "sqlite3.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6000

void mostrarMenuPrincipal(SOCKET comm_socket) {
    char recvBuff[512];
    int bytes;
    bool sesionActiva = true;

    while (sesionActiva) {
        std::string subMenu = "\n=== MENU PRINCIPAL ===\n1) Consulta\n2) Orden\n3) Cuenta\n4) Salir\nSeleccione una opción: ";
        send(comm_socket, subMenu.c_str(), subMenu.size(), 0);

        bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
        if (bytes <= 0) break;
        recvBuff[bytes] = '\0';
        std::string subOption(recvBuff);

        if (subOption == "1") {
            std::string msg = "Opción seleccionada: Consulta\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
        } else if (subOption == "2") {
            std::string msg = "Opción seleccionada: Orden\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
        } else if (subOption == "3") {
            std::string msg = "Opción seleccionada: Cuenta\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
        } else if (subOption == "4") {
            std::string msg = "Saliendo del menú principal...\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            sesionActiva = false;
        } else {
            std::string msg = "Opción inválida. Intente de nuevo.\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET conn_socket, comm_socket;
    sockaddr_in server{}, client{};
    char recvBuff[512];

    std::cout << "\nInitialising Winsock...\n";
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed. Error: " << WSAGetLastError() << '\n';
        return -1;
    }

    conn_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed. Error: " << WSAGetLastError() << '\n';
        WSACleanup();
        return -1;
    }

    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    if (bind(conn_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        std::cerr << "Bind failed. Error: " << WSAGetLastError() << '\n';
        closesocket(conn_socket);
        WSACleanup();
        return -1;
    }

    if (listen(conn_socket, 1) == SOCKET_ERROR) {
        std::cerr << "Listen failed. Error: " << WSAGetLastError() << '\n';
        closesocket(conn_socket);
        WSACleanup();
        return -1;
    }

    while (true) {
        std::cout << "Waiting for incoming connection...\n";
        int client_size = sizeof(sockaddr);
        comm_socket = accept(conn_socket, (sockaddr*)&client, &client_size);
        if (comm_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed. Error: " << WSAGetLastError() << '\n';
            continue;
        }

        std::cout << "Client connected: " << inet_ntoa(client.sin_addr)
                  << ":" << ntohs(client.sin_port) << "\n";

        bool clienteActivo = true;

        while (clienteActivo) {
            std::string menu = "===== MENU =====\n1. Iniciar sesion\n2. Registrarse\n3. Salir\nSeleccione una opcion: ";
            send(comm_socket, menu.c_str(), menu.size(), 0);

            int bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
            if (bytes <= 0) {
                std::cerr << "Client disconnected unexpectedly.\n";
                break;
            }

            recvBuff[bytes] = '\0';
            std::string option(recvBuff);

            if (option == "1") {
                std::cout << "Client selected login.\n";

                send(comm_socket, "Enter email: ", 15, 0);
                bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
                if (bytes <= 0) break;
                recvBuff[bytes] = '\0';
                std::string email(recvBuff);

                send(comm_socket, "Enter password: ", 17, 0);
                bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
                if (bytes <= 0) break;
                recvBuff[bytes] = '\0';
                std::string password(recvBuff);

                std::cout << "Login attempt: " << email << "\n";

                bool loginSuccess = iniciarSesion(email, password);

                if (loginSuccess) {
                    send(comm_socket, "Login exitoso.\n", 15, 0);
                    mostrarMenuPrincipal(comm_socket);
                } else {
                    send(comm_socket, "Email o contraseña incorrectos.\n", 32, 0);
                }

            } else if (option == "2") {
                std::cout << "Client selected register.\n";

                send(comm_socket, "Ingrese nombre: ", 16, 0);
                bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
                if (bytes <= 0) break;
                recvBuff[bytes] = '\0';
                std::string nombre(recvBuff);

                send(comm_socket, "Ingrese apellido: ", 18, 0);
                bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
                if (bytes <= 0) break;
                recvBuff[bytes] = '\0';
                std::string apellido(recvBuff);

                send(comm_socket, "Ingrese email: ", 15, 0);
                bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
                if (bytes <= 0) break;
                recvBuff[bytes] = '\0';
                std::string email(recvBuff);

                send(comm_socket, "Ingrese contraseña: ", 20, 0);
                bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
                if (bytes <= 0) break;
                recvBuff[bytes] = '\0';
                std::string password(recvBuff);

                int id_rol = 2;

                std::cout << "Registro de nuevo usuario: " << nombre << " " << apellido << " / " << email << "\n";

                bool registroSuccess = registrarUsuario(nombre, apellido, email, password, id_rol);

                if (registroSuccess) {
                    send(comm_socket, "Registro completado exitosamente.\n", 34, 0);
                    mostrarMenuPrincipal(comm_socket);
                } else {
                    send(comm_socket, "Error al registrar. Posiblemente el email ya existe.\n", 53, 0);
                }

            } else if (option == "3") {
                std::cout << "Client chose to exit.\n";
                send(comm_socket, "Adiós. Gracias por usar nuestro servicio.\n", 42, 0);
                clienteActivo = false;
            } else {
                std::cout << "Opción inválida recibida: " << option << "\n";
                send(comm_socket, "Opción inválida. Por favor, intente de nuevo.\n", 45, 0);
            }
        }

        closesocket(comm_socket);
        std::cout << "Client disconnected.\n";
    }

    closesocket(conn_socket);
    WSACleanup();
    return 0;
}
