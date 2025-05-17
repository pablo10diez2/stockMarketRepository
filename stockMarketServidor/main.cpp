#include <iostream>
#include <winsock2.h>
#include <string>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6000

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

    std::cout << "Waiting for incoming connection...\n";
    int client_size = sizeof(sockaddr);
    comm_socket = accept(conn_socket, (sockaddr*)&client, &client_size);
    if (comm_socket == INVALID_SOCKET) {
        std::cerr << "Accept failed. Error: " << WSAGetLastError() << '\n';
        closesocket(conn_socket);
        WSACleanup();
        return -1;
    }

    std::cout << "Client connected: " << inet_ntoa(client.sin_addr)
              << ":" << ntohs(client.sin_port) << "\n";

    closesocket(conn_socket);

    // Enviar menú
    std::string menu = "===== MENU =====\n1. Iniciar sesion\n2. Registrarse\n3. Salir\nSeleccione una opcion: ";
    send(comm_socket, menu.c_str(), menu.size(), 0);

    // Recibir opción
    int bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (bytes <= 0) {
        std::cerr << "Client disconnected unexpectedly.\n";
        closesocket(comm_socket);
        WSACleanup();
        return 0;
    }

    recvBuff[bytes] = '\0';
    std::string option(recvBuff);

    if (option == "1") {
        std::cout << "Client selected login.\n";

        send(comm_socket, "Enter username: ", 17, 0);
        bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
        recvBuff[bytes] = '\0';
        std::string username(recvBuff);

        send(comm_socket, "Enter password: ", 17, 0);
        bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
        recvBuff[bytes] = '\0';
        std::string password(recvBuff);

        std::cout << "Login attempt: " << username << " / " << password << "\n";

        // Aquí puedes validar con la base de datos
        send(comm_socket, "Login recibido.\n", 17, 0);

    } else if (option == "2") {
        std::cout << "Client selected register.\n";

        send(comm_socket, "Choose username: ", 18, 0);
        bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
        recvBuff[bytes] = '\0';
        std::string newUser(recvBuff);

        send(comm_socket, "Choose password: ", 18, 0);
        bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
        recvBuff[bytes] = '\0';
        std::string newPass(recvBuff);

        std::cout << "New user: " << newUser << " / " << newPass << "\n";

        // Aquí puedes registrar el usuario en la base de datos
        send(comm_socket, "Registro completado.\n", 23, 0);

    } else if (option == "3") {
        std::cout << "Client chose to exit.\n";
        send(comm_socket, "Adiós.\n", 7, 0);
    } else {
        std::cout << "Opción inválida recibida.\n";
        send(comm_socket, "Opción inválida.\n", 17, 0);
    }

    closesocket(comm_socket);
    WSACleanup();
    return 0;
}
