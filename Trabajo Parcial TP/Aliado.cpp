#include "Aliado.h"
#include "Jugador.h"
#include "Enemigo.h"
#include "Mapa.h"
#include "Recurso.h" // Para EnColision
#include <cstdlib>
#include <cmath>

using namespace System;

Aliado::Aliado(int xInicial, int yInicial, ConsoleColor colPorDefecto, int vel, wchar_t caracterTipo)
    : ObjetoJuego(xInicial, yInicial, colPorDefecto),
    velocidad(Math::Max(1, vel)), contadorMovimiento(0) {
    switch (caracterTipo) {
    case L'F': this->color = ConsoleColor::Green; AgregarLineaRepresentacion(" /\\_"); AgregarLineaRepresentacion("(o.o)"); AgregarLineaRepresentacion(" U U "); break;
    case L'H': this->color = ConsoleColor::DarkCyan; AgregarLineaRepresentacion("[^_^]"); AgregarLineaRepresentacion("I H I"); AgregarLineaRepresentacion("-o-o-"); break;
    default: this->color = ConsoleColor::Blue; AgregarLineaRepresentacion(" (A) "); AgregarLineaRepresentacion(" /|\\ "); AgregarLineaRepresentacion(" / \\ "); break;
    }
    if (lineasRepresentacion == 0) {
        this->color = ConsoleColor::Blue;
        AgregarLineaRepresentacion("  A  ");
    }
}

void Aliado::Dibujar() { // Similar a Enemigo::Dibujar
    Console::ForegroundColor = color; int yActual = ObtenerYInicialDibujo(); int anchoMaximoArte = ObtenerAnchoVisual();
    int xDibujo = ObtenerXInicialDibujo();
    for (int i = 0; i < lineasRepresentacion; ++i) {
        String^ linea = representacion[i]; if (linea == nullptr) continue;
        try {
            if (yActual >= 0 && yActual < Console::BufferHeight && xDibujo >= 0 && xDibujo + linea->Length <= Console::BufferWidth) {
                Console::SetCursorPosition(xDibujo, yActual); Console::Write(linea);
            } yActual++;
        }
        catch (ArgumentOutOfRangeException^) { break; }
    } Console::ResetColor();
}

void Aliado::Limpiar() { // Similar a Enemigo::Limpiar
    int yActual = yPrev; int anchoMaximoArte = 0;
    for (int i = 0; i < lineasRepresentacion; ++i) if (representacion[i] != nullptr && representacion[i]->Length > anchoMaximoArte) anchoMaximoArte = representacion[i]->Length;
    int xLimpiar = xPrev - (anchoMaximoArte / 2);

    for (int i = 0; i < lineasRepresentacion; ++i) {
        String^ linea = representacion[i]; if (linea == nullptr) continue;
        try {
            if (yActual >= 0 && yActual < Console::BufferHeight && xLimpiar >= 0 && xLimpiar + linea->Length <= Console::BufferWidth) {
                Console::SetCursorPosition(xLimpiar, yActual); Console::Write(gcnew String(L' ', linea->Length));
            } yActual++;
        }
        catch (ArgumentOutOfRangeException^) { break; }
    }
}

void Aliado::Actualizar(Mapa^ mapa, Jugador^ jugador) {
    contadorMovimiento++;
    if (contadorMovimiento < velocidad) return;
    contadorMovimiento = 0;

    if (mapa == nullptr) return;

    int dx = (rand() % 3) - 1; // Movimiento aleatorio simple
    int dy = (rand() % 3) - 1;

    if (dx == 0 && dy == 0) return; // No se mueve esta vez

    int xSiguiente = x + dx;
    int ySiguiente = y + dy;

    int anchoAliadoVisual = ObtenerAnchoVisual();
    int altoAliadoVisual = ObtenerAltoVisual();
    int xEsquinaAliadoSiguiente = xSiguiente - (anchoAliadoVisual / 2);
    int yEsquinaAliadoSiguiente = ySiguiente;

    bool bloqueadoPorPared = !mapa->EsAreaValidaParaObjeto(xEsquinaAliadoSiguiente, yEsquinaAliadoSiguiente, anchoAliadoVisual, altoAliadoVisual);
    bool bloqueadoPorOtroObjeto = false;

    if (!bloqueadoPorPared) {
        array<ObjetoJuego^>^ objetosEnCelda = mapa->ObtenerObjetosEn(xSiguiente, ySiguiente);
        for each(ObjetoJuego ^ objetivo in objetosEnCelda) {
            if (objetivo == nullptr || objetivo == this) continue;
            if (objetivo->EnColision(this)) { // Si el Aliado considera que 'objetivo' es una barrera
                bloqueadoPorOtroObjeto = true;
                break;
            }
            // Aliados podrían tener interacciones especiales, ej. empujar enemigos leves o señalar recursos.
            // Por ahora, solo se mueven y son bloqueados.
        }
    }

    if (!bloqueadoPorPared && !bloqueadoPorOtroObjeto) {
        EstablecerPosicion(xSiguiente, ySiguiente);
    }
}

bool Aliado::EnColision(ObjetoJuego^ otro) {
    // El Aliado pregunta si 'otro' es una barrera para él.
    if (dynamic_cast<Jugador^>(otro) != nullptr) return false; // El jugador no bloquea al aliado (pueden ocupar misma celda de referencia)
    if (dynamic_cast<Enemigo^>(otro) != nullptr) return true;  // Los enemigos bloquean a los aliados.
    if (dynamic_cast<Recurso^>(otro) != nullptr) return false; // Los recursos no bloquean.
    if (dynamic_cast<Aliado^>(otro) != nullptr && otro != this) return true; // Otros aliados se bloquean entre sí.
    return true; // Otros tipos de ObjetoJuego bloquean.
}