#pragma once

class GameObject{
    public:
        float vel[2]={0,0};
        float pos[2]={0,0};
        float dim[2]={0,0};

        GameObject(){}
        GameObject(float velocity[2], float position[2], float size[2]){
            vel[0] = velocity[0];
            vel[1] = velocity[1];
            pos[0] = position[0];
            pos[1] = position[1];
            dim[0] = size[0];
            dim[1] = size[1];
        }
};