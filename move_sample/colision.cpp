#ifndef LIBS
#define LIBS
#include <exception>
#include <cmath>
#include <iostream>
#include <string>
#endif

struct Field {
    float halfX = 219.0 / 2.0; // eje largo
    float halfZ = 158.0 / 2.0; // eje corto
    float safety = 9.0 + 1.0;  // radio robot + 1 cm extra
};

// Penales pegados a los bordes z = ±halfZ, “entran” 80 cm hacia adentro
struct Penalty {
    float half_d   = 25/2 + 9.0 + 1.0;  //ancho por cuadrante + radio robot + safe dist
    float w  = 80.0/2 + 9.0 + 1.0;      //largo por cuadrante + idem
};

// Limita una coordenada al rectángulo de juego con margen de seguridad (el radio del robot + 1cm)
inline void clampToField(float& x, float& z, const Field& f) {
    const float maxX = f.halfX - f.safety;
    const float maxZ = f.halfZ - f.safety;
    if (x >  maxX) x =  maxX;
    if (x < -maxX) x = -maxX;
    if (z >  maxZ) z =  maxZ;
    if (z < -maxZ) z = -maxZ;
}

void Penalty_out(float& x, float& z, Penalty& p){
    if (x < -p.half_d) x=  -p.half_d;
    if (x > p.half_d) x=  p.half_d;
    if (z< -p.w) z= -p.w;
    if (z> p.w) z= p.w;
}
