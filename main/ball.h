#pragma once 
#include "gameObject.h"

class Ball : public GameObject{
    public:
        Ball(float velocity[2], float position[2], float size[2]);
};