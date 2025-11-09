// EDACup example

#include <exception>
#include <cmath>
#include <iostream>
#include <string>
#include <stdio.h>

#include <nlohmann/json.hpp>

#define DEG_TO_RAD (3.1415926535F / 180.0F)

using namespace std;
using json = nlohmann::json;

typedef struct
{
    float position[3];
    float rotation[3];
    float velocity[3];
    float angularVelocity[3];
}ballState_t;

void trackBall(ballState_t &ballState, const json &message) {
    const auto &ball = message["data"]["ball"];

    ballState.position[0] = ball["position"][0];
    ballState.position[1] = ball["position"][1];
    ballState.position[2] = ball["position"][2];

    ballState.rotation[0] = ball["rotation"][0];
    ballState.rotation[1] = ball["rotation"][1];
    ballState.rotation[2] = ball["rotation"][2];

    ballState.velocity[0] = ball["velocity"][0];
    ballState.velocity[1] = ball["velocity"][1];
    ballState.velocity[2] = ball["velocity"][2];

    ballState.angularVelocity[0] = ball["angularVelocity"][0];
    ballState.angularVelocity[1] = ball["angularVelocity"][1];
    ballState.angularVelocity[2] = ball["angularVelocity"][2];
}

void chaseBall(const ballState_t &ballState)
{
    // Implement chasing logic here
    // For example, calculate the direction to the ball and set robot velocities accordingly

    float positionX = ballState.position[0];
    float positionZ = ballState.position[2];

    json sampleMessage = {
        {"type", "set"},
        {"data",
         {{
             "homeBot1",
             {
                 {"positionXZ", {positionX, positionZ}},
                 {"dribbler", 1}
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

int main(int argc, char *argv[])
{
    bool isRunning = false;
    uint32_t time = 0;
    ballState_t ball;


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
                    trackBall(ball, message);
                     // Moves robot every two seconds
                    if (time == 0)
                        chaseBall(ball);
                    time++;
                    if (time >= 50)
                        time = 0;
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

