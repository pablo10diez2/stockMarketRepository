#include "Orden.h"

Orden::Orden(int id, const std::string& fechaCreacion, double precio, int cantidad,
             const std::string& email, const std::string& ticker, const std::string& tipoOrden)
    : id(id), fechaCreacion(fechaCreacion), precio(precio), cantidad(cantidad),
      email(email), ticker(ticker), tipoOrden(tipoOrden), estado(0), fechaEjecucion("") {}

int Orden::getId() const { return id; }
std::string Orden::getFechaCreacion() const { return fechaCreacion; }
std::string Orden::getFechaEjecucion() const { return fechaEjecucion; }
int Orden::getEstado() const { return estado; }
double Orden::getPrecio() const { return precio; }
int Orden::getCantidad() const { return cantidad; }
std::string Orden::getEmail() const { return email; }
std::string Orden::getTicker() const { return ticker; }
std::string Orden::getTipoOrden() const { return tipoOrden; }

void Orden::setFechaEjecucion(const std::string& fecha) { fechaEjecucion = fecha; }
void Orden::setEstado(int e) { estado = e; }
