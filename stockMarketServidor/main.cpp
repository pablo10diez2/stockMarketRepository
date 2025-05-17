/*
 * main.c
 *
 *  Created on: 16 abr 2025
 *      Author: pablo.diez
 */


#include <iostream>
#include <winsock2.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6000

int main() {
    WSADATA wsaData;
    SOCKET conn_socket, comm_socket;
    sockaddr_in server{}, client{};
    char sendBuff[512], recvBuff[512];

    std::cout << "\nInitialising Winsock...\n";
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed. Error Code: " << WSAGetLastError() << '\n';
        return -1;
    }

    std::cout << "Initialised.\n";

    // Crear socket
    conn_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_socket == INVALID_SOCKET) {
        std::cerr << "Could not create socket: " << WSAGetLastError() << '\n';
        WSACleanup();
        return -1;
    }

    std::cout << "Socket created.\n";

    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    // Asociar IP/puerto al socket
    if (bind(conn_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error code: " << WSAGetLastError() << '\n';
        closesocket(conn_socket);
        WSACleanup();
        return -1;
    }

    std::cout << "Bind done.\n";

    // Escuchar conexiones entrantes
    if (listen(conn_socket, 1) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error code: " << WSAGetLastError() << '\n';
        closesocket(conn_socket);
        WSACleanup();
        return -1;
    }

    std::cout << "Waiting for incoming connections...\n";
    int stsize = sizeof(sockaddr);
    comm_socket = accept(conn_socket, (sockaddr*)&client, &stsize);
    if (comm_socket == INVALID_SOCKET) {
        std::cerr << "Accept failed with error code: " << WSAGetLastError() << '\n';
        closesocket(conn_socket);
        WSACleanup();
        return -1;
    }

    std::cout << "Incoming connection from: " << inet_ntoa(client.sin_addr)
              << " (" << ntohs(client.sin_port) << ")\n";

    // Cerrar el socket de escucha
    closesocket(conn_socket);

    std::cout << "Waiting for incoming messages from client...\n";

    while (true) {
        int bytes = recv(comm_socket, recvBuff, sizeof(recvBuff), 0);
        if (bytes <= 0) break;

        recvBuff[bytes] = '\0';
        std::string option(recvBuff);

        if (option == "1") {
            std::cout << "Client chose to login.\n";

            send(comm_socket, "Enter username:", 16, 0);
            bytes = recv(comm_socket, recvBuff, sizeof(recvBuff), 0);
            recvBuff[bytes] = '\0';
            std::string username(recvBuff);

            send(comm_socket, "Enter password:", 16, 0);
            bytes = recv(comm_socket, recvBuff, sizeof(recvBuff), 0);
            recvBuff[bytes] = '\0';
            std::string password(recvBuff);

            // Aquí deberías validar las credenciales (ej. desde archivo)
            std::cout << "Received login: " << username << " / " << password << "\n";
            send(comm_socket, "Login received.\n", 17, 0);

        } else if (option == "2") {
            std::cout << "Client chose to register.\n";

            send(comm_socket, "Choose a username:", 19, 0);
            bytes = recv(comm_socket, recvBuff, sizeof(recvBuff), 0);
            recvBuff[bytes] = '\0';
            std::string newUser(recvBuff);

            send(comm_socket, "Choose a password:", 19, 0);
            bytes = recv(comm_socket, recvBuff, sizeof(recvBuff), 0);
            recvBuff[bytes] = '\0';
            std::string newPass(recvBuff);

            // Aquí podrías guardar el usuario en un archivo o base de datos
            std::cout << "New user registered: " << newUser << " / " << newPass << "\n";
            send(comm_socket, "Registration completed.\n", 25, 0);

        } else if (option == "3") {
            std::cout << "Client chose to exit.\n";
            send(comm_socket, "Goodbye!\n", 9, 0);
            break;

        } else {
            send(comm_socket, "Invalid option.\n", 16, 0);
        }
    }


    closesocket(comm_socket);
    WSACleanup();
    return 0;
}
