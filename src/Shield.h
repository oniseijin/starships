//
//  Shield.h
//  Starships
//
//  Created by Ryan Mills on 2014-04-19.
//
//

#ifndef __Starships__Shield__
#define __Starships__Shield__

#include <iostream>
class Ship; 
#include "Ship.h"

class Shield{
    
public:
    float STRENGTH_MAX = 5.0;
    float DECAY_RATE = 0.02;
    float CHARGE_RATE = 0.01;
    Ship* ship;
    float strength;
    bool on;
    bool empty;
    bool enabled;
    int size;
    Shield(Ship* ship);
    void update();
    void display();
    bool toggleShield();
    void handleCollision(Lazer lazer);
    
    
};

#endif /* defined(__Starships__Shield__) */
