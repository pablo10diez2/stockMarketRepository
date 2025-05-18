#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include "funcionesBD.h"
#include "MenuCuenta.h"
#include "sqlite3.h"
#include "Usuario.h"
#include "logs.h"
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6000

// Declaración de funciones
bool mostrarMenuPrincipal(SOCKET comm_socket, Usuario usuario);
Usuario cargarUsuarioDesdeBD(const std::string& email);

// Función para cargar los datos completos del usuario desde la BD
Usuario cargarUsuarioDesdeBD(const std::string& email) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    Usuario usuario; // Crear un usuario vacío por defecto

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::cerr << "No se pudo abrir la base de datos: " << sqlite3_errmsg(db) << '\n';
        return usuario;
    }

    const char* sql = "SELECT Nombre_Usuario, Apellido_Usuario, Email, Contrasena, ID_Rol, Dinero "
                      "FROM Usuario WHERE Email = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Fallo al preparar la consulta: " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return usuario;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        std::string nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string apellido = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string dbEmail = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int id_rol = sqlite3_column_int(stmt, 4);
        double dinero = sqlite3_column_double(stmt, 5);

        // Crear un nuevo objeto Usuario con todos los datos
        usuario = Usuario(nombre, apellido, dbEmail, password, id_rol, dinero);
        std::cout << "Usuario cargado: " << nombre << " " << apellido << " (Email: " << dbEmail << ")\n";
    } else {
        std::cerr << "No se encontró al usuario con email: " << email << "\n";
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return usuario;
}

// Función para mostrar el menú principal
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

            // Usar la nueva clase MenuCuenta con el email del usuario
            MenuCuenta menuCuenta(comm_socket, usuario.getEmail());
            if (menuCuenta.mostrarMenu()) {
                // Si el usuario eligió salir completamente
                return true;
            }
        } else if (subOption == "4") {
            std::string msg = "Adiós. Gracias por usar nuestro servicio.\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            escribirLog("Usuario " + usuario.getEmail() + " cerró sesión");
            return true;  // Salir completamente (como la opción 3 del menú inicial)
        } else {
            std::string msg = "Opción inválida. Intente de nuevo.\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
        }
    }

    return false;
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

    // Registro de inicio del servidor
    escribirLog("Servidor iniciado en " + std::string(SERVER_IP) + ":" + std::to_string(SERVER_PORT));

    conn_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed. Error: " << WSAGetLastError() << '\n';
        escribirLog("Error al crear el socket: " + std::to_string(WSAGetLastError()));
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
        escribirLog("Nueva conexión desde: " + std::string(inet_ntoa(client.sin_addr)) + ":" + std::to_string(ntohs(client.sin_port)));

        bool clienteActivo = true;

        while (clienteActivo) {
            std::string menu = "===== MENU =====\n1. Iniciar sesion\n2. Registrarse\n3. Salir\nSeleccione una opcion: ";
            send(comm_socket, menu.c_str(), menu.size(), 0);

            int bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
            if (bytes <= 0) {
                std::cerr << "Client disconnected unexpectedly.\n";
                escribirLog("Cliente desconectado inesperadamente en el menú principal");
                break;
            }

            recvBuff[bytes] = '\0';
            std::string option(recvBuff);

            if (option == "1") {
                std::cout << "Client selected login.\n";
                escribirLog("Cliente seleccionó iniciar sesión");

                send(comm_socket, "Enter email: ", 15, 0);
                bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
                if (bytes <= 0) {
                    escribirLog("Cliente desconectado durante el inicio de sesión");
                    break;
                }
                recvBuff[bytes] = '\0';
                std::string email(recvBuff);

                send(comm_socket, "Enter password: ", 17, 0);
                bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
                if (bytes <= 0) {
                    escribirLog("Cliente desconectado durante el inicio de sesión");
                    break;
                }
                recvBuff[bytes] = '\0';
                std::string password(recvBuff);

                std::cout << "Login attempt: " << email << "\n";

                try {
                    bool loginSuccess = iniciarSesion(email, password);

                    if (loginSuccess) {
                        send(comm_socket, "Login exitoso.\n", 15, 0);
                        escribirLog("Inicio de sesión exitoso para el usuario: " + email);

                        // Cargar los datos completos del usuario desde la BD
                        Usuario usuario = cargarUsuarioDesdeBD(email);
                        if (usuario.getEmail().empty()) {
                            escribirLog("Error al cargar datos de usuario: " + email);
                            send(comm_socket, "Error al cargar datos de usuario.\n", 33, 0);
                            continue;
                        }

                        escribirLog("Usuario " + email + " ha iniciado sesión.");

                        // Mostrar el menú principal con el objeto Usuario completo
                        if (mostrarMenuPrincipal(comm_socket, usuario)) {
                            clienteActivo = false;
                        }
                    } else {
                        send(comm_socket, "Email o contraseña incorrectos.\n", 32, 0);
                        escribirLog("Intento fallido de inicio de sesión para: " + email);
                    }
                } catch (const std::exception& e) {
                    std::string errorMsg = "Error en inicio de sesión: ";
                    errorMsg += e.what();
                    escribirLog(errorMsg);
                    send(comm_socket, "Error en el servidor. Intente nuevamente.\n", 42, 0);
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
                    escribirLog("Nuevo usuario registrado: " + email);
                    send(comm_socket, "Registro completado exitosamente.\n", 34, 0);

                    // Crear el objeto usuario directamente ya que tenemos todos los datos
                    Usuario usuario(nombre, apellido, email, password, id_rol, 0);

                    // Mostrar el menú principal con el objeto Usuario completo
                    if (mostrarMenuPrincipal(comm_socket, usuario)) {
                        clienteActivo = false;
                    }
                } else {
                    send(comm_socket, "Error al registrar. Posiblemente el email ya existe.\n", 53, 0);
                    escribirLog("Intento fallido de registro para email: " + email);
                }

            } else if (option == "3") {
                std::cout << "Client chose to exit.\n";
                send(comm_socket, "Adiós. Gracias por usar nuestro servicio.\n", 42, 0);
                escribirLog("Cliente eligió salir desde el menú principal");
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
