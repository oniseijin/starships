//
//  Lazer.h
//  Starships
//
//  Created by Ryan Mills on 2014-04-19.
//
//

#ifndef __Starships__Lazer__
#define __Starships__Lazer__

#include <iostream>
class Ship;
class rgb; 
#include "Ship.h"
class Shield;
#include "Shield.h"
#include "Ripple.h"




class rgb {
public:
    int r;
    int g;
    int b;
    rgb(int r, int g, int b);
};


class Lazer{
 
public:
    Lazer(Ship* ship, int verticalBounds, Ripple* backgroundRipple, rgb* color);
    float LENGTH = 20;
    float SPEED = 11;
    int KEEP_ALIVE = 650;
    /** the tip of the lazer */
    ofVec2f* position;
    ofVec2f* endPoint;
    /** derived at the moment of being shot by the angle, ot derived each time*/
    ofVec2f* velocity;
    /** direction to calculate the end of the lazer */
    float angle;
    /** source ship */
    Ship* ship;
    /** count how long alive for */
    int framesAlive;
    int verticalBounds; 
    bool active;
    Ripple* backgroundRipple; 
    void update();
    void display();
    bool collision(Shield* shield);
    bool collision(Ship* ship);
    void handleCollision();
    Lazer* shoot();
    rgb* color; 
    
    
};
#endif /* defined(__Starships__Lazer__) */
