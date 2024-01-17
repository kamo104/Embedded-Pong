#include <TFT_eSPI.h>
#include <SPI.h>
#include <Arduino.h>
#include <Bluepad32.h>

#include "constants.h"
#include "game.h"
#include "gameObject.h"
#include "paddle.h"
#include "ball.h"

CtrlData *tmpData;

Game::Game(){
    tft = new TFT_eSPI();

    tft->begin();

    screen = new TFT_eSprite(tft);
    screen->setColorDepth(1);
    screen->createSprite(SCREEN_WIDTH,SCREEN_HEIGHT,1);
}

void Game::setup(){

    for(int i=0;i<3;i++){
        delete gameObjects[i];
    }

    frameStart = esp_timer_get_time()/1000;
    frameEnd = esp_timer_get_time()/1000;
    dt = 0;

    float vel[2] = {0.05,_scored?float(0.1*(_scored*2-3)):float(0.1)};

    float paddlePosition1[2] = {104,5};
    float paddlePosition2[2] = {104,305};
    float ballPosition[2] = {115,155};

    float paddleSize[2] = {32,10};
    float ballSize[2] = {10,10};

    gameObjects[0] = new Paddle(vel, paddlePosition1, paddleSize);
    gameObjects[1] = new Paddle(vel, paddlePosition2, paddleSize);
    gameObjects[2] = new Ball(vel, ballPosition, ballSize);

}

void Game::loop(){
    dt = (frameEnd - frameStart)/5;
    frameStart = esp_timer_get_time()/1000;

    if(_gameOver == false && _running == false && tmpData->miscButtons & 0x04){
        setup();
        _running = true;
    }
    if(_gameOver == true && _running == false && tmpData->miscButtons & 0x04){
        _score1 = 0;
        _score2 = 0;
        _scored = 0;
        setup();
        _running = true;
        _gameOver = false;
    }

    screen->fillSprite(TFT_BLACK);

    if(_running) onFrame();
    else if(!_gameOver){
        // print start screen
        screen->setTextColor(TFT_WHITE);
        screen->setTextSize(10);
        screen->setCursor(40,20);
        screen->printf("Pong");
        screen->setCursor(10,100);
        screen->setTextSize(2);
        screen->printf("Press START to play");
    }
    else if(_gameOver){
        // print game over screen
        screen->setTextColor(TFT_WHITE);
        screen->setTextSize(2);
        screen->setCursor(40,0);
        screen->setTextSize(3);
        screen->printf("Game Over");

        screen->setTextSize(2);
        screen->setCursor(0,80);
        screen->printf("Player %d wins",_scored);
        screen->setCursor(0,120);
        screen->printf("Press START to play again");

    }

    screen->pushSprite(0,0);
    frameEnd = esp_timer_get_time()/1000;
}

float paddleMove(GameObject *obj, float dt){
    float next_pos = obj->pos[0] + obj->vel[0]*dt;
    if(next_pos > SCREEN_WIDTH-obj->dim[0]){
        return SCREEN_WIDTH-obj->dim[0];
    }
    else if(next_pos < 0){
        return 0;
    }
    else{
        return next_pos;
    }
}

