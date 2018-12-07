//
//  Ship.h
//  Starships
//
//  Created by Ryan Mills on 2014-04-19.
//
//

#ifndef __Starships__Ship__
#define __Starships__Ship__

#include <iostream>
#include "ofMain.h"
class Lazer;
#include "Lazer.h"
class Shield;
#include "Shield.h"
#include "Ripple.h"
#include "Sounds.h"



class Ship{
    
public:
    float MAX_SPEED = 10;
    ofVec2f* position;
    ofVec2f* velocity;
    ofVec2f* acceleration;
    float rotation;
    int strength; // strength never recharges
    Shield* shield; // must be a pointer
    vector<Lazer> lazers; // assumed pointers in vector
    vector<Lazer> inactiveLazers;
    int MAX_LAZERS = 10;
    bool active;
    bool thrustOn;
    std::string baseImage; 
    ofImage starship;
    ofImage starshipThrust; 
    ofSoundPlayer explosionSound;
    ofSoundPlayer hitSound;
    ofSoundPlayer thrustSound;
    ofSoundPlayer thrustLongSound; 
    Ship(int x, int y, int verticalBounds, Ripple* backgroundRipple, rgb* lazerColor);
    void display();
    void update();
    int verticalBounds;
    Ripple* backgroundRipple; 
    /** actually shoot */
    void shoot();
    void thruster(float t);
    void thrusterSound();
    void thrusterLongSoundStart();
    void thrusterLongSoundStop(); 
    void changeAcceleration(float x, float y);
    void halfAccelerate();
    void increaseRotation(float rad);
    void setAcceleration(float x, float y);
    void handleCollision(Lazer* collidedLazer);
    void setLazerColor(ofColor& color);
    Ship* opponentShip;
    rgb* lazerColor; 
    
};

#endif /* defined(__Starships__Ship__) */
