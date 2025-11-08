// EDACup example

#include <exception>
#include <cmath>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#define DEG_TO_RAD (3.1415926535F / 180.0F)

using namespace std;
using json = nlohmann::json;

void goalieControl(const json& message)
{
    static bool initialized = false;
    static float goalLineZ = 0.0f;

    const auto& data = message["data"];

    float pel_x = data["ball"]["position"][0];     // pelota X
    float pel_z = data["ball"]["position"][2];     // pelota Z
    float arq_x = data["homeBot1"]["position"][0]; // robot X
    float arq_z = data["homeBot1"]["position"][2]; // robot Z
    float arq_y = data["homeBot1"]["rotation"][1]; // orientación actual

    if (!initialized) 
    { 
        goalLineZ = arq_z; 
        initialized = true; 
    }

    float targetX = pel_x;
    float targetZ = goalLineZ;

    float dx = pel_x - arq_x, dz = pel_z - arq_z;
    float dist = std::sqrt(dx*dx + dz*dz);
    float kick = (dist < 0.12f) ? 1.0f : 0.0f;   

    json setMsg = {
        {"type", "set"},
        {"data", {
            {
                "homeBot1", {
                    {"positionXZ", {targetX, targetZ}},
                    {"rotationY",  arq_y},
                    {"dribbler",   1},
                    {"kick",       kick}
                }
            }
        }}
    };

    cout << setMsg.dump() << endl;              
    cerr << "Goalie -> targetX:" << targetX << " Z:" << targetZ << " kick:" << kick << endl;
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
