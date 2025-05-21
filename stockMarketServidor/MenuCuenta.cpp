#include "MenuCuenta.h"
#include "sqlite3.h"
#include "logs.h"
#include <iostream>
#include <string>
#include <winsock2.h>
#include <algorithm>  // Para std::remove

MenuCuenta::MenuCuenta(SOCKET socket, const std::string& email) : comm_socket(socket), email_usuario(email) {
    std::cout << "MenuCuenta creado para usuario: " << email << std::endl;
}

bool MenuCuenta::mostrarMenu() {
    char recvBuff[512];
    int bytes;
    bool menuActivo = true;
    bool salirCompleto = false;

    std::cout << "Entrando en MenuCuenta::mostrarMenu() para usuario: " << email_usuario << std::endl;

    try {
        escribirLog("Usuario " + email_usuario + " accedió al menú de cuenta");
    } catch (const std::exception& e) {
        std::cout << "Error al escribir log: " << e.what() << std::endl;
    }

    while (menuActivo) {
        std::string menuOpciones = "\n=== MENU CUENTA ===\n1) Ver perfil\n2) Cambiar contrasena\n3) Introducir fondos\n4) Volver\n5) Salir\nSeleccione una opción: ";

        // Display menu on server console as well
        std::cout << "\nMostrando al cliente el menú de cuenta:\n" << menuOpciones << std::endl;

        int sendResult = send(comm_socket, menuOpciones.c_str(), menuOpciones.size(), 0);
        if (sendResult == SOCKET_ERROR) {
            std::cout << "Error al enviar menú cuenta: " << WSAGetLastError() << std::endl;
            return true; // Salir completamente en caso de error
        }

        // Asegurar que el mensaje llegue antes de esperar respuesta
        Sleep(100);

        std::cout << "Esperando respuesta del cliente en MenuCuenta..." << std::endl;
        bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);

        if (bytes <= 0) {
            std::cout << "El cliente se desconectó durante el menú de cuenta. Error: " << WSAGetLastError() << std::endl;
            return true; // Salir completamente si hay desconexión
        }

        recvBuff[bytes] = '\0';
        std::string opcion(recvBuff);
        // Limpiar caracteres de nueva línea y retorno de carro
        opcion.erase(std::remove(opcion.begin(), opcion.end(), '\r'), opcion.end());
        opcion.erase(std::remove(opcion.begin(), opcion.end(), '\n'), opcion.end());

        std::cout << "Opción recibida en menú cuenta: '" << opcion << "'" << std::endl;

        if (opcion == "1") {
            std::cout << "Usuario seleccionó: Ver perfil" << std::endl;
            // Enviar esta información al cliente también
            std::string msg = "Opción seleccionada: Ver perfil\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100); // Asegurar que el mensaje llegue

            verPerfil();

            // No necesitamos volver a mostrar el menú aquí, ya que el bucle while se encargará de eso
        } else if (opcion == "2") {
            std::cout << "Usuario seleccionó: Cambiar contraseña" << std::endl;
            // Enviar esta información al cliente también
            std::string msg = "Opción seleccionada: Cambiar contraseña\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100); // Asegurar que el mensaje llegue

            cambiarContrasena();

            // No necesitamos volver a mostrar el menú aquí, ya que el bucle while se encargará de eso
        } else if (opcion == "3") {
            std::cout << "Usuario seleccionó: Introducir fondos" << std::endl;
            // Enviar esta información al cliente también
            std::string msg = "Opción seleccionada: Introducir fondos\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100); // Asegurar que el mensaje llegue

            introducirFondos();

            // No necesitamos volver a mostrar el menú aquí, ya que el bucle while se encargará de eso
        } else if (opcion == "4") {
            std::cout << "Usuario seleccionó: Volver al menú principal" << std::endl;
            std::string msg = "Volviendo al menú principal...\n";
            std::cout << "Enviando: " << msg;
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100); // Asegurar que el mensaje llegue
            menuActivo = false;
        } else if (opcion == "5") {
            std::cout << "Usuario seleccionó: Salir del sistema" << std::endl;
            std::string msg = "Saliendo del sistema. ¡Hasta pronto!\n";
            std::cout << "Enviando: " << msg;
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100); // Asegurar que el mensaje llegue
            menuActivo = false;
            salirCompleto = true;
        } else {
            std::cout << "Usuario introdujo opción inválida: " << opcion << std::endl;
            std::string msg = "Opción inválida. Intente de nuevo.\n";
            std::cout << "Enviando: " << msg;
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100); // Asegurar que el mensaje llegue
        }
    }

    std::cout << "Saliendo de MenuCuenta::mostrarMenu()" << std::endl;
    return salirCompleto;
}

