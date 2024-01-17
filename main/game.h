#pragma once

#include <TFT_eSPI.h>
#include <SPI.h>
#include <Arduino.h>

#include "gameObject.h"

struct CtrlData{
    uint8_t dpad;
    uint16_t buttons;
    int32_t axisX;
    int32_t axisY;

    int32_t axisRX;
    int32_t axisRY;

    int32_t brake;
    int32_t throttle;
    uint16_t miscButtons;

};

void gameTask(void * pvParameters );


class Game{
    private:
        GameObject *gameObjects[3] = {nullptr,nullptr,nullptr};


        TFT_eSPI *tft;
        TFT_eSprite *screen;

        float dt;
        float frameStart;
        float frameEnd;
        bool _running = false;

        int _score1 = 0;
        int _score2 = 0;
        bool _gameOver = false;
        int _scored = 0;

    public:
        Game();
        void setup();
        void loop();
        void onFrame();
};
