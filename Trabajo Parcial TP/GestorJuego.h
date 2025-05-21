#pragma once

#include "Mapa.h"
#include "Jugador.h"
#include "Recurso.h" 
#include <ctime> 

using namespace System;
using namespace System::IO;

ref class GestorJuego {
public: // Asegurarse de que el enum class sea public
    enum class EstadoJuego {
        INICIALIZANDO, MENU_PRINCIPAL, MENU_DIFICULTAD, MOSTRAR_INSTRUCCIONES,
        MOSTRAR_INTEGRANTES, MOSTRAR_CREDITOS, MUNDO1, MUNDO2, VISTA_MUNDO3,
        PANTALLA_CARGA,
        FIN_JUEGO_VICTORIA, FIN_JUEGO_DERROTA
    };
    enum class Dificultad { FACIL, NORMAL, DIFICIL };

private:
    static const int JUGADOR_VIDAS_INICIO = 3;
    static const int JUGADOR_ENERGIA_INICIO = 100;
    static const int META_PORCENTAJE_MUNDO3 = 80;
    static const double LIMITE_TIEMPO_JUEGO_SEGUNDOS = 300.0;
    static const int MAX_ALIADOS_SPAWN = 3;
    static const int OPCIONES_MENU_PRINCIPAL = 5;
    static const int OPCIONES_MENU_DIFICULTAD = 3;
    static const double DURACION_PANTALLA_CARGA_SEGUNDOS = 5.0;

    Mapa^ mapaMundo1;
    Mapa^ mapaMundo2;
    Mapa^ mapaMundo3;
    Mapa^ mapaActual;
    Jugador^ jugador;

    // Usar el tipo enum correctamente para los miembros
    EstadoJuego estadoActual;
    EstadoJuego estadoActualAnterior;
    EstadoJuego estadoMundoSiguiente;
    String^ nombreMundoCargando;
    int progresoCargaActual;
    clock_t tiempoInicioCarga;

    Dificultad dificultadJuego;
    bool juegoEnCurso;
    int progresoMundo3;
    time_t tiempoInicio;
    int aliadosSpawneados;

    int anchoConsola;
    int altoConsola;
    int opcionMenuPrincipalSel;
    int opcionDificultadSel;
    array<String^>^ elementosMenuPrincipal;
    array<String^>^ elementosMenuDificultad;
    bool necesitaRedibujarCompleto;

    String^ ultimoRecursoNombre;
    ConsoleColor ultimoRecursoColor;
    bool mostrarMensajeRecurso;

    // Declaraciones de métodos
    void InicializarConsolaYObtenerTamano();
    void InicializarDatosJuego();
    void InicializarMundosYJugador();
    void InicializarMenus();
    void BucleJuego();
    void ProcesarEntrada();
    void ProcesarEntradaMenuPrincipal(ConsoleKeyInfo infoTecla);
    void ProcesarEntradaMenuDificultad(ConsoleKeyInfo infoTecla);
    void ProcesarEntradaPantallaInfo(ConsoleKeyInfo infoTecla);
    void ProcesarEntradaMundo(ConsoleKeyInfo infoTecla);
    void ProcesarEntradaFinJuego(ConsoleKeyInfo infoTecla);
    void ActualizarJuego();
    void ActualizarMundo(Mapa^ mapaAActualizar);
    void RenderizarJuego();
    void RenderizarMenuPrincipal();
    void RenderizarMenuDificultad();
    void RenderizarPantallaInstrucciones();
    void RenderizarPantallaIntegrantes();
    void RenderizarPantallaCreditos();
    void RenderizarMundo();
    void RenderizarVistaMundo3();
    void RenderizarPantallaCarga();
    void RenderizarFinJuego();
    void IniciarJuegoNuevo(Dificultad dificultad); // Dificultad es GestorJuego::Dificultad
    void ManejarMovimientoJugador(int dx, int dy);
    void CambiarMundo(EstadoJuego estadoMundoDestinoDeseado); // EstadoJuego es GestorJuego::EstadoJuego
    void ConstruirMundo3(Recurso^ recurso);
    void VerificarCondicionesFin();
    void MostrarHUD();
    void MostrarInventarioYRecoleccionHUD();
    void LimpiarAreaConsola(int izq, int arr, int ancho, int altoParam);
    void MostrarFinJuegoInterno(bool jugadorGano);
    void DibujarMenu(array<String^>^ elementosMenu, int cantidadElementos, int indiceOpcionSel, String^ titulo);
    void IntentarInvocarAliado();

    bool GenerarPosicionValidaParaObjeto(Mapa^ mapa, ObjetoJuego^ objPrototipo, int% outX, int% outY, int intentosMax);
    bool GenerarPosicionValidaParaObjeto(Mapa^ mapa, ObjetoJuego^ objPrototipo, int% outX, int% outY);

public:
    GestorJuego();
    ~GestorJuego();
    void Ejecutar();
};