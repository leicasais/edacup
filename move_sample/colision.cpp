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
    float halfWidth = 25.0 / 2.0;  // 12.5 cm desde el centro en X
    float depth = 80.0;             // 80 cm de profundidad desde la portería
    float robotRadius = 9.0;        // radio del robot
    float safetyMargin = 1.0;       // margen adicional
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
// Portería propia: z < 0 (negativo)
// Portería rival: z > 0 (positivo)
void avoidPenaltyAreas(float& x, float& z, const Penalty& p, const Field& f) {
    const float safeRadius = p.robotRadius + p.safetyMargin;
    const float safeHalfWidth = p.halfWidth + safeRadius;
    
    // ÁREA PENAL PROPIA (z negativo, cerca de -halfZ)
    // Si estoy en el rango X del área penal Y muy cerca del borde negativo Z
    if (std::abs(x) < safeHalfWidth && z < -(f.halfZ - p.depth - safeRadius)) {
        // Estoy dentro o muy cerca del área penal propia
        // Opción 1: Sacar hacia afuera en Z (alejarse de la portería)
        z = -(f.halfZ - p.depth - safeRadius);
    }
    
    // ÁREA PENAL RIVAL (z positivo, cerca de +halfZ)
    // Si estoy en el rango X del área penal Y muy cerca del borde positivo Z
    if (std::abs(x) < safeHalfWidth && z > (f.halfZ - p.depth - safeRadius)) {
        // Estoy dentro o muy cerca del área penal rival
        // Sacar hacia afuera en Z (alejarse de la portería rival)
        z = (f.halfZ - p.depth - safeRadius);
    }
}

// Restringe al arquero DENTRO de su área penal
// El arquero debe moverse SOLO dentro del área penal propia (z negativo)
void keepInOwnPenaltyArea(float& x, float& z, const Penalty& p, const Field& f) {
    const float safeRadius = p.robotRadius + p.safetyMargin;
    
    // Límites del área penal propia
    const float maxX = p.halfWidth - safeRadius;
    const float minZ = -(f.halfZ - safeRadius);  // borde de la portería
    const float maxZ = -(f.halfZ - p.depth + safeRadius);  // límite frontal del área
    
    // Restringir X al ancho del área penal
    if (x > maxX) x = maxX;
    if (x < -maxX) x = -maxX;
    
    // Restringir Z dentro del área penal (entre la portería y el límite frontal)
    if (z < minZ) z = minZ;
    if (z > maxZ) z = maxZ;
}