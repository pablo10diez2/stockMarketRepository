#ifndef ORDEN_H
#define ORDEN_H

#include <string>

class Orden {
private:
    int id;
    std::string fechaCreacion;
    std::string fechaEjecucion; // Vacía si no está ejecutada
    int estado; // 0 = pendiente, 1 = ejecutada
    double precio;
    double cantidad;
    std::string email;
    std::string ticker;
    std::string tipoOrden; // "Compra" o "Venta"

public:
    Orden(int id, const std::string& fechaCreacion, double precio, double cantidad,
          const std::string& email, const std::string& ticker, const std::string& tipoOrden);

    int getId() const;
    std::string getFechaCreacion() const;
    std::string getFechaEjecucion() const;
    int getEstado() const;
    double getPrecio() const;
    double getCantidad() const;
    std::string getEmail() const;
    std::string getTicker() const;
    std::string getTipoOrden() const;

    void setFechaEjecucion(const std::string& fecha);
    void setEstado(int estado);
};

#endif
