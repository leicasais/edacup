#ifndef LIBS
#define LIBS
#include <exception>
#include <cmath>
#include <iostream>
#include <string>
#endif

struct Field {
    double halfX = 219.0 / 2.0; // eje largo
    double halfZ = 158.0 / 2.0; // eje corto
    double safety = 9.0 + 1.0;  // radio robot + 1 cm extra
};

// Penales pegados a los bordes z = ±halfZ, “entran” 80 cm hacia adentro
struct Penalty {
    double depth   = 80.0;      // cuánto entra desde la línea de gol
    double halfW   = 25.0 / 2.0; // ancho total 25 → mitad 12.5 (centro del arco)
    // Si tu simulador usa el ancho del arco (60 cm) para limitar, cambiá halfW=30.0
};

// Limita una coordenada al rectángulo de juego con margen de seguridad (el radio del robot + 1cm)
inline void clampToField(float& x, float& z, const Field& f) {
    const double maxX = f.halfX - f.safety;
    const double maxZ = f.halfZ - f.safety;
    if (x >  maxX) x =  maxX;
    if (x < -maxX) x = -maxX;
    if (z >  maxZ) z =  maxZ;
    if (z < -maxZ) z = -maxZ;
}
