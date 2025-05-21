#include "Enemigo.h"
#include "Jugador.h" // Necesario para dynamic_cast en EnColision y para MoverHaciaJugador
#include "Mapa.h"    // Necesario para la lógica de Actualizar y Mover
#include "Recurso.h" // Necesario para dynamic_cast en EnColision
#include "Aliado.h"  // Necesario para dynamic_cast en EnColision
#include <cstdlib>   // Para rand()
#include <cmath>     // Para Math::Max, Math::Abs (si se usa)

using namespace System;

Enemigo::Enemigo(int xInicial, int yInicial, ConsoleColor colPorDefectoIgnorado, int dmg, int vel, wchar_t caracterTipo)
    : ObjetoJuego(xInicial, yInicial, ConsoleColor::Red), // Color base por defecto para todos los enemigos
    dano(dmg), velocidad(Math::Max(1, vel)), contadorMovimiento(0) {

    // Se asigna un color rojo base, pero se puede especializar si es necesario,
    // o simplemente dejar que todos hereden el ConsoleColor::Red de la inicialización base.
    // Para hacerlos a TODOS ROJOS, simplemente podemos confiar en el color base
    // y solo ajustar el ASCII. Si un tipo específico debe ser DarkRed, se ajusta.

    switch (caracterTipo) {
    case L'X':
        this->color = ConsoleColor::Red; // O DarkRed si prefieres
        AgregarLineaRepresentacion("\\ V /");
        AgregarLineaRepresentacion(" >X< ");
        AgregarLineaRepresentacion("/ A \\");
        break;
    case L'O':
        this->color = ConsoleColor::Red; // O DarkRed
        AgregarLineaRepresentacion(".---.");
        AgregarLineaRepresentacion("(o o)");
        AgregarLineaRepresentacion("`---´");
        break;
    case L'#':
        this->color = ConsoleColor::Red; // O DarkRed
        AgregarLineaRepresentacion("+---+");
        AgregarLineaRepresentacion("|#-#|");
        AgregarLineaRepresentacion("+---+");
        break;
    case L'*':
        this->color = ConsoleColor::DarkRed; // Este ya era rojo oscuro, lo mantenemos o cambiamos a Red
        // Si quieres TODO rojo brillante: this->color = ConsoleColor::Red;
        AgregarLineaRepresentacion(" /*\\ ");
        AgregarLineaRepresentacion("*-*-*");
        AgregarLineaRepresentacion(" \\*/ ");
        break;
    default:
        this->color = ConsoleColor::Red; // Enemigo desconocido también en rojo
        AgregarLineaRepresentacion(" ??? ");
        AgregarLineaRepresentacion("?????");
        AgregarLineaRepresentacion(" ??? ");
        break;
    }

    // Fallback si no se añadió ninguna representación (no debería ocurrir con el switch default)
    if (lineasRepresentacion == 0) {
        this->color = ConsoleColor::Red; // Asegurar color rojo
        AgregarLineaRepresentacion("  E  ");
    }
}

void Enemigo::Dibujar() {
    Console::ForegroundColor = this->color; // Usar el color del enemigo
    int yActual = ObtenerYInicialDibujo();
    int anchoMaximoArte = ObtenerAnchoVisual();
    int xDibujo = ObtenerXInicialDibujo();

    for (int i = 0; i < lineasRepresentacion; ++i) {
        String^ linea = representacion[i];
        if (linea == nullptr) continue;
        try {
            if (yActual >= 0 && yActual < Console::BufferHeight &&
                xDibujo >= 0 && xDibujo + linea->Length <= Console::BufferWidth) {
                Console::SetCursorPosition(xDibujo, yActual);
                Console::Write(linea);
            }
            yActual++;
        }
        catch (ArgumentOutOfRangeException^) {
            // Intentar dibujar fuera de los límites del buffer de la consola
            break;
        }
    }
    Console::ResetColor(); // Es buena práctica resetear el color después de dibujar
}

void Enemigo::Limpiar() {
    int yActual = yPrev; // Usar yPrev para la limpieza
    int anchoMaximoArte = 0;
    // Es importante recalcular el ancho basado en la representación actual,
    // aunque en este caso no cambia dinámicamente para el enemigo.
    for (int i = 0; i < lineasRepresentacion; ++i) {
        if (representacion[i] != nullptr && representacion[i]->Length > anchoMaximoArte) {
            anchoMaximoArte = representacion[i]->Length;
        }
    }
    int xLimpiar = xPrev - (anchoMaximoArte / 2); // Usa xPrev

    String^ espacios = gcnew String(L' ', anchoMaximoArte > 0 ? anchoMaximoArte : 1); // Evitar string de 0 espacios

    for (int i = 0; i < lineasRepresentacion; ++i) {
        // String^ linea = representacion[i]; // No necesitamos el contenido, solo el ancho
        // if (linea == nullptr) continue; // No es necesario si solo usamos espacios del ancho máximo

        try {
            if (yActual >= 0 && yActual < Console::BufferHeight &&
                xLimpiar >= 0 && xLimpiar + anchoMaximoArte <= Console::BufferWidth && anchoMaximoArte > 0) {
                Console::SetCursorPosition(xLimpiar, yActual);
                // Limpiar usando el ancho de la línea más larga del ASCII
                Console::Write(gcnew String(L' ', representacion[i] != nullptr ? representacion[i]->Length : 0));
            }
            yActual++;
        }
        catch (ArgumentOutOfRangeException^) {
            break;
        }
    }
}

