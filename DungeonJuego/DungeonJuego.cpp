#include "UNIR-2D.h"
#include "JuegoDungeon.h"

// ============================================================
// MAIN
// ============================================================
// Punto de entrada del programa.
// Crea el motor y arranca el juego.
// ============================================================
int main() {
    unir2d::Motor motor;
    JuegoDungeon juego;
    motor.ejecuta(&juego);
    return 0;
}