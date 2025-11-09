// EDACup example
#ifndef LIBS
#define LIBS
#include <exception>
#include <cmath>
#include <iostream>
#include <string>
#endif
#include "colision.hpp"


#include <nlohmann/json.hpp>

#define DEG_TO_RAD (3.1415926535F / 180.0F)

#define GOALKEEPER 'g'
#define GOALIE 'p'
#define BALL 'b'

using namespace std;    
using json = nlohmann::json;

typedef struct{
    float position[3];
    float rotation[3];
    float velocity[3];
    float angularVelocity[3];
}objectState_t;

// ===== NUEVO (Datasheet): Arquero que se mantiene en linea de arco, sigue X y patea al arco =====
void goalieControlFacingGoal(const json& message, float oppGoalX, float oppGoalZ)
{
    static bool initialized = false;
    static float goalLineZ = 0.0f;

    const auto& data = message["data"];

    // Estado (Datasheet): position = [x, y, z], rotation = [x, y, z]
    float bx = data["ball"]["position"][0];
    float bz = data["ball"]["position"][2];
    float rx = data["homeBot1"]["position"][0];
    float rz = data["homeBot1"]["position"][2];

    if (!initialized) { goalLineZ = rz; initialized = true; } // fijar linea del arco una sola vez

    // 1) Mantenerse en la linea del arco y seguir X de la pelota
    float targetX = bx;
    float targetZ = goalLineZ;

    // 2) Orientarse al centro del arco rival
    //    Convencion asumida: rotationY=0 mira hacia +Z. Ajustar si tu sim difiere.
    float dirX = oppGoalX - rx;
    float dirZ = oppGoalZ - rz;
    float yaw  = std::atan2(dirX, dirZ);

    // 3) Patear si la pelota esta cerca
    float dx = bx - rx, dz = bz - rz;
    float dist2 = dx*dx + dz*dz;
    float kick = (dist2 < (0.12f*0.12f)) ? 1.0f : 0.0f;

    // 4) Enviar "set" (estructura del Datasheet)
    json setMsg = {
        {"type", "set"},
        {"data", {
            {
                "homeBot1", {
                    {"positionXZ", {targetX, targetZ}},
                    {"rotationY",  yaw},
                    {"dribbler",   1},
                    {"kick",       kick}
                }
            }
        }}
    };

    std::cout << setMsg.dump() << std::endl;
    std::cerr << "Goalie(Face&Kick) -> X:" << targetX << " Z:" << targetZ << " yaw:" << yaw << " kick:" << kick << std::endl;
}



void trackObject(objectState_t &objectState, char objectType, const json &message) {

    std::string key;

    switch (objectType)
    {
        case BALL:
            key = "ball";
            break;
        case GOALIE:
            key = "homeBot1";
            break;
        case GOALKEEPER:
            key = "homeBot2";
            break;
        default:
            return; // o tirar error
    }

    const auto &jobj = message["data"][key];

    // pos
    objectState.position[0] = jobj["position"][0];
    objectState.position[1] = jobj["position"][1];
    objectState.position[2] = jobj["position"][2];

    // rot
    objectState.rotation[0] = jobj["rotation"][0];
    objectState.rotation[1] = jobj["rotation"][1];
    objectState.rotation[2] = jobj["rotation"][2];

    // vel
    objectState.velocity[0] = jobj["velocity"][0];
    objectState.velocity[1] = jobj["velocity"][1];
    objectState.velocity[2] = jobj["velocity"][2];

    // ang vel
    objectState.angularVelocity[0] = jobj["angularVelocity"][0];
    objectState.angularVelocity[1] = jobj["angularVelocity"][1];
    objectState.angularVelocity[2] = jobj["angularVelocity"][2];
}

void chaseBall(const objectState_t &ballState, const Field f, const Penalty p)
{
    // Implement chasing logic here
    // For example, calculate the direction to the ball and set robot velocities accordingly
    float positionX = ballState.position[0];
    float positionZ = ballState.position[2];

    clampToField(positionX, positionZ, f);
    avoidPenaltyAreas(positionX, positionZ, p, f);

    json sampleMessage = {
        {"type", "set"},
        {"data",
         {{
             "homeBot1",
             {
                 {"positionXZ", {positionX, positionZ}},
                 {"dribbler", 1},
                 {"kick", 1}
             },
         }}},
    };

    // cout connects to server
    cout << sampleMessage.dump() << endl;

    // cerr prints to debug console
    cerr << "Updated homeBot1 pose." << endl;


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


void goalKeeperTracking(const objectState_t &ballState, const objectState_t &goalKeeper, const Field& f, const Penalty& p)
{
    float newGKPosition[2];

    float ballPositionX = ballState.position[0];
    float ballPositionZ = ballState.position[2];

    float GKPositionX = goalKeeper.position[0];
    float GKPositionZ = goalKeeper.position[2];

    float GKBallDistanceX = fabs(GKPositionX - ballPositionX);
    float GKBallDistanceZ = fabs(GKPositionZ - ballPositionZ);

    // El arquero sigue la posición X de la pelota pero dentro del área penal
    newGKPosition[0] = ballPositionX;
    newGKPosition[1] = ballPositionZ;
    
    // Mantener al arquero DENTRO de su área penal
    keepInOwnPenaltyArea(newGKPosition[0], newGKPosition[1], p, f);

    json sampleMessage;

    if(GKBallDistanceX < 1.0f && GKBallDistanceZ < 1.0f)
    {
        sampleMessage = {
            {"type", "set"},
            {"data",
                {
                    {"homeBot2",
                        {
                            {"positionXZ", {newGKPosition[0], newGKPosition[1]}},
                            {"rotationY", 0},
                            {"dribbler", 1}
                        }
                    }
                }
            }
        };
    }
    else
    {
        sampleMessage = {
            {"type", "set"},
            {"data",
                {
                    {"homeBot2",
                        {
                            {"positionXZ", {newGKPosition[0], newGKPosition[1]}},
                            {"rotationY", 0},
                            {"kick", 1}
                        }
                    }
                }
            }
        };
    }

    cout << sampleMessage.dump() << endl;
    cerr << "Updated homeBot2 defense." << endl;
}

int main(int argc, char *argv[])
{
    bool isRunning = false;
    uint32_t time = 0;
    Penalty p;
    Field f;
    objectState_t ball;
    objectState_t goalie;
    objectState_t goalKeeper;


    while (true)
    {
        try
        {
            string line;
            getline(cin, line);

            if (line.empty())
                continue; // ignorar

            if (line[0] != '{' && line[0] != '[')
            {
                std::cout << "Línea ignorada, no es JSON: " << line << std::endl;
                continue;
            }

           json message = json::parse(line);

            string type = message["type"];
            if (type == "start")
            {
                isRunning = true;
            }
            else if (type == "stop")
                isRunning = false;
            else if (type == "state")
            {
                if (isRunning)
                {
                    trackObject(ball, BALL, message);              
                    trackObject(goalKeeper, GOALKEEPER, message);     
                    chaseBall(ball,f,p);
                    //goalKeeperTracking(ball, goalKeeper, f, p);
                    goalieControlFacingGoal(message, 0.0f, +0.9f);

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

