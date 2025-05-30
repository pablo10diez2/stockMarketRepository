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

double calcularDineroRetenido(const std::string& email) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    double retenido = 0.0;

    if (sqlite3_open("JP.sqlite", &db) == SQLITE_OK) {
        const char* sql = "SELECT SUM(Precio * Cantidad) FROM Orden WHERE Email = ? AND TipoOrden = 'Compra' AND Estado = 0";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
                retenido = sqlite3_column_double(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }

    return retenido;
}

void MenuOrden::cancelarOrdenPendiente() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        send(comm_socket, "Error al conectar con la base de datos.\n", 41, 0);
        return;
    }

    std::string consulta = "SELECT ID_Orden, TipoOrden, Ticker, Cantidad, Precio, Fecha_Creacion "
                           "FROM Orden WHERE Email = ? AND Estado = 0";
    rc = sqlite3_prepare_v2(db, consulta.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        send(comm_socket, "Error al preparar la consulta.\n", 32, 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, usuario.getEmail().c_str(), -1, SQLITE_STATIC);
    std::string resultado = "\n--- Órdenes Pendientes ---\n";
    std::vector<int> idsDisponibles;
    std::vector<std::tuple<int, std::string, double, int>> ordenesDetalles;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string tipo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string ticker = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int cantidad = sqlite3_column_int(stmt, 3);
        double precio = sqlite3_column_double(stmt, 4);
        std::string fecha = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        resultado += "ID: " + std::to_string(id) + ", Tipo: " + tipo + ", Ticker: " + ticker +
                     ", Cantidad: " + std::to_string(cantidad) + ", Precio: " + std::to_string(precio) +
                     ", Fecha: " + fecha + "\n";

        idsDisponibles.push_back(id);
        ordenesDetalles.emplace_back(id, tipo, precio, cantidad);
    }

    sqlite3_finalize(stmt);

    if (idsDisponibles.empty()) {
        resultado += "No hay órdenes pendientes.\n";
        send(comm_socket, resultado.c_str(), resultado.size(), 0);
        sqlite3_close(db);
        return;
    }

    send(comm_socket, resultado.c_str(), resultado.size(), 0);
    send(comm_socket, "\nIngrese el ID de la orden a cancelar: ", 39, 0);

    char buffer[256];
    int bytes = recv(comm_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        sqlite3_close(db);
        return;
    }
    buffer[bytes] = '\0';
    int idIngresado = std::stoi(buffer);

    if (std::find(idsDisponibles.begin(), idsDisponibles.end(), idIngresado) == idsDisponibles.end()) {
        send(comm_socket, "ID no válido o no corresponde a una orden pendiente.\n", 54, 0);
        sqlite3_close(db);
        return;
    }

    for (auto& [id, tipo, precio, cantidad] : ordenesDetalles) {
        if (id == idIngresado) {
            if (tipo == "Compra") {
                double total = precio * cantidad;
                restarDineroAlComprador(usuario.getEmail(), -total);  // esto suma de nuevo
            }
            break;
        }
    }

    std::string deleteSQL = "DELETE FROM Orden WHERE ID_Orden = ?";
    rc = sqlite3_prepare_v2(db, deleteSQL.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, idIngresado);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            send(comm_socket, "Orden cancelada exitosamente.\n", 31, 0);
        } else {
            send(comm_socket, "Error al cancelar la orden.\n", 29, 0);
        }
        sqlite3_finalize(stmt);
    } else {
        send(comm_socket, "Error al preparar eliminación.\n", 31, 0);
    }

    sqlite3_close(db);
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
               int cantidad = sqlite3_column_int(stmt, 3);
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
               resumen += "No se encontraron ordenes para este usuario.\n";
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
    double precio;
    int cantidad;

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
    cantidad = std::stoi(buffer);

    send(comm_socket, "Ingrese el precio: ", 20, 0);
    recvLen = recv(comm_socket, buffer, sizeof(buffer) - 1, 0);
    if (recvLen <= 0) return;
    buffer[recvLen] = '\0';
    precio = std::stod(buffer);

    std::string fecha = obtenerFechaActual();
    sqlite3* db;
    sqlite3_stmt* stmt;

    if (sqlite3_open("JP.sqlite", &db) == SQLITE_OK) {
        double total = cantidad * precio;
        double saldoTotal = 0.0;

        const char* sqlSaldo = "SELECT Dinero FROM Usuario WHERE Email = ?";
        if (sqlite3_prepare_v2(db, sqlSaldo, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, usuario.getEmail().c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                saldoTotal = sqlite3_column_double(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }

        double retenido = calcularDineroRetenido(usuario.getEmail());
        if ((saldoTotal - retenido) < total) {
            send(comm_socket, "Fondos insuficientes. Saldo libre no cubre la orden.\n", 53, 0);
            sqlite3_close(db);
            return;
        }

        std::string fechaEjecucion = "";
        int estado = 0;

        std::string query = "SELECT ID_Orden FROM Orden WHERE TipoOrden = 'Venta' AND Ticker = ? AND Precio <= ? AND Cantidad = ? AND Estado = 0 ORDER BY Fecha_Creacion ASC LIMIT 1";
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_double(stmt, 2, precio);
            sqlite3_bind_int(stmt, 3, cantidad);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int idVenta = sqlite3_column_int(stmt, 0);
                estado = 1;
                fechaEjecucion = fecha;
                sqlite3_finalize(stmt);

                std::string update = "UPDATE Orden SET Estado = 1, Fecha_Ejecucion = ? WHERE ID_Orden = ?";
                if (sqlite3_prepare_v2(db, update.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_text(stmt, 1, fecha.c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_int(stmt, 2, idVenta);
                    sqlite3_step(stmt);
                    sqlite3_finalize(stmt);

                    // 💸 Transferencia de dinero
                    restarDineroAlComprador(usuario.getEmail(), total);
                    std::string emailVendedor = obtenerEmailPorIdOrden(idVenta);
                    sumarDineroAlVendedor(emailVendedor, total);
                }
            } else {
                sqlite3_finalize(stmt);
            }
        }

        std::string insert = "INSERT INTO Orden (Fecha_Creacion, Precio, Cantidad, Email, Ticker, TipoOrden, Estado, Fecha_Ejecucion) VALUES (?, ?, ?, ?, ?, 'Compra', ?, ?)";
        if (sqlite3_prepare_v2(db, insert.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, fecha.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_double(stmt, 2, precio);
            sqlite3_bind_int(stmt, 3, cantidad);
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

                if (estado == 0) {
                    restarDineroAlComprador(usuario.getEmail(), total);
                }
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
    double precio;
    int cantidad;

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
    cantidad = std::stoi(buffer);

    send(comm_socket, "Ingrese el precio: ", 20, 0);
    recvLen = recv(comm_socket, buffer, sizeof(buffer) - 1, 0);
    if (recvLen <= 0) return;
    buffer[recvLen] = '\0';
    precio = std::stod(buffer);

    sqlite3* db;
    sqlite3_stmt* stmt;
    if (sqlite3_open("JP.sqlite", &db) != SQLITE_OK) return;


    const char* sqlCheck = R"(
        SELECT 
            SUM(CASE 
                WHEN Estado = 1 AND TipoOrden = 'Compra' THEN Cantidad
                WHEN Estado = 1 AND TipoOrden = 'Venta' THEN -Cantidad
                WHEN Estado = 0 AND TipoOrden = 'Venta' THEN -Cantidad
                ELSE 0 
            END) AS Disponible
        FROM Orden
        WHERE Email = ? AND Ticker = ?
    )";

    if (sqlite3_prepare_v2(db, sqlCheck, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, usuario.getEmail().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, ticker.c_str(), -1, SQLITE_STATIC);

        int disponible = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            disponible = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);

        if (disponible < cantidad) {
            std::string msg = "No tienes suficientes acciones disponibles de este ticker para vender.\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            sqlite3_close(db);
            return;
        }
    } else {
        send(comm_socket, "Error al verificar disponibilidad de acciones.\n", 49, 0);
        sqlite3_close(db);
        return;
    }

    std::string fecha = obtenerFechaActual();
    int estado = 0;
    std::string fechaEjecucion = "";

    std::string query = "SELECT ID_Orden FROM Orden WHERE TipoOrden = 'Compra' AND Ticker = ? AND Precio >= ? AND Cantidad = ? AND Estado = 0 ORDER BY Fecha_Creacion ASC LIMIT 1";
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, ticker.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, precio);
        sqlite3_bind_int(stmt, 3, cantidad);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int idCompra = sqlite3_column_int(stmt, 0);

            std::string updateCompra = "UPDATE Orden SET Estado = 1, Fecha_Ejecucion = ? WHERE ID_Orden = ?";
            sqlite3_stmt* updateStmt;
            if (sqlite3_prepare_v2(db, updateCompra.c_str(), -1, &updateStmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(updateStmt, 1, fecha.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(updateStmt, 2, idCompra);
                sqlite3_step(updateStmt);
                sqlite3_finalize(updateStmt);
            }

            estado = 1;
            fechaEjecucion = fecha;
        }
        sqlite3_finalize(stmt);
    }

    std::string insert = "INSERT INTO Orden (Fecha_Creacion, Precio, Cantidad, Email, Ticker, TipoOrden, Estado, Fecha_Ejecucion) VALUES (?, ?, ?, ?, ?, 'Venta', ?, ?)";
    if (sqlite3_prepare_v2(db, insert.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, fecha.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, precio);
        sqlite3_bind_int(stmt, 3, cantidad);
        sqlite3_bind_text(stmt, 4, usuario.getEmail().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, ticker.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 6, estado);

        if (estado == 1)
            sqlite3_bind_text(stmt, 7, fecha.c_str(), -1, SQLITE_STATIC);
        else
            sqlite3_bind_null(stmt, 7);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            int id = static_cast<int>(sqlite3_last_insert_rowid(db));
            std::string msg = "Orden de venta " + std::string((estado == 1) ? "ejecutada" : "pendiente") +
                              " con ID " + std::to_string(id) + "\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
        } else {
            send(comm_socket, "Error al insertar orden de venta.\n", 34, 0);
        }

        sqlite3_finalize(stmt);
    }

    sqlite3_close(db);
}

bool MenuOrden::mostrarMenu() {
   char buffer[256];
   bool continuar = true;

   while (continuar) {
       std::string menu = "\n--- Menú de Órdenes ---\n"
                           "1. Mostrar órdenes anteriores\n"
                           "2. Comprar\n"
                           "3. Vender\n"
       				   "4. Cancelar orden\n"
                           "5. Volver al menú principal\n"
                           "6. Cerrar conexión\n"
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
               cancelarOrdenPendiente();
               break;
           case 5:
               return false;
           case 6:
               send(comm_socket, "Cerrando sesión. Hasta luego.\n", 31, 0);
               continuar = false;
               break;
           default:
               send(comm_socket, "Opción inválida\n", 16, 0);
               break;
       }
   }
   return true;
}
