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
inline void clampToField(double& x, double& z, const Field& f) {
    const double maxX = f.halfX - f.safety;
    const double maxZ = f.halfZ - f.safety;
    if (x >  maxX) x =  maxX;
    if (x < -maxX) x = -maxX;
    if (z >  maxZ) z =  maxZ;
    if (z < -maxZ) z = -maxZ;
}

// Saca al bot si quedaría completamente dentro del área penal
// Asume penales frente a z=±halfZ y extendiéndose hacia el centro en z.
// Si tu simulador los pone sobre x, intercambiá (x,z) en los if.
inline void keepOutPenalty(double& x, double& z, const Field& f, const Penalty& p) {
    // Penal superior (cerca de z = +halfZ)
    double zInTop = f.halfZ - p.depth;
    bool insideTop =
        std::abs(x) <= p.halfW &&
        z >= zInTop && z <= f.halfZ;

    // Penal inferior (cerca de z = -halfZ)
    double zInBottom = -f.halfZ + p.depth;
    bool insideBottom =
        std::abs(x) <= p.halfW &&
        z <= zInBottom && z >= -f.halfZ;

    // Si está completamente adentro, lo proyectamos a la “frontera externa” del área
    if (insideTop) {
        // Lo dejamos justo afuera del área, con 1 cm de colchón
        z = zInTop - 1.0;
    } else if (insideBottom) {
        z = zInBottom + 1.0;
    }
}

// aplica límites y penal
inline void enforceBoundaries(double& x, double& z, const Field& f, const Penalty& p) {
    clampToField(x, z, f);
    keepOutPenalty(x, z, f, p);
    // Re-clamp por si la proyección al sacar del penal te empujó fuera por 1 cm
    clampToField(x, z, f);
}

