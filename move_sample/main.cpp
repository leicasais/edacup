// ============================================================================
// EDACup 2025
// Based on CMDragons, ZJUNlict, and UT Austin Villa winning strategies
// Designed for maximum goal scoring and continuous pressure
// Sources: https://www.ifaamas.org/Proceedings/aamas2014/aamas/p493.pdf 
// https://www.cs.utexas.edu/~pstone/Papers/bib2html-links/LNAI12-MacAlpine.pdf 
// https://ceur-ws.org/Vol-1032/paper-23.pdf 
// ============================================================================

#include <exception>
#include <cmath>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#define FIELD_X 1.095F         // Half of field length (219 cm / 2)
#define FIELD_Z 0.79F          // Half of field width (158 cm / 2)
#define OPP_GOAL_X FIELD_X     // Opponent goal at positive X
#define OPP_GOAL_Z 0.0F        // Opponent goal centered at Z

using namespace std;
using json = nlohmann::json;

/**
 * @brief Clamp a float value between min and max
 *
 * @param v The value to clamp
 * @param mn The minimum value
 * @param mx The maximum value
 * @return The clamped value
 */
inline float clampf(float v, float mn, float mx) { 
    return max(mn, min(v, mx)); 
}

/**
 * @brief Calculate Euclidean distance between two 2D points
 *
 * @param x1 The x-coordinate of the first point
 * @param z1 The z-coordinate of the first point
 * @param x2 The x-coordinate of the second point
 * @param z2 The z-coordinate of the second point
 * @return The distance between the two points
 */
inline float dist2d(float x1, float z1, float x2, float z2) {
    return sqrt((x2-x1)*(x2-x1) + (z2-z1)*(z2-z1)); 
}

/**
 * @brief Calculate angle from point (x1, z1) to point (x2, z2)
 *
 * @param x1 The x-coordinate of the first point
 * @param z1 The z-coordinate of the first point
 * @param x2 The x-coordinate of the second point
 * @param z2 The z-coordinate of the second point
 * @return The angle in radians
 */
inline float angTo(float x1, float z1, float x2, float z2) {
    return atan2(z2-z1, x2-x1); 
}

/**
 * @brief Calculate normalized angle to [-PI, PI]
 *
 * @param a The angle in radians
 * @return The normalized angle in radians
 */
inline float normAng(float a) {
    while(a > M_PI) a -= 2*M_PI; 
    while(a < -M_PI) a += 2*M_PI; 
    return a; 
}

/**
 * @brief Send command to HomeBot1
 *
 * @param x The target x-coordinate
 * @param z The target z-coordinate
 * @param r The target rotation (radians)
 * @param dr The dribbler speed (0.0 to 1.0)
 * @param kick The kick power (0.0 to 1.0)
 */
static void HomeBot1(float x, float z, float r, float dr, float kick) {
    json msg = {
        {"type", "set"},
        {"data", {
            {"homeBot1", {
                {"positionXZ", {x, z}},
                {"rotationY", r},
                {"dribbler", dr},
                {"kick", kick},
                {"chip", 0.0}
            }}
        }}
    };
    cout << msg.dump() << endl;
}