void Game::onFrame(){
    // pre-move and collision with sub tick precision step
    float projectedPositions[2] = {0,0};
    float projectedBallPosition[2] = {0,0};
    for(int i = 0; i < 3; i++){
        GameObject *obj = gameObjects[i];
        switch(i){
            case 0:
                obj->vel[0] = float(-1*tmpData->axisY)/512;
                projectedPositions[0] = paddleMove(obj, dt);
                break;
            case 1:
                obj->vel[0] = float(-1*tmpData->axisRY)/512;
                projectedPositions[1] = paddleMove(obj, dt);
                break;
            case 2:
                // check collision with paddles and walls
                projectedBallPosition[0] = obj->pos[0]+obj->vel[0]*dt;
                projectedBallPosition[1] = obj->pos[1]+obj->vel[1]*dt;

                if(projectedBallPosition[0] < 0){
                    projectedBallPosition[0] = -1*projectedBallPosition[0];
                    obj->vel[0] = -1*obj->vel[0];
                }
                else if(projectedBallPosition[0] > SCREEN_WIDTH-obj->dim[0]){
                    projectedBallPosition[0] = 2*(SCREEN_WIDTH-obj->dim[0])-projectedBallPosition[0];
                    obj->vel[0] = -1*obj->vel[0];
                }

                GameObject *paddle1 = gameObjects[0];
                GameObject *paddle2 = gameObjects[1];

                float paddleY1 = paddle1->pos[1]+paddle1->dim[1];
                if(projectedBallPosition[1] < paddleY1 && obj->pos[1] > paddleY1){
                    float projYTime = ((obj->pos[1]-paddleY1)/obj->vel[1])/5;
                    float projBallX = obj->pos[0]+obj->vel[0]*projYTime;
                    float projPaddleX = paddle1->pos[0]+paddle1->vel[0]*projYTime;

                    if((projBallX > projPaddleX && projBallX < projPaddleX+paddle1->dim[0]) ||
                        (projBallX+obj->dim[0] > projPaddleX && projBallX+obj->dim[0] < projPaddleX+paddle1->dim[0]))
                        {
                        obj->vel[1] = -1*obj->vel[1];
                        projectedBallPosition[1] = (dt-projYTime)*obj->vel[1]+paddleY1;

                        // add velocity from bounce
                        obj->vel[0] += paddle1->vel[0]/2;
                        obj->vel[1] *= 1.1;
                        Console.print("collision with paddle1\n");
                    }

                }

                float paddleY2 = paddle2->pos[1];
                if(projectedBallPosition[1]+obj->dim[1] > paddleY2 && obj->pos[1] + obj->dim[1] < paddleY2){
                    float projYTime = ((paddleY2-obj->pos[1]-obj->dim[1])/obj->vel[1])/5;
                    float projBallX = obj->pos[0]+obj->vel[0]*projYTime;
                    float projPaddleX = paddle2->pos[0]+paddle2->vel[0]*projYTime;

                    if((projBallX > projPaddleX && projBallX < projPaddleX+paddle2->dim[0]) || 
                        (projBallX+obj->dim[0] > projPaddleX && projBallX+obj->dim[0] < projPaddleX+paddle2->dim[0]))
                        {
                        obj->vel[1] = -1*obj->vel[1];
                        
                        projectedBallPosition[1] = paddleY2-(dt-projYTime)*obj->vel[1]-obj->dim[1];

                        // add velocity from bounce
                        obj->vel[0] += paddle1->vel[0]/2;
                        obj->vel[1] *= 1.1;
                        Console.print("collision with paddle2\n");
                    }
                }

                break;
        }

    }

    // move and draw step
    for(int i = 0; i < 3; i++){
        GameObject *obj = gameObjects[i];
        switch(i){
            case 0:
                obj->pos[0] = projectedPositions[0];
                break;
            case 1:
                obj->pos[0] = projectedPositions[1];
                break;
            case 2:
                obj->pos[0] = projectedBallPosition[0];
                obj->pos[1] = projectedBallPosition[1];
                
                break;
        }

        screen->drawRect(obj->pos[0],obj->pos[1],obj->dim[0],obj->dim[1],TFT_WHITE);
    }

    // scoring
    for(int i = 0; i < 3; i++){
        GameObject *obj = gameObjects[i];
        switch(i){
            case 2:
                if(obj->pos[1] < 0){
                    _running = false;
                    _score2 += 1;
                    _scored = 2;
                    if(_score2 >= 3){
                        _gameOver = true;
                        Console.print("Player 2 wins\n");
                    }
                    else
                        Console.print("Player 2 scores\n");
                }
                else if(obj->pos[1] > SCREEN_HEIGHT){
                    _running = false;
                    _score1 += 1;
                    _scored = 1;
                    if(_score1 >= 3){
                        _gameOver = true;
                        Console.print("Player 1 wins\n");
                    }
                    else
                        Console.print("Player 1 scores\n");
                }
                break;
        }
    }

    // draw scores
    screen->setTextColor(TFT_WHITE);

    screen->setCursor(0,16);
    screen->setTextSize(3);
    screen->printf("P1:");
    screen->printf("%d",_score1);

    screen->setCursor(0,SCREEN_HEIGHT-40);
    screen->setTextSize(3);
    screen->printf("P2:");
    screen->printf("%d",_score2);
}


void gameTask(void * pvParameters ){

    tmpData = (CtrlData *) pvParameters;
    Game game = Game();

    while(1){
        game.loop();
        vTaskDelay(1);
    }
}