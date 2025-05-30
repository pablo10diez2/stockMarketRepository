#ifndef ORDEN_H
#define ORDEN_H

#include <string>

class Orden {
private:
    int id;
    std::string fechaCreacion;
    std::string fechaEjecucion;
    int estado;
    double precio;
    int cantidad;
    std::string email;
    std::string ticker;
    std::string tipoOrden;

public:
    Orden(int id, const std::string& fechaCreacion, double precio, int cantidad,
          const std::string& email, const std::string& ticker, const std::string& tipoOrden);

    int getId() const;
    std::string getFechaCreacion() const;
    std::string getFechaEjecucion() const;
    int getEstado() const;
    double getPrecio() const;
    int getCantidad() const;
    std::string getEmail() const;
    std::string getTicker() const;
    std::string getTipoOrden() const;

    void setFechaEjecucion(const std::string& fecha);
    void setEstado(int estado);
};

#endif
