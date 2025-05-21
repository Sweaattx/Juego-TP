#include "Mapa.h"
#include "Jugador.h"
#include <stdexcept>

using namespace System;

Mapa::Mapa(int anchoMapa, int altoMapa, String^ nom, ConsoleColor colorBg)
    : ancho(Math::Max(10, anchoMapa)), alto(Math::Max(5, altoMapa)),
    nombreMapa(nom), colorFondo(colorBg),
    cantidadObjetos(0), capacidadObjetos(CAPACIDAD_INICIAL_OBJETOS) {
    objetosJuego = gcnew array<ObjetoJuego^>(this->capacidadObjetos);
}

Mapa::~Mapa() {}

void Mapa::RedimensionarArrayObjetos() {
    int nuevaCapacidad = (capacidadObjetos == 0) ? CAPACIDAD_INICIAL_OBJETOS : capacidadObjetos * 2;
    array<ObjetoJuego^>^ nuevoArray = gcnew array<ObjetoJuego^>(nuevaCapacidad);
    for (int i = 0; i < cantidadObjetos; ++i) nuevoArray[i] = objetosJuego[i];
    objetosJuego = nuevoArray; capacidadObjetos = nuevaCapacidad;
}

void Mapa::AgregarObjeto(ObjetoJuego^ obj) {
    if (obj == nullptr) return;
    if (cantidadObjetos == capacidadObjetos) RedimensionarArrayObjetos();
    objetosJuego[cantidadObjetos++] = obj;
}

int Mapa::BuscarIndiceObjeto(ObjetoJuego^ obj) {
    if (obj == nullptr) return -1;
    for (int i = 0; i < cantidadObjetos; ++i) if (objetosJuego[i] == obj) return i;
    return -1;
}

void Mapa::EliminarObjeto(ObjetoJuego^ obj) {
    int indice = BuscarIndiceObjeto(obj);
    if (indice != -1) {
        if (objetosJuego[indice] != nullptr) objetosJuego[indice]->Limpiar();
        for (int i = indice; i < cantidadObjetos - 1; ++i) objetosJuego[i] = objetosJuego[i + 1];
        cantidadObjetos--;
        if (cantidadObjetos >= 0 && cantidadObjetos < capacidadObjetos) objetosJuego[cantidadObjetos] = nullptr;
    }
}

array<ObjetoJuego^>^ Mapa::ObtenerObjetosEn(int x, int y) {
    int contadorEnLugar = 0;
    for (int i = 0; i < cantidadObjetos; ++i) if (objetosJuego[i] != nullptr && objetosJuego[i]->ObtenerX() == x && objetosJuego[i]->ObtenerY() == y) contadorEnLugar++;
    array<ObjetoJuego^>^ objetosEnLugar = gcnew array<ObjetoJuego^>(contadorEnLugar);
    int indiceActual = 0;
    for (int i = 0; i < cantidadObjetos; ++i) if (objetosJuego[i] != nullptr && objetosJuego[i]->ObtenerX() == x && objetosJuego[i]->ObtenerY() == y) {
        if (indiceActual < contadorEnLugar) objetosEnLugar[indiceActual++] = objetosJuego[i]; else break;
    } return objetosEnLugar;
}

