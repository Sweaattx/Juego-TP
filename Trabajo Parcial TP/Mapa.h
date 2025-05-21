#pragma once

#include "ObjetoJuego.h"

using namespace System;

ref class Jugador;

ref class Mapa {
private:
    int ancho;
    int alto;
    String^ nombreMapa;
    ConsoleColor colorFondo;
    array<ObjetoJuego^>^ objetosJuego;
    int cantidadObjetos;
    int capacidadObjetos;
    static const int CAPACIDAD_INICIAL_OBJETOS = 50;

    void DibujarMarco();
    void RedimensionarArrayObjetos();
    int BuscarIndiceObjeto(ObjetoJuego^ obj);

public:
    Mapa(int anchoMapa, int altoMapa, String^ nom, ConsoleColor colorBg);
    ~Mapa();
    void AgregarObjeto(ObjetoJuego^ obj);
    void EliminarObjeto(ObjetoJuego^ obj);
    array<ObjetoJuego^>^ ObtenerObjetosEn(int x, int y);
    void Dibujar();
    void ActualizarTodos(Jugador^ jugador);
    int ObtenerAncho();
    int ObtenerAlto();
    String^ ObtenerNombre();
    ConsoleColor ObtenerColorFondo();
    bool EsPosicionValida(int x, int y); // Valida un punto dentro de los bordes jugables
    bool EsAreaValidaParaObjeto(int xEsquinaSupIzq, int yEsquinaSupIzq, int anchoVisual, int altoVisual); // Valida un área completa
    array<ObjetoJuego^>^ ObtenerTodosObjetos();
    int ObtenerCantidadObjetos();
};