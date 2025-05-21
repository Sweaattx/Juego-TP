#pragma once

using namespace System;

ref class Mapa;
ref class Jugador;

ref class ObjetoJuego abstract {
protected:
    int x, y;
    int xPrev, yPrev;
    ConsoleColor color;
    static const int MAX_LINEAS_REPRESENTACION = 5;
    array<String^>^ representacion;
    int lineasRepresentacion;

public:
    ObjetoJuego(int xInicial, int yInicial, ConsoleColor col)
        : x(xInicial), y(yInicial), xPrev(xInicial), yPrev(yInicial), color(col) {
        representacion = gcnew array<String^>(MAX_LINEAS_REPRESENTACION);
        lineasRepresentacion = 0;
    }

    virtual ~ObjetoJuego() {}

    virtual void Dibujar() abstract;
    virtual void Limpiar() abstract;
    virtual void Actualizar(Mapa^ mapa, Jugador^ jugador) abstract;
    virtual bool EnColision(ObjetoJuego^ otro) abstract;

    virtual bool HaySuperposicionVisual(ObjetoJuego^ otro);

    int ObtenerX() { return x; }
    int ObtenerY() { return y; }
    int ObtenerXPrev() { return xPrev; }
    int ObtenerYPrev() { return yPrev; }
    ConsoleColor ObtenerColor() { return color; }
    array<String^>^ ObtenerRepresentacion() { return representacion; }
    int ObtenerLineasRepresentacion() { return lineasRepresentacion; }

    virtual void EstablecerPosicion(int nuevoX, int nuevoY);

    virtual int ObtenerAnchoVisual();
    virtual int ObtenerAltoVisual();
    virtual int ObtenerXInicialDibujo();
    virtual int ObtenerYInicialDibujo();

protected:
    void AgregarLineaRepresentacion(String^ linea);
};
