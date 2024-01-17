#pragma once

#include "gameObject.h"

class Paddle : public GameObject{
    public:
        Paddle(float velocity[2], float position[2], float size[2]);
};