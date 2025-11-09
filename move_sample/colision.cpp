#ifndef LIBS
#define LIBS
#include <exception>
#include <cmath>
#include <iostream>
#include <string>
#endif

struct Field {
    float halfX = 219.0 / 2.0; // eje largo (109.5 cm)
    float halfZ = 158.0 / 2.0; // eje corto (79 cm)
    float safety = 9.0 + 1.0;  // radio robot (9cm) + 1 cm extra
};

// Áreas de penalti: 25 cm de ancho (±12.5 desde centro) y 80 cm de largo
struct Penalty {
    float Width = 25.0;  // largo en x de cda porteria
    float depth = 80.0;             // 80 cm de profundidad desde la portería
    float robotRadius = 9.0;     
    float safetyMargin = 1.0;
};

// Limita una coordenada al rectángulo de juego con margen de seguridad
inline void clampToField(float& x, float& z, const Field& f) {
    const float maxX = f.halfX - f.safety;
    const float maxZ = f.halfZ - f.safety;
    if (x >  maxX) x =  maxX;
    if (x < -maxX) x = -maxX;
    if (z >  maxZ) z =  maxZ;
    if (z < -maxZ) z = -maxZ;
}

// Evita que el robot entre en CUALQUIERA de las dos áreas penales

void avoidPenaltyAreas(float& x, float& z, const Penalty& p, const Field& f) {
    const float safeRadius = p.robotRadius + p.safetyMargin;
    const float safeWidth = p.Width + safeRadius;
    
    //mitad de abajo de arco rival y propio
    if (!(std::abs(x) > f.halfX - safeWidth && abs(z) > p.depth /2 + safeRadius)) {
        if(z < p.depth /2)
            z = p.depth/2 + safeRadius;
        else
            z = -(p.depth/2 + safeRadius);
    }
    
}

// Restringe al arquero DENTRO de su área penal

void keepInOwnPenaltyArea(float& x, float& z, const Penalty& p, const Field& f) {
    const float safeRadius = p.robotRadius + p.safetyMargin;
    
    // Límites del área penal propia
    const float maxX = -(f.halfX-p.Width-safeRadius);
    const float minX = -f.halfX;
    const float minZ = -p.depth/2 +safeRadius;  // borde de la portería
    const float maxZ = p.depth/2 -safeRadius;  // límite frontal del área
    
    // Restringir X al ancho del área penal
    if (x > maxX) x = maxX;
    if (x < -maxX) x = -maxX;
    
    // Restringir Z dentro del área penal (entre la portería y el límite frontal)
    if (z < minZ) z = minZ;
    if (z > maxZ) z = maxZ;
}