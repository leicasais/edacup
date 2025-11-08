// EDACup example

#include <exception>
#include <cmath>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#define DEG_TO_RAD (3.1415926535F / 180.0F)

using namespace std;
using json = nlohmann::json;

// --- NUEVO: controlador de arquero para homeBot1 ---
void goalieControl(const json& message)
{
    // Guardamos una vez la "línea del arco" (el Z donde lo colocaste al inicio)
    static bool init = false;
    static float goalLineZ = 0.0f;

    const auto& data = message["data"];

    // Posiciones relevantes
    float bx = data["ball"]["position"][0];   // pelota X
    float bz = data["ball"]["position"][2];   // pelota Z
    float rx = data["homeBot1"]["position"][0];
    float rz = data["homeBot1"]["position"][2];
    float ry = data["homeBot1"]["rotation"][1];

    if (!init) { goalLineZ = rz; init = true; }  // fijamos línea del arco (Z) una sola vez

    // Reglas de arquero:
    // - SIEMPRE en la línea del arco (Z fijo)
    // - Seguir la pelota en el eje X
    float targetX = bx;
    float targetZ = goalLineZ;

    // Si la pelota está "al lado", patear hacia adelante a máxima potencia
    float dx = bx - rx, dz = bz - rz;
    float dist = std::sqrt(dx*dx + dz*dz);
    float kick = (dist < 0.12f) ? 1.0f : 0.0f;  // umbral cercano

    // Construimos el mensaje "set" solo para homeBot1
    json setMsg = {
        {"type", "set"},
        {"data", {{
            "homeBot1", {
                {"positionXZ", {targetX, targetZ}},
                {"rotationY",  ry},     // mantenemos orientación actual
                {"dribbler",   1},      // dribbler siempre prendido
                {"kick",       kick}    // máximo si está al lado
            }
        }}}
    };

    // Enviamos comando al simulador
    std::cout << setMsg.dump() << std::endl;

    // Debug opcional
    std::cerr << "Goalie update -> X:" << targetX
              << " Z:" << targetZ
              << " kick:" << kick << std::endl;
}

void poseHomeBot1(float positionX, float positionZ, float rotationY)
{
    json sampleMessage = {
        {"type", "set"},
        {"data",
         {{
             "homeBot1",
             {
                 {"positionXZ", {positionX, positionZ}},
                 {"rotationY", rotationY},
                 {"dribbler", 1}
             },
         }}},
    };

    // cout connects to server
    cout << sampleMessage.dump() << endl;

    // cerr prints to debug console
    cerr << "Updated homeBot1 pose." << endl;
}

int main(int argc, char *argv[])
{
    bool isRunning = false;
    uint32_t time = 0;

    while (true)
    {
        // Try-catch allows intercepting errors.
        // On error, the catch block is executed.
        try
        {
            string line;
            getline(cin, line);

            json message = json::parse(line);

            string type = message["type"];
            if (type == "start")
                isRunning = true;
            else if (type == "stop")
                isRunning = false;
            else if (type == "state")
            {
                if (isRunning)
                {
                    // Moves robot every two seconds
                    if (time == 0)
                        poseHomeBot1(-0.9, 0.0, -90 * DEG_TO_RAD);
                    else if (time == 40)
                        poseHomeBot1(-0.09, 0.0, -90 * DEG_TO_RAD);
                    time++;
                    if (time >= 80)
                        time = 0;
                    
                    goalieControl(message);

                }
            }
        }
        catch (exception &error)
        {
            // cerr prints to debug console
            cerr << error.what() << endl;
        }
    }
}
