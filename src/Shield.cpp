//
//  Shield.cpp
//  Starships
//
//  Created by Ryan Mills on 2014-04-19.
//
//

#include "Shield.h"

Shield::Shield(Ship* ship){
    this->ship = ship;
    strength = STRENGTH_MAX;
    on = false;
    empty = false;
    enabled = true;
    size = 40;
    
}

void Shield::update(){
    
    if (on){
        strength -= DECAY_RATE;
    }
    if (strength < STRENGTH_MAX ){
        strength += CHARGE_RATE;
    }  if (strength < 0.0){ // disabled
        enabled = false;
        on = false;
    } if(strength > STRENGTH_MAX){ // re-enabled
        strength = STRENGTH_MAX;
        enabled = true;
    }
    
    
}

void Shield::display(){
    if (on && enabled){
        ofPushStyle();
        ofNoFill();
        ofSetColor(255);
        ofPushMatrix();
        ofTranslate(0, 3);
        if(strength <= 1.0){
            ofSetColor(255, 0, 0);
        }
        ofDrawEllipse(ship->position->x, ship->position->y, size, size);
        ofPopMatrix();
        ofPopStyle();
    }
}

bool Shield::toggleShield(){
    if(enabled){ // only allow toggle if enabled (stay off)
        on = !on;
    }
    return on;
}
void Shield::handleCollision(Lazer lazer){
    strength -= 1.0;
}