void Mapa::DibujarMarco() {
    Console::ForegroundColor = ConsoleColor::White;
    int mapDrawWidth = Math::Min(this->ancho, Console::BufferWidth);
    int mapDrawHeight = Math::Min(this->alto, Console::BufferHeight);

    if (mapDrawWidth <= 0 || mapDrawHeight <= 0) return;

    String^ horizontalBorder = gcnew String(L'=', mapDrawWidth);

    try {
        // Línea superior
        if (0 < mapDrawHeight) { // Asegura que la fila 0 es visible
            Console::SetCursorPosition(0, 0); Console::Write(horizontalBorder);
        }
        // Línea inferior
        if (mapDrawHeight > 1 && (this->alto - 1) < mapDrawHeight) { // Asegura que la última fila del mapa es visible
            Console::SetCursorPosition(0, this->alto - 1); Console::Write(horizontalBorder);
        }

        // Líneas verticales
        for (int i = 1; i < this->alto - 1; ++i) {
            if (i < mapDrawHeight) { // Solo dibujar si la fila 'i' está dentro de la altura visible
                if (0 < mapDrawWidth) { Console::SetCursorPosition(0, i); Console::Write(L'|'); }
                if (this->ancho - 1 < mapDrawWidth && this->ancho - 1 >= 0) { Console::SetCursorPosition(this->ancho - 1, i); Console::Write(L'|'); }
            }
        }

        // Esquinas, asegurando que no se dibujen fuera de la pantalla visible
        if (0 < mapDrawWidth && 0 < mapDrawHeight) { Console::SetCursorPosition(0, 0); Console::Write(L'+'); }
        if ((this->ancho - 1) < mapDrawWidth && (this->ancho - 1) >= 0 && 0 < mapDrawHeight) { Console::SetCursorPosition(this->ancho - 1, 0); Console::Write(L'+'); }
        if (0 < mapDrawWidth && (this->alto - 1) < mapDrawHeight && (this->alto - 1) >= 0) { Console::SetCursorPosition(0, this->alto - 1); Console::Write(L'+'); }
        if ((this->ancho - 1) < mapDrawWidth && (this->ancho - 1) >= 0 && (this->alto - 1) < mapDrawHeight && (this->alto - 1) >= 0) { Console::SetCursorPosition(this->ancho - 1, this->alto - 1); Console::Write(L'+'); }
    }
    catch (ArgumentOutOfRangeException^) { /* Ignorar si se intenta dibujar fuera de la pantalla */ }
    Console::ResetColor();
}


void Mapa::Dibujar() {
    for (int i = 0; i < cantidadObjetos; ++i) if (objetosJuego[i] != nullptr) objetosJuego[i]->Limpiar();
    DibujarMarco();
    for (int i = 0; i < cantidadObjetos; ++i) if (objetosJuego[i] != nullptr) objetosJuego[i]->Dibujar();
}

void Mapa::ActualizarTodos(Jugador^ jugador) {
    array<ObjetoJuego^>^ objetosAActualizar = gcnew array<ObjetoJuego^>(cantidadObjetos);
    Array::Copy(objetosJuego, objetosAActualizar, cantidadObjetos);
    for (int i = 0; i < objetosAActualizar->Length; ++i) {
        ObjetoJuego^ obj = objetosAActualizar[i];
        if (obj != nullptr) {
            bool aunExisteEnMapa = false;
            for (int j = 0; j < cantidadObjetos; ++j) if (objetosJuego[j] == obj) { aunExisteEnMapa = true; break; }
            if (aunExisteEnMapa) obj->Actualizar(this, jugador);
        }
    }
}

int Mapa::ObtenerAncho() { return ancho; }
int Mapa::ObtenerAlto() { return alto; }
String^ Mapa::ObtenerNombre() { return nombreMapa; }
ConsoleColor Mapa::ObtenerColorFondo() { return colorFondo; }
bool Mapa::EsPosicionValida(int x, int y) { return x > 0 && x < ancho - 1 && y > 0 && y < alto - 1; }

bool Mapa::EsAreaValidaParaObjeto(int xEsquinaSupIzq, int yEsquinaSupIzq, int anchoVisual, int altoVisual) {
    if (xEsquinaSupIzq < 1) return false;
    if ((xEsquinaSupIzq + anchoVisual) > (this->ancho - 1)) return false;
    if (yEsquinaSupIzq < 1) return false;
    if ((yEsquinaSupIzq + altoVisual) > (this->alto - 1)) return false;
    return true;
}

array<ObjetoJuego^>^ Mapa::ObtenerTodosObjetos() {
    array<ObjetoJuego^>^ copiaObjetos = gcnew array<ObjetoJuego^>(cantidadObjetos);
    Array::Copy(objetosJuego, copiaObjetos, cantidadObjetos); return copiaObjetos;
}
int Mapa::ObtenerCantidadObjetos() { return cantidadObjetos; }