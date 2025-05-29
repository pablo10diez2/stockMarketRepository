#include "MenuOrden.h"
#include "Orden.h"
#include <vector>
#include <iostream>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <string>
#include <cstring>
#include "sqlite3.h"

MenuOrden::MenuOrden(SOCKET comm_socket, const Usuario& usuario)
    : comm_socket(comm_socket), usuario(usuario) {}

std::string obtenerFechaActual() {
    time_t now = time(nullptr);
    tm* tstruct = localtime(&now);
    char buf[80];
    if (tstruct) {
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tstruct);
        return std::string(buf);
    }
    return "";
}

void MenuOrden::mostrarOrdenesAnteriores() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    std::string resumen = "\n=== ÓRDENES ANTERIORES ===\n";

    if (sqlite3_open("JP.sqlite", &db) == SQLITE_OK) {
        std::string sql = "SELECT ID_Orden, TipoOrden, Ticker, Cantidad, Precio, Estado, Fecha_Creacion, Fecha_Ejecucion "
                          "FROM Orden WHERE Email = ? ORDER BY ID_Orden DESC";

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, usuario.getEmail().c_str(), -1, SQLITE_STATIC);

            bool hayOrdenes = false;
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                hayOrdenes = true;
                int id = sqlite3_column_int(stmt, 0);
                std::string tipo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                std::string ticker = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                double cantidad = sqlite3_column_double(stmt, 3);
                double precio = sqlite3_column_double(stmt, 4);
                int estado = sqlite3_column_int(stmt, 5);
                std::string fechaCreacion = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
                const char* fechaEjecucionPtr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
                std::string fechaEjecucion = fechaEjecucionPtr ? fechaEjecucionPtr : "";

                resumen += "ID: " + std::to_string(id) + ", Tipo: " + tipo +
                           ", Ticker: " + ticker + ", Cantidad: " + std::to_string(cantidad) +
                           ", Precio: " + std::to_string(precio) + ", Estado: " +
                           (estado == 1 ? "Ejecutada" : "Pendiente") + ", Fecha creación: " + fechaCreacion;
                if (estado == 1) resumen += ", Fecha ejecución: " + fechaEjecucion;
                resumen += "\n";
            }
            if (!hayOrdenes) {
                resumen += "No se encontraron órdenes para este usuario.\n";
            }
            sqlite3_finalize(stmt);
        } else {
            resumen += "Error al preparar la consulta.\n";
        }
        sqlite3_close(db);
    } else {
        resumen += "Error al acceder a la base de datos.\n";
    }

    send(comm_socket, resumen.c_str(), resumen.size(), 0);
}



void MenuOrden::comprar() {
    char buffer[256];
    std::string ticker;
    double precio, cantidad;

    send(comm_socket, "Ingrese el ticker: ", 20, 0);
    int recvLen = recv(comm_socket, buffer, sizeof(buffer) - 1, 0);
    if (recvLen <= 0) return;
    buffer[recvLen] = '\0';
    ticker = std::string(buffer);
    ticker.erase(ticker.find_last_not_of("\r\n") + 1);

    send(comm_socket, "Ingrese la cantidad: ", 22, 0);
    recvLen = recv(comm_socket, buffer, sizeof(buffer) - 1, 0);
    if (recvLen <= 0) return;
    buffer[recvLen] = '\0';
    cantidad = std::stod(buffer);

    send(comm_socket, "Ingrese el precio: ", 20, 0);
    recvLen = recv(comm_socket, buffer, sizeof(buffer) - 1, 0);
    if (recvLen <= 0) return;
    buffer[recvLen] = '\0';
    precio = std::stod(buffer);

    std::string fecha = obtenerFechaActual();
    sqlite3* db;
    sqlite3_stmt* stmt;

    if (sqlite3_open("JP.sqlite", &db) == SQLITE_OK) {
        std::string fechaEjecucion = "";
        int estado = 0; // 0 = Pendiente, 1 = Ejecutada

        // Buscar orden venta compatible para ejecución inmediata
        std::string query = "SELECT ID_Orden FROM Orden WHERE TipoOrden = 'Venta' AND Ticker = ? AND Precio <= ? AND Cantidad >= ? AND Estado = 0 LIMIT 1";
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_double(stmt, 2, precio);
            sqlite3_bind_double(stmt, 3, cantidad);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int idVenta = sqlite3_column_int(stmt, 0);
                estado = 1;
                fechaEjecucion = fecha;
                sqlite3_finalize(stmt);

                // Actualizar orden venta como ejecutada
                std::string update = "UPDATE Orden SET Estado = 1, Fecha_Ejecucion = ? WHERE ID_Orden = ?";
                if (sqlite3_prepare_v2(db, update.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_text(stmt, 1, fecha.c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_int(stmt, 2, idVenta);
                    sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                }
            } else {
                sqlite3_finalize(stmt);
            }
        }

        // Insertar orden compra
        std::string insert = "INSERT INTO Orden (Fecha_Creacion, Precio, Cantidad, Email, Ticker, TipoOrden, Estado, Fecha_Ejecucion) "
                             "VALUES (?, ?, ?, ?, ?, 'Compra', ?, ?)";
        if (sqlite3_prepare_v2(db, insert.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, fecha.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_double(stmt, 2, precio);
            sqlite3_bind_double(stmt, 3, cantidad);
            sqlite3_bind_text(stmt, 4, usuario.getEmail().c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 5, ticker.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 6, estado);
            if (estado == 1)
                sqlite3_bind_text(stmt, 7, fecha.c_str(), -1, SQLITE_STATIC);
            else
                sqlite3_bind_null(stmt, 7);

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                int id = static_cast<int>(sqlite3_last_insert_rowid(db));
                std::string msg = "Orden de compra creada con ID " + std::to_string(id) + "\n";
                send(comm_socket, msg.c_str(), msg.size(), 0);
            } else {
                send(comm_socket, "Error al insertar orden\n", 24, 0);
            }
            sqlite3_finalize(stmt);
        }

        sqlite3_close(db);
    }
}

void MenuOrden::vender() {
    char buffer[256];
    std::string ticker;
    double precio, cantidad;

    send(comm_socket, "Ingrese el ticker: ", 20, 0);
    int recvLen = recv(comm_socket, buffer, sizeof(buffer) - 1, 0);
    if (recvLen <= 0) return;
    buffer[recvLen] = '\0';
    ticker = std::string(buffer);
    ticker.erase(ticker.find_last_not_of("\r\n") + 1);

    send(comm_socket, "Ingrese la cantidad: ", 22, 0);
    recvLen = recv(comm_socket, buffer, sizeof(buffer) - 1, 0);
    if (recvLen <= 0) return;
    buffer[recvLen] = '\0';
    cantidad = std::stod(buffer);

    send(comm_socket, "Ingrese el precio: ", 20, 0);
    recvLen = recv(comm_socket, buffer, sizeof(buffer) - 1, 0);
    if (recvLen <= 0) return;
    buffer[recvLen] = '\0';
    precio = std::stod(buffer);

    std::string fecha = obtenerFechaActual();
    sqlite3* db;
    sqlite3_stmt* stmt;

    if (sqlite3_open("JP.sqlite", &db) == SQLITE_OK) {
        std::string fechaEjecucion = "";
        int estado = 0;

        // Buscar orden compra compatible para ejecución inmediata
        std::string query = "SELECT ID_Orden FROM Orden WHERE TipoOrden = 'Compra' AND Ticker = ? AND Precio >= ? AND Cantidad >= ? AND Estado = 0 LIMIT 1";
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_double(stmt, 2, precio);
            sqlite3_bind_double(stmt, 3, cantidad);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int idCompra = sqlite3_column_int(stmt, 0);
                estado = 1;
                fechaEjecucion = fecha;
                sqlite3_finalize(stmt);

                // Actualizar orden compra como ejecutada
                std::string update = "UPDATE Orden SET Estado = 1, Fecha_Ejecucion = ? WHERE ID_Orden = ?";
                if (sqlite3_prepare_v2(db, update.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_text(stmt, 1, fecha.c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_int(stmt, 2, idCompra);
                    sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                }
            } else {
                sqlite3_finalize(stmt);
            }
        }

        // Insertar orden venta
        std::string insert = "INSERT INTO Orden (Fecha_Creacion, Precio, Cantidad, Email, Ticker, TipoOrden, Estado, Fecha_Ejecucion) "
                             "VALUES (?, ?, ?, ?, ?, 'Venta', ?, ?)";
        if (sqlite3_prepare_v2(db, insert.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, fecha.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_double(stmt, 2, precio);
            sqlite3_bind_double(stmt, 3, cantidad);
            sqlite3_bind_text(stmt, 4, usuario.getEmail().c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 5, ticker.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 6, estado);
            if (estado == 1)
                sqlite3_bind_text(stmt, 7, fecha.c_str(), -1, SQLITE_STATIC);
            else
                sqlite3_bind_null(stmt, 7);

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                int id = static_cast<int>(sqlite3_last_insert_rowid(db));
                std::string msg = "Orden de venta creada con ID " + std::to_string(id) + "\n";
                send(comm_socket, msg.c_str(), msg.size(), 0);
            } else {
                send(comm_socket, "Error al insertar orden\n", 24, 0);
            }
            sqlite3_finalize(stmt);
        }

        sqlite3_close(db);
    }
}


bool MenuOrden::mostrarMenu() {
    char buffer[256];
    bool continuar = true;

    while (continuar) {
        std::string menu = "\n--- Menú de Órdenes ---\n"
                           "1. Mostrar órdenes anteriores\n"
                           "2. Comprar\n"
                           "3. Vender\n"
                           "4. Volver al menú principal\n"
                           "5. Cerrar conexión\n"
                           "Seleccione una opción: ";
        send(comm_socket, menu.c_str(), menu.size(), 0);

        int bytesRecibidos = recv(comm_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRecibidos <= 0) return false;
        buffer[bytesRecibidos] = '\0';

        int opcion = atoi(buffer);
        switch (opcion) {
            case 1:
                mostrarOrdenesAnteriores();
                break;
            case 2:
                comprar();
                break;
            case 3:
                vender();
                break;
            case 4:
                // volver al menú principal
                continuar = false;
                break;
            case 5:
                // cerrar la conexión
                return false;
            default:
                send(comm_socket, "Opción inválida\n", 16, 0);
                break;
        }
    }
    return true;
}

