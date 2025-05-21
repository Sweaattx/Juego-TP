#pragma once

#include "ObjetoJuego.h"
#include <ctime>

using namespace System;

ref class Recurso;
ref class Mapa;

ref class Jugador : public ObjetoJuego {
private:
    int vidas;
    int energia;
    array<Recurso^>^ inventario;
    int cantidadInventario;
    int capacidadInventario;
    static const int CAPACIDAD_INICIAL_INVENTARIO = 10;
    clock_t tiempoUltimoDano;
    static const double COOLDOWN_DANO_SEGUNDOS = 0.75;
    void RedimensionarInventario();
public:
    Jugador(int xInicial, int yInicial, int vidasIniciales, int energiaInicial);
    virtual ~Jugador() override;
    virtual void Dibujar() override;
    virtual void Limpiar() override;
    virtual void Actualizar(Mapa^ mapa, Jugador^ jugador) override;
    virtual bool EnColision(ObjetoJuego^ otro) override;
    void Mover(int dx, int dy);
    void RecolectarRecurso(Recurso^ recurso);
    void RecibirDano(int cantidad);
    int ObtenerVidas();
    int ObtenerEnergia();
    array<Recurso^>^ ObtenerInventario();
    int ObtenerCantidadInventario();
};