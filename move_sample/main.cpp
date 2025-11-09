// EDACup example
#ifndef LIBS
#define LIBS
#include <exception>
#include <cmath>
#include <iostream>
#include <string>
#endif
#include "colision.cpp"


#include <nlohmann/json.hpp>

#define DEG_TO_RAD (3.1415926535F / 180.0F)

#define areaInferioirBoundaryX 39
#define areaSuperiorBoundaryX 119
#define centreAreaBoundaryX 79
#define areaInferioirBoundaryZ 0
#define areaSuperiorBoundaryZ 25
#define centreAreaBoundaryZ 12.5

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


void trackObject(objectState_t &objectState, char object, const json &message) {
    switch (object)
    {
        case BALL: 
            const auto &object = message["data"]["ball"];
            break;
        case GOALIE:
            const auto &object = message["data"]["homeBot1"];
            break;
        case GOALKEEPER:
            const auto &object = message["data"]["homeBot2"];
            break;
    }

    objectState.position[0] = ball["position"][0];
    objectState.position[1] = ball["position"][1];
    objectState.position[2] = ball["position"][2];

    objectState.rotation[0] = ball["rotation"][0];
    objectState.rotation[1] = ball["rotation"][1];
    objectState.rotation[2] = ball["rotation"][2];

    objectState.velocity[0] = ball["velocity"][0];
    objectState.velocity[1] = ball["velocity"][1];
    objectState.velocity[2] = ball["velocity"][2];

    objectState.angularVelocity[0] = ball["angularVelocity"][0];
    objectState.angularVelocity[1] = ball["angularVelocity"][1];
    objectState.angularVelocity[2] = ball["angularVelocity"][2];
}

void chaseBall(const ballState_t &ballState, const Field f)
{
    // Implement chasing logic here
    // For example, calculate the direction to the ball and set robot velocities accordingly
    float positionX = ballState.position[0];
    float positionZ = ballState.position[2];

    clampToField(positionX, positionZ, f);

    json sampleMessage = {
        {"type", "set"},
        {"data",
         {{
             "homeBot1",
             {
                 {"positionXZ", {positionX, positionZ}},
                 {"dribbler", 1}
                 {"kicker", 1}
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


bool ballOutsideArea (float position, char axis)
{
    switch (axis)
    {
        case 'Z':
            if(position < areaSuperiorBoundaryZ && position > areaInferioirBoundaryZ)
            {
                return true;
            }
            else
            {
                return false;
            }
            break;
        case 'X':
            if(position < areaSuperiorBoundaryX && position > areaInferioirBoundaryX)
            {
                return true;
            }
            else
            {
                return false;
            }

            break;
        default:
            return false;
            break;
    }
}



void goalKeepeTracking(const objectState_t &ballState, const objectState_t &goalKeeper)
{
    float newGKPosition[2];

    float GKpositionX = ballState.position[0];
    float GKpositionZ = ballState.position[2];

    if(ballOutsideArea(ballState))
    {
        newGKPosition[0] = (areaSuperiorBoundaryX >= GKpositionX? GKpositionX : areaSuperiorBoundaryX);
        newGKPosition[1] = (areaSuperiorBoundaryZ >= GKpositionZ? GKpositionZ : areaSuperiorBoundaryZ);
    }
    if(GKBallDistance[0] < 1.0f && GKBallDistance[2] < 1.0f)
    {
        json sampleMessage = {
            {"type", "set"},
            {"data",
            {{
                "homeBot2",
                {
                    {"positionXZ", {newGKPosition[0]}, newGKPosition[2]}},
                    {"rotationY", {newGKPosition[2]}},
                    {"dribbler", 1}
                },
            }},
        };
    }
    else
    {
        json sampleMessage = {
            {"type", "set"},
            {"data",
            {{
                "homeBot2",
                {
                    {"positionXZ", {newGKPosition[0]}, newGKPosition[2]}},
                    {"rotationY", {newGKPosition[2]}},
                    {"kicker", 1}
                },
            }},
        };
    }

    // cout connects to server
    cout << sampleMessage.dump() << endl;

    // cerr prints to debug console
    cerr << "Updated homeBot2 defense." << endl;
}

int main(int argc, char *argv[])
{
    bool isRunning = false;
    uint32_t time = 0;
    objectState_t ball;
    objectState_t goalie;
    objectState_t goalKeeper;


    while (true)
    {
        // Try-catch allows intercepting errors.
        // On error, the catch block is executed.
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
                isRunning = true;
            else if (type == "stop")
                isRunning = false;
            else if (type == "state")
            {
                if (isRunning)
                {
                    trackObject(ball, BALL, message);                     // Moves robot every two seconds
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

