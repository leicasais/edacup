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
void avoidPenaltyAreas(float& x, float& z, const Penalty& p, const Field& f)
{
    const float safe = p.robotRadius + p.safetyMargin;

    const float halfD = p.depth * 0.5f;

    // derecha (arco rival)
    float leftLimitR  =  f.halfX - p.width - safe;   // lado izquierdo del area rival
    float rightLimitR =  f.halfX + safe;             // pared absoluta, no deberías llegar igual

    // izquierda (arco propio)
    float leftLimitL  = -f.halfX - safe;             // pared absoluta
    float rightLimitL = -f.halfX + p.width + safe;   // lado derecho del area propia

    // área rival
    if (x > leftLimitR && x < rightLimitR && fabs(z) < halfD + safe)
    {
        x = leftLimitR;
    }

    // área propia
    if (x < rightLimitL && x > leftLimitL && fabs(z) < halfD + safe)
    {
        x = rightLimitL;
    }
}

bool nearArea( float x, float z, const Penalty& p, const Field& f)
{
    bool isXnear = (f.halfX - p.width - x < 0.1)?
                    true: false;
    bool isZnear = (abs(f.halfZ - p.depth/2 - z) < 0.1)?
                    true: false;
    return (isXnear || isZnear);
}

int nearBorderX(float x, const Field f)
{
    float borderRight = f.halfX - 0.1f;
    float borderLeft  = -f.halfX + 0.1f;

    if (x > borderRight)
        return 1;   // near top border
    else if (x < borderLeft)
        return -1;  // near bottom border
    else
        return 0;   // not near borders
}

int nearBorderZ(float z, const Field f)
{
    float borderTop    = f.halfZ - 0.1f;
    float borderBottom = -f.halfZ + 0.1f;

    if (z > borderTop)
        return 1;   // near left border
    else if (z < borderBottom)
        return -1;  // near right border
    else
        return 0;   // not near borders
}