int main(int argc, char *argv[]) {
    bool isRunning = false;
    int kickCooldown = 0;
    bool tienePelota = false;

    while (true) {
        try {
            string line;
            getline(cin, line);
            json message = json::parse(line);

            string type = message["type"];
            if (type == "start") {
                isRunning = true;
                kickCooldown = 0;
                tienePelota = false;
            }
            else if (type == "stop") {
                isRunning = false;
            }
            else if (type == "state") {
                if (isRunning) {
                    // 1. --- PERCEPTION ---
                    json data = message["data"];
                    float bx = data["ball"]["position"][0];
                    float bz = data["ball"]["position"][2];
                    float rx = data["homeBot1"]["position"][0];
                    float rz = data["homeBot1"]["position"][2];
                    float rr = data["homeBot1"]["rotation"][1];

                    if (kickCooldown > 0) kickCooldown--;

                    float distRobotBall = dist2d(rx, rz, bx, bz);
                    float distBallGoal = dist2d(bx, bz, OPP_GOAL_X, OPP_GOAL_Z);

                    // Angles
                    float anguloRobotPelota = angTo(rx, rz, bx, bz);
                    float anguloRobotArco = angTo(rx, rz, OPP_GOAL_X, OPP_GOAL_Z);
                    float anguloPelotaArco = angTo(bx, bz, OPP_GOAL_X, OPP_GOAL_Z);

                    // 2. --- DECISION ---
                    
                    // Histeresis para control de pelota
                    if (distRobotBall < 0.16F && !tienePelota) {
                        tienePelota = true;
                        cerr << ">>> PELOTA CONTROLADA! <<<" << endl;
                    } else if (distRobotBall > 0.28F && tienePelota) {
                        tienePelota = false;
                        cerr << ">>> Pelota perdida <<<" << endl;
                    }

                    float targetX, targetZ, targetRot;
                    float dribbler = 1.0;  // Siempre activado
                    float kick = 0.0;

                    if (tienePelota) {
                        // ====== TIENE LA PELOTA - AVANZAR PROGRESIVAMENTE ======
                        
                        // Calcular dirección desde pelota hacia arco
                        float dirX = OPP_GOAL_X - bx;
                        float dirZ = OPP_GOAL_Z - bz;
                        float dirLen = sqrt(dirX*dirX + dirZ*dirZ);
                        dirX /= dirLen;
                        dirZ /= dirLen;
                        
                        // Target: UN PASO adelante de la pelota hacia el arco
                        // NO ir directo al arco, sino avanzar de a poco
                        float pasoAvance = 0.25F;  // 25cm por vez
                        targetX = bx + pasoAvance * dirX;
                        targetZ = bz + pasoAvance * dirZ;
                        
                        // Rotar hacia el arco
                        targetRot = anguloPelotaArco;
                        
                        cerr << "AVANZANDO AL ARCO (ball:" << bx << "," << bz << ")" << endl;
                        cerr << "  Ball(" << bx << "," << bz << ") → Tgt(" 
                             << targetX << "," << targetZ << ") d=" << distBallGoal << "m" << endl;

                        // Patear cuando la pelota esté cerca del arco Y alineado
                        if (bx > 0.65F && kickCooldown == 0) {
                            float errorAngulo = fabs(normAng(rr - anguloRobotArco));
                            
                            if (errorAngulo < 0.4F) {  // ~23 grados
                                kick = 1.0;
                                kickCooldown = 25;
                                cerr << "*** PATEANDO!!! ***" << endl;
                            } else {
                                cerr << "  Alineando... err=" 
                                     << (errorAngulo*180/M_PI) << "°" << endl;
                            }
                        }
                    }
                    else {
                        // ====== NO TIENE LA PELOTA - IR A CAPTURARLA ======
                        
                        // SIMPLE: Ir directo a donde está la pelota
                        targetX = bx;
                        targetZ = bz;
                        targetRot = anguloRobotPelota;
                        
                        cerr << "PERSIGUIENDO (d=" << distRobotBall << "m)" << endl;
                        cerr << "  Ball(" << bx << "," << bz << ") → Tgt(" 
                             << bx << "," << bz << ") d=" << distRobotBall << "m" << endl;
                    }

                    // 3. --- ACTION ---
                    
                    // Límites del campo
                    targetX = clampf(targetX, -FIELD_X + 0.15F, FIELD_X - 0.15F);
                    targetZ = clampf(targetZ, -FIELD_Z + 0.15F, FIELD_Z - 0.15F);

                    HomeBot1(targetX, targetZ, targetRot, dribbler, kick);
                }
            }
        }
        catch (exception &error) {
            cerr << "ERROR: " << error.what() << endl;
        }
    }
}