void Enemigo::Actualizar(Mapa^ mapa, Jugador^ jugador) {
    contadorMovimiento++;
    if (contadorMovimiento < velocidad) return;
    contadorMovimiento = 0;

    if (jugador == nullptr || mapa == nullptr) return;

    int dx = 0, dy = 0;
    // 70% de probabilidad de moverse hacia el jugador, 30% aleatorio
    if (rand() % 100 < 70) {
        MoverHaciaJugador(mapa, jugador, dx, dy);
    }
    else {
        MoverAleatorio(mapa, dx, dy);
    }

    if (dx != 0 || dy != 0) {
        int xSiguiente = x + dx;
        int ySiguiente = y + dy;

        int anchoEnemigoVisual = ObtenerAnchoVisual();
        int altoEnemigoVisual = ObtenerAltoVisual();
        int xEsquinaEnemigoSiguiente = xSiguiente - (anchoEnemigoVisual / 2);
        int yEsquinaEnemigoSiguiente = ySiguiente;

        bool bloqueadoPorPared = !mapa->EsAreaValidaParaObjeto(xEsquinaEnemigoSiguiente, yEsquinaEnemigoSiguiente, anchoEnemigoVisual, altoEnemigoVisual);
        bool bloqueadoPorOtroObjeto = false;

        if (!bloqueadoPorPared) {
            // Verificar colisión con otros objetos en la celda de destino (del punto de referencia)
            array<ObjetoJuego^>^ objetosEnCelda = mapa->ObtenerObjetosEn(xSiguiente, ySiguiente);
            for each(ObjetoJuego ^ objetivo in objetosEnCelda) {
                if (objetivo == nullptr || objetivo == this) continue;

                // Si el enemigo choca con el jugador
                Jugador^ j = dynamic_cast<Jugador^>(objetivo);
                if (j != nullptr) {
                    j->RecibirDano(this->dano); // Enemigo hace daño al jugador
                    bloqueadoPorOtroObjeto = true; // El jugador bloquea el paso del enemigo
                    break;
                }

                // Si el enemigo choca con otro objeto que lo bloquea (definido en EnColision de Enemigo)
                if (this->EnColision(objetivo)) {
                    bloqueadoPorOtroObjeto = true;
                    break;
                }
            }
        }

        if (!bloqueadoPorPared && !bloqueadoPorOtroObjeto) {
            EstablecerPosicion(xSiguiente, ySiguiente);
        }
    }
}

void Enemigo::MoverHaciaJugador(Mapa^ mapa, Jugador^ jugador, int% salidaDx, int% salidaDy) {
    int xObjetivo = jugador->ObtenerX();
    int yObjetivo = jugador->ObtenerY();
    salidaDx = 0;
    salidaDy = 0;

    if (x < xObjetivo) salidaDx = 1;
    else if (x > xObjetivo) salidaDx = -1;

    if (y < yObjetivo) salidaDy = 1;
    else if (y > yObjetivo) salidaDy = -1;

    // Mover en un eje a la vez para un movimiento más predecible o menos errático
    if (salidaDx != 0 && salidaDy != 0) {
        if (rand() % 2 == 0) salidaDx = 0;
        else salidaDy = 0;
    }
}

void Enemigo::MoverAleatorio(Mapa^ mapa, int% salidaDx, int% salidaDy) {
    salidaDx = (rand() % 3) - 1; // -1, 0, or 1
    salidaDy = (rand() % 3) - 1; // -1, 0, or 1

    // Pequeña chance de forzar un movimiento si ambos son 0, para evitar que se quede quieto mucho tiempo
    if (salidaDx == 0 && salidaDy == 0 && (rand() % 4 == 0)) {
        int dir = rand() % 4;
        if (dir == 0) salidaDx = 1;
        else if (dir == 1) salidaDx = -1;
        else if (dir == 2) salidaDy = 1;
        else salidaDy = -1;
    }
}

bool Enemigo::EnColision(ObjetoJuego^ otro) {
    // Esta función define si 'otro' es una barrera para ESTE enemigo.
    // El jugador NO es una barrera per se (el enemigo puede entrar en su celda para atacar),
    // la interacción de daño se maneja en Actualizar.
    if (dynamic_cast<Jugador^>(otro) != nullptr) return false; // El enemigo puede superponerse para atacar.
    if (dynamic_cast<Recurso^>(otro) != nullptr) return false; // Los recursos no bloquean.
    if (dynamic_cast<Enemigo^>(otro) != nullptr && otro != this) return true; // Otros enemigos sí bloquean.
    if (dynamic_cast<Aliado^>(otro) != nullptr) return true; // Los aliados bloquean a los enemigos.

    // Por defecto, si no es ninguno de los anteriores y no es nulo, podría ser un obstáculo genérico
    // (si tuvieras una clase Obstaculo, la chequearías aquí).
    // Si no hay otros tipos de objetos que bloqueen, podrías retornar false aquí.
    // Pero para ser seguro, si es un ObjetoJuego desconocido, asumamos que bloquea.
    if (otro != nullptr) return true;

    return false; // Si 'otro' es nullptr
}

int Enemigo::ObtenerDano() { return dano; }