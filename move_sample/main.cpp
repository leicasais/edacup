// EDACup example

#include <exception>
#include <cmath>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#define DEG_TO_RAD (3.1415926535F / 180.0F)

#define areaInferioirBoundaryX 39
#define areaSuperiorBoundaryX 119
#define centreAreaBoundaryX 79
#define areaInferioirBoundaryZ 0
#define areaSuperiorBoundaryZ 25
#define centreAreaBoundaryZ 12.5


using namespace std;
using json = nlohmann::json;

static inline float clampf(float v, float lo, float hi)
{ 
    return std::max(lo, std::min(hi, v)); 
}
static inline float norm2(float dx, float dz)
{ 
    return std::sqrt(dx*dx + dz*dz); 
}
static inline float yawTo(float fromX, float fromZ, float toX, float toZ)
{
    return std::atan2(toZ - fromZ, toX - fromX);
}



static void setHomeStriker(float x, float z, float yaw, float dribbler, float kick, float chirp)
{
    json sampleMessage = {
        {"type", "set"},
        {"data",
         {
             {
                 "homeBot1",
                 {
                     {"positionXZ", {x, z}},
                     {"rotationY", yaw},
                     {"dribbler", clampf(dribbler, 0.f, 1.f)},
                     {"kick",     clampf(kick,     0.f, 1.f)},
                     {"chirp",    clampf(chirp,    0.f, 1.f)},
                 },
             },
         }},
    };

    // cout connects to server
    cout << sampleMessage.dump() << endl;

    // cerr prints to debug console
    cerr << "Updated homeBot1 pose." << endl;
}

bool validateArea (float position, char axis)
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

void goalKeepeTracking(void)
{
    float ballPosition[3];
    float GKPosition[3];
    float newGKPosition[2];

    json sampleMessage = {
        "type": "state",
        {"data",
         {{
             "ball",
             {
                 {"position",ballPosition},
             },
             "homeBot2",
             {
                 {"position", GKPosition},
             },
         }}},
         
    };
    // cout connects to server
    cout << sampleMessage.dump() << endl;

    // cerr prints to debug console
    cerr << "Updated ball data." << endl;

    float GKBallDistance [3] = 
    {
        ballPosition[0] - GKPosition[0], 
        ballPosition[1] - GKPosition[1],
        ballPosition[2] - GKPosition[2]
    };

    if(GKBallDistance[0] < 0.0f)
    {
        if(validateArea(GKPosition[0] + 1, 'X'))
        {
            newGKPosition[0] = GKPosition[0] + 1;
        }else
        {
            newGKPosition[0] = GKPosition[0];
        }
    }
    else
    {
        if(validateArea(GKPosition[0] - 1, 'X'))
        {
            newGKPosition[0] = GKPosition[0] - 1;
        }else
        {
            newGKPosition[0] = GKPosition[0];
        }
    }

    if(GKBallDistance[2] < 0.0f)
    {
        if(validateArea(GKPosition[2] + 0.5f, 'Z'))
        {
            newGKPosition[1] = GKPosition[2] + 0.5f;
        }else
        {
            newGKPosition[1] = GKPosition[2];
        }
    }
    else
    {
        if(validateArea(GKPosition[2] - 0.5f, 'Z'))
        {
            newGKPosition[1] = GKPosition[2] - 0.5f;
        }else
        {
            newGKPosition[1] = GKPosition[2];
        }
    }
    
    json sampleMessage2 = {
        {"type", "set"},
        {"data",
         {{
             "homeBot2", 
             {
                {"positionXZ", newGKPosition},
            },
         }}},
    };


    // cout connects to server
    cout << sampleMessage2.dump() << endl;

    // cerr prints to debug console
    cerr << "Updated homeBot2 defense." << endl;
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
                        //setHomeStriker(-0.5, 0.3, 135 * DEG_TO_RAD);
                        goalKeepeTracking();
                    else if (time == 40)
                        //setHomeStriker(-0.1, 0.3, -135 * DEG_TO_RAD);
                        goalKeepeTracking();
                    time++;
                    if (time >= 80)
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