void MenuCuenta::verPerfil() {
    std::cout << "Ejecutando verPerfil() para usuario: " << email_usuario << std::endl;

    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::string errorMsg = "Error al conectar con la base de datos.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    const char* sql = "SELECT Nombre_Usuario, Apellido_Usuario, Email, ID_Rol, Dinero "
                    "FROM Usuario WHERE Email = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la consulta.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
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

        // Display what's being sent to the client
        std::cout << "Enviando información de perfil al cliente:\n" << perfil << std::endl;

        send(comm_socket, perfil.c_str(), perfil.size(), 0);
        // Asegurar que el mensaje llegue
        Sleep(100);

        try {
            escribirLog("Usuario " + email_usuario + " consultó su perfil");
        } catch (const std::exception& e) {
            std::cout << "Error al escribir log: " << e.what() << std::endl;
        }
    } else {
        std::string errorMsg = "No se encontró información del usuario.\n";
        std::cout << "No se encontró el usuario en la base de datos" << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void MenuCuenta::cambiarContrasena() {
    std::cout << "Ejecutando cambiarContrasena() para usuario: " << email_usuario << std::endl;

    char recvBuff[512];
    int bytes;

    std::string msg = "Introduzca la nueva contraseña: ";
    std::cout << "Enviando: " << msg;
    send(comm_socket, msg.c_str(), msg.size(), 0);
    Sleep(100); // Asegurar que el mensaje llegue

    std::cout << "Esperando nueva contraseña del cliente..." << std::endl;
    bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (bytes <= 0) {
        std::cout << "Cliente desconectado durante cambio de contraseña" << std::endl;
        return;
    }
    recvBuff[bytes] = '\0';
    std::string nuevaContrasena(recvBuff);
    // Limpiar caracteres de nueva línea y retorno de carro
    nuevaContrasena.erase(std::remove(nuevaContrasena.begin(), nuevaContrasena.end(), '\r'), nuevaContrasena.end());
    nuevaContrasena.erase(std::remove(nuevaContrasena.begin(), nuevaContrasena.end(), '\n'), nuevaContrasena.end());

    std::cout << "Nueva contraseña recibida, actualizando en BD..." << std::endl;

    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::string errorMsg = "Error al conectar con la base de datos.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    const char* sql = "UPDATE Usuario SET Contrasena = ? WHERE Email = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la consulta.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, nuevaContrasena.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email_usuario.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        std::string exito = "Contraseña actualizada correctamente.\n";
        std::cout << "Enviando: " << exito;
        send(comm_socket, exito.c_str(), exito.size(), 0);
        Sleep(100); // Asegurar que el mensaje llegue

        try {
            escribirLog("Usuario " + email_usuario + " cambió su contraseña");
        } catch (const std::exception& e) {
            std::cout << "Error al escribir log: " << e.what() << std::endl;
        }
    } else {
        std::string errorMsg = "Error al actualizar la contraseña.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void MenuCuenta::introducirFondos() {
    std::cout << "Ejecutando introducirFondos() para usuario: " << email_usuario << std::endl;

    char recvBuff[512];
    int bytes;

    std::string msg = "Introduce tu número de cuenta: ";
    std::cout << "Enviando: " << msg;
    send(comm_socket, msg.c_str(), msg.size(), 0);
    Sleep(100); // Asegurar que el mensaje llegue

    std::cout << "Esperando número de cuenta del cliente..." << std::endl;
    bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (bytes <= 0) {
        std::cout << "Cliente desconectado durante introducción de fondos" << std::endl;
        return;
    }
    recvBuff[bytes] = '\0';
    std::string numeroCuenta(recvBuff);
    numeroCuenta.erase(std::remove(numeroCuenta.begin(), numeroCuenta.end(), '\r'), numeroCuenta.end());
    numeroCuenta.erase(std::remove(numeroCuenta.begin(), numeroCuenta.end(), '\n'), numeroCuenta.end());

    std::cout << "Número de cuenta recibido: " << numeroCuenta << std::endl;

    msg = "Introduce la cantidad de fondos a añadir: ";
    std::cout << "Enviando: " << msg;
    send(comm_socket, msg.c_str(), msg.size(), 0);
    Sleep(100); // Asegurar que el mensaje llegue

    std::cout << "Esperando cantidad de fondos del cliente..." << std::endl;
    bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (bytes <= 0) {
        std::cout << "Cliente desconectado durante introducción de fondos" << std::endl;
        return;
    }
    recvBuff[bytes] = '\0';
    std::string cantidadStr(recvBuff);
    cantidadStr.erase(std::remove(cantidadStr.begin(), cantidadStr.end(), '\r'), cantidadStr.end());
    cantidadStr.erase(std::remove(cantidadStr.begin(), cantidadStr.end(), '\n'), cantidadStr.end());

    std::cout << "Cantidad de fondos recibida: " << cantidadStr << std::endl;

    // Convertir a número y validar
    double cantidad = 0;
    try {
        cantidad = std::stod(cantidadStr);
        if (cantidad <= 0) {
            std::string errorMsg = "La cantidad debe ser un número positivo.\n";
            std::cout << "Enviando: " << errorMsg;
            send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
            return;
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Por favor, introduzca un valor numérico válido.\n";
        std::cout << "Enviando: " << errorMsg;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::string errorMsg = "Error al conectar con la base de datos.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    // Primero, obtener el dinero actual
    const char* sqlSelect = "SELECT Dinero FROM Usuario WHERE Email = ?";
    rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la consulta.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, email_usuario.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);

    double dineroActual = 0;
    if (rc == SQLITE_ROW) {
        dineroActual = sqlite3_column_double(stmt, 0);
        std::cout << "Dinero actual del usuario: " << dineroActual << std::endl;
    } else {
        std::string errorMsg = "No se pudo obtener la información de la cuenta.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    sqlite3_finalize(stmt);

    // Actualizar con el nuevo saldo
    double nuevoSaldo = dineroActual + cantidad;
    const char* sqlUpdate = "UPDATE Usuario SET Dinero = ? WHERE Email = ?";

    rc = sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la actualización.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_double(stmt, 1, nuevoSaldo);
    sqlite3_bind_text(stmt, 2, email_usuario.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        std::string confirmacion = "Se han añadido " + std::to_string(cantidad) +
                                " € a tu cuenta.\nNuevo saldo: " +
                                std::to_string(nuevoSaldo) + " €\n";
        std::cout << "Enviando: " << confirmacion;
        send(comm_socket, confirmacion.c_str(), confirmacion.size(), 0);
        Sleep(100); // Asegurar que el mensaje llegue

        try {
            escribirLog("Usuario " + email_usuario + " añadió " + std::to_string(cantidad) + " € a su cuenta");
        } catch (const std::exception& e) {
            std::cout << "Error al escribir log: " << e.what() << std::endl;
        }
    } else {
        std::string errorMsg = "Error al actualizar el saldo.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
