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
#define RIVAL1 '1'
#define RIVAL2 '2'

using namespace std;    
using json = nlohmann::json;

typedef struct{
    float position[3];
    float rotation[3];
    float velocity[3];
    float angularVelocity[3];
}objectState_t;


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
        case RIVAL1:
            key = "rivalBot1";
            break;
        case RIVAL2:
            key = "rivalBot2";
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

bool nearRival(const objectState_t& ballState, const json &message)
{
    objectState_t rival1;
    trackObject(rival1, RIVAL1, message);
    objectState_t rival2;
    trackObject(rival2, RIVAL2, message);

    float distanceToRival1 = sqrt(pow(ballState.position[0] - rival1.position[0], 2) +
                                 pow(ballState.position[2] - rival1.position[2], 2));
    float distanceToRival2 = sqrt(pow(ballState.position[0] - rival2.position[0], 2) +
                                    pow(ballState.position[2] - rival2.position[2], 2));
    
    if(distanceToRival1 < 0.2f || distanceToRival2 < 0.2f)
        return true;
    else
        return false;


}

void chaseBall(const objectState_t &ballState, const Field f, const Penalty p, const json &message)
{
    // Implement chasing logic here
    // For example, calculate the direction to the ball and set robot velocities accordingly
    float positionX = ballState.position[0];
    float positionZ = ballState.position[2];

    float kickVal = 1.0f;
    float rotateY = 0.0f;
    float chirpVal = 1.0f;

    avoidPenaltyAreas(positionX, positionZ, p, f);
    clampToField(positionX, positionZ, f);


    if(nearArea(positionX, positionZ, p, f))
    {
        kickVal = 1.0f;
    }
    else
    {
         kickVal = 0.0f;
    }

    if(int n = nearBorderX(positionX, f))
    {
        if(n == 1)
        {
            rotateY += 180 * DEG_TO_RAD;
        }
        else if (n == -1)
        {
            rotateY +=  180 * DEG_TO_RAD;
        }

    }
    else
    {
        rotateY = 0.0f;
    }

    if(int n = nearBorderZ(positionZ, f))
    {
        if(n == 1)
        {
            rotateY += 90 * DEG_TO_RAD;
        }
        else if (n == -1)
        {
            rotateY += -90 * DEG_TO_RAD;
        }   
    }

    if (nearRival(ballState, message))
    {
        chirpVal = 0.5f;
        kickVal = 0.5f;
    }
    {
        chirpVal = 0.0f;
    }

    json sampleMessage = {
            {"type", "set"},
            {"data",
            {{
                "homeBot1",
                {
                    {"positionXZ", {positionX, positionZ}},
                    {"rotationY", rotateY},
                    {"dribbler", 1},
                    {"kick", kickVal},
                    {"chirp", chirpVal}
                },
            }}},
        };

    // cout connects to server
    cout << sampleMessage.dump() << endl;

    // cerr prints to debug console
    cerr << "Updated homeBot1 pose." << endl;


}

void goalKeeperTracking(const objectState_t &ballState, const objectState_t &goalKeeper, const objectState_t &goalie, const Field& f, const Penalty& p)
{
    float newGKPosition[2];

    float ballPositionX = ballState.position[0];
    float ballPositionZ = ballState.position[2];

    float GKPositionX = goalKeeper.position[0];
    float GKPositionZ = goalKeeper.position[2];

    float GKBallDistanceX = fabs(GKPositionX - ballPositionX);
    float GKBallDistanceZ = fabs(GKPositionZ - ballPositionZ);

     //(xmin, ymin) (xmax,ymax). obs x es igual para las dos
    float linea_sup_arco[2][2] = {{-f.halfX +p.width-p.robotRadius-p.safetyMargin, -f.halfZ +p.depth-p.robotRadius-p.safetyMargin} ,
    {-f.halfX +p.width-p.robotRadius-p.safetyMargin, f.halfZ -p.depth +p.robotRadius+p.safetyMargin}};

    float dist_goalie_ball = std::sqrt (std::pow((goalie.position[0]-ballPositionX), 2.0f) + std::pow((goalie.position[2]-ballPositionZ), 2.0f) );

    //si la pelota esta lejos del area y fuera del rango z del arco
    if(ballPositionX >= linea_sup_arco[0][0]){
        newGKPosition[0] = linea_sup_arco[0][0];

        //si la pelota esta en el intervalo del eje z correspondiente a las coordenadas del arco
        if(linea_sup_arco[0][1] <= ballPositionZ && linea_sup_arco[1][1] >= ballPositionZ){

            if(dist_goalie_ball <= p.robotRadius){  //distancia minima para contemplar la posicion del goalie
                if( fabs(goalie.position[2]) < f.halfZ - (p.depth/2.0)*1.5){
                    newGKPosition[1] = goalie.position[2] -p.robotRadius*2;
                }
                else{
                    newGKPosition[1] = goalie.position[2] +p.robotRadius*2;
                }
            }
            else{
                newGKPosition[1] = ballPositionZ;
            }
        }
        else{
            if(dist_goalie_ball <= p.robotRadius){
                if( fabs(goalie.position[2]) >= f.halfZ- ((f.halfZ-p.depth/2.0) /2.0) ){
                    newGKPosition[1] = goalie.position[2] -p.robotRadius*2;
                }
                else{
                     newGKPosition[1] = goalie.position[2] +p.robotRadius*2;
                }
            }
            else{
                if(ballPositionZ <= linea_sup_arco[0][1]){
                    newGKPosition[1] = linea_sup_arco[0][1];
                }
                else{
                    newGKPosition[1] = linea_sup_arco[1][1];
                }
            }
        }
    }
    
    //si la pelota esta muy cerca del area se empieza a mover en x y seguir a la pelota
    else{       
        newGKPosition[0] = ballPositionX;
        newGKPosition[1] = ballPositionZ;
    }

    //corroboro que el robot no entre en el area del arco aunque la pelota este adentro
    avoidPenaltyAreas(newGKPosition[0], newGKPosition[1], p, f);

    json sampleMessage;

    if(GKBallDistanceX > 0.05f && GKBallDistanceZ > 0.05f)
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
                    chaseBall(ball,f,p, message);
                    goalKeeperTracking(ball, goalKeeper, goalie, f, p);

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
