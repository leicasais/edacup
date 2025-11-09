#include "colision.hpp"

// Limita una coordenada al rectángulo de juego con margen de seguridad
void clampToField(float& x, float& z, const Field& f) {
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
    const float safeDeph = p.depth +safeRadius;

    //mitad de abajo de arco rival y propio
    if (std::abs(x) > (f.halfX - safeWidth) && abs(z) < (p.depth /2 - safeDeph)) {
        if(z < p.depth /2 - safeDeph && z > 0)
            z = p.depth/2 + safeDeph;
        else if(z > -(p.depth /2 - safeDeph) && z < 0)
            z = -(p.depth/2 + safeDeph);

        if (x > (f.halfX - safeWidth))   
            x = f.halfX - safeWidth;
        else if (x < -(f.halfX - safeWidth))
            x = -(f.halfX - safeWidth);   
    }

}
