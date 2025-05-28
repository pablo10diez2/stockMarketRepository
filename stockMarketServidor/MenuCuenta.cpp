#include "MenuCuenta.h"
#include "sqlite3.h"
#include "logs.h"
#include <string>
#include <winsock2.h>
#include "funcionesBD.h"
#include <algorithm>

MenuCuenta::MenuCuenta(SOCKET socket, const std::string& email) : comm_socket(socket), email_usuario(email) {}

bool MenuCuenta::mostrarMenu() {
    char recvBuff[512];
    int bytes;
    bool menuActivo = true;
    bool salirCompleto = false;

    try {
        escribirLog("Usuario " + email_usuario + " accedió al menú de cuenta");
    } catch (...) {}

    while (menuActivo) {
        std::string menuOpciones = "\n=== MENU CUENTA ===\n1) Ver perfil\n2) Cambiar contrasena\n3) Introducir fondos\n4) Volver\n5) Salir\nSeleccione una opción: ";
        int sendResult = send(comm_socket, menuOpciones.c_str(), menuOpciones.size(), 0);
        if (sendResult == SOCKET_ERROR) return true;
        Sleep(100);

        bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
        if (bytes <= 0) return true;

        recvBuff[bytes] = '\0';
        std::string opcion= limpiarInput(recvBuff);
        opcion.erase(std::remove(opcion.begin(), opcion.end(), '\r'), opcion.end());
        opcion.erase(std::remove(opcion.begin(), opcion.end(), '\n'), opcion.end());

        if (opcion == "1") {
            std::string msg = "Opción seleccionada: Ver perfil\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100);
            verPerfil();
        } else if (opcion == "2") {
            std::string msg = "Opción seleccionada: Cambiar contraseña\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100);
            cambiarContrasena();
        } else if (opcion == "3") {
            std::string msg = "Opción seleccionada: Introducir fondos\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100);
            introducirFondos();
        } else if (opcion == "4") {
            std::string msg = "Volviendo al menú principal...\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100);
            menuActivo = false;
        } else if (opcion == "5") {
            std::string msg = "Saliendo del sistema. ¡Hasta pronto!\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100);
            menuActivo = false;
            salirCompleto = true;
        } else {
            std::string msg = "Opción inválida. Intente de nuevo.\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100);
        }
    }

    return salirCompleto;
}

void MenuCuenta::verPerfil() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::string errorMsg = "Error al conectar con la base de datos.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    const char* sql = "SELECT Nombre_Usuario, Apellido_Usuario, Email, ID_Rol, Dinero FROM Usuario WHERE Email = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la consulta.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, email_usuario.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        std::string nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string apellido = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int rol = sqlite3_column_int(stmt, 3);
        double dinero = sqlite3_column_double(stmt, 4);
        std::string rolTexto = (rol == 1) ? "Administrador" : "Usuario";

        std::string perfil = "\n=== PERFIL DE USUARIO ===\n";
        perfil += "Nombre: " + nombre + "\n";
        perfil += "Apellido: " + apellido + "\n";
        perfil += "Email: " + email + "\n";
        perfil += "Rol: " + rolTexto + "\n";
        perfil += "Dinero disponible: " + std::to_string(dinero) + " €\n";

        send(comm_socket, perfil.c_str(), perfil.size(), 0);
        Sleep(100);

        try {
            escribirLog("Usuario " + email_usuario + " consultó su perfil");
        } catch (...) {}
    } else {
        std::string errorMsg = "No se encontró información del usuario.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void MenuCuenta::cambiarContrasena() {
    char recvBuff[512];
    int bytes;

    std::string msg = "Introduzca la nueva contraseña: ";
    send(comm_socket, msg.c_str(), msg.size(), 0);
    Sleep(100);

    bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (bytes <= 0) return;

    recvBuff[bytes] = '\0';
    std::string nuevaContrasena(recvBuff);
    nuevaContrasena.erase(std::remove(nuevaContrasena.begin(), nuevaContrasena.end(), '\r'), nuevaContrasena.end());
    nuevaContrasena.erase(std::remove(nuevaContrasena.begin(), nuevaContrasena.end(), '\n'), nuevaContrasena.end());

    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::string errorMsg = "Error al conectar con la base de datos.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    const char* sql = "UPDATE Usuario SET Contrasena = ? WHERE Email = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la consulta.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, nuevaContrasena.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email_usuario.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        std::string exito = "Contraseña actualizada correctamente.\n";
        send(comm_socket, exito.c_str(), exito.size(), 0);
        Sleep(100);

        try {
            escribirLog("Usuario " + email_usuario + " cambió su contraseña");
        } catch (...) {}
    } else {
        std::string errorMsg = "Error al actualizar la contraseña.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void MenuCuenta::introducirFondos() {
    char recvBuff[512];
    int bytes;

    std::string msg = "Introduce tu número de cuenta: ";
    send(comm_socket, msg.c_str(), msg.size(), 0);
    Sleep(100);

    bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (bytes <= 0) return;

    recvBuff[bytes] = '\0';
    std::string numeroCuenta(recvBuff);
    numeroCuenta.erase(std::remove(numeroCuenta.begin(), numeroCuenta.end(), '\r'), numeroCuenta.end());
    numeroCuenta.erase(std::remove(numeroCuenta.begin(), numeroCuenta.end(), '\n'), numeroCuenta.end());

    msg = "Introduce la cantidad de fondos a añadir: ";
    send(comm_socket, msg.c_str(), msg.size(), 0);
    Sleep(100);

    // Resto de lógica no incluida en tu mensaje original
}
