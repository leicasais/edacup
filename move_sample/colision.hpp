#ifndef COLISION_HPP
#define COLISION_HPP
#ifndef LIBS
#define LIBS
#include <exception>
#include <cmath>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#endif

struct Field {
    float halfX = 2.19 / 2.0; // eje largo (109.5 cm)
    float halfZ = 1.58 / 2.0; // eje corto (79 cm)
    float safety = 0.09 + 0.01;  // radio robot (9cm) + 1 cm extra
};

// Áreas de penalti: 25 cm de ancho (±12.5 desde centro) y 80 cm de largo
struct Penalty {
    float Width = 0.25;  // largo en x de cda porteria
    float depth = 0.8;             // 80 cm de profundidad desde la portería
    float robotRadius = 0.09;     
    float safetyMargin = 0.01;
};

void keepInOwnPenaltyArea(float& x, float& z, const Penalty& p, const Field& f);
void clampToField(float& x, float& z, const Field& f);
void avoidPenaltyAreas(float& x, float& z, const Penalty& p, const Field& f);
#endif // COLISION_HPP