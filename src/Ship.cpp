
//  Ship.cpp
//  Starships
//
//  Created by Ryan Mills on 2014-04-19.
//
//

#include "Ship.h"
#include "ofMain.h"
#include "Lazer.h"
//#include <algorithm>

Ship::Ship(int x, int y, int verticalBounds, Ripple* ripple, rgb* lazerColor){
    position = new ofVec2f(x, y);
    velocity = new ofVec2f();
    acceleration = new ofVec2f();
    rotation = 0.0;
    strength = 5;
    shield = new Shield(this);
    starship.loadImage("starship1.png");
    //lazers = new ArrayList<Lazer>();
    //inactiveLazers =  new ArrayList<Lazer>();
    active = true;
    backgroundRipple = ripple;
    this->verticalBounds = verticalBounds;
    this->lazerColor = lazerColor;
    
    
}

void Ship::update(){
    // acceleration needs to be applied in the direction of the rotation
    
    *velocity+=*acceleration;
    velocity->limit(MAX_SPEED);
    *position+=*velocity;
    
    if (position->x < 0){
        position->x = ofGetWidth();
    } if (position->x > ofGetWidth()){
        position->x = 0;
    }if (position->y < 0){
        position->y = verticalBounds;
    } if (position->y > verticalBounds){
        position->y = 0;
    }
    
    shield->update();
    // need a different iterator when stage changes
    Lazer* lazer;
    for(int x = 0; x< lazers.size(); x++){
        lazer = &lazers.at(x);
        lazer->update();
        if(shield->on && lazer->collision(this->shield)){
            shield->handleCollision(*lazer);
        }
        else if(lazer->collision(this)){
            handleCollision(lazer);
        }
        if(!lazer->active){
            //inactiveLazers.push_back(lazer); //java avoided concurrent modification, does C++ suffer the same?
        }
    }
    // lazers from other ship
    for(int x = 0; x< opponentShip->lazers.size(); x++){
        lazer = &opponentShip->lazers.at(x);
        if(shield->on && lazer->collision(this->shield)){
            shield->handleCollision(*lazer);
        }
        else if(lazer->collision(this)){
            handleCollision(lazer);
        }
    }
    
    // iterate and skip from vector
    // http://stackoverflow.com/questions/9927163/erase-element-in-vector-while-iterating-the-same-vector
    vector<Lazer>::iterator it2;
    for(it2 = lazers.begin(); it2 != lazers.end();)
    {
        if(lazers.at(it2 - lazers.begin()).active == false)
        {
            it2 = lazers.erase(it2);
        }
        else
        {
            ++it2;
        }
    }
    inactiveLazers.clear();
    
    
    
}



void Ship::display(){
    
    for(Lazer lazer: lazers){
        lazer.display();
    }
    if(!active) return;
    shield->display();
    
    
    ofPushMatrix();
    ofTranslate(position->x, position->y);
    // ofRotate works in degrees, not radians
    ofRotate(ofRadToDeg(rotation));
    ofPushStyle();
    if(strength <= 1.0){
        ofPushStyle();
        ofSetColor(255, 0, 0); // in trouble, show red
        ofNoFill();
        ofEllipse(0, -7, 23, 23);
        ofPopStyle(); 
    }
    starship.draw(-25/2, -40/2, 25, 40);
    ofPopStyle();
    ofPopMatrix();
    
}



void Ship::shoot(){
    if(!active) return;
    if (lazers.size() <= MAX_LAZERS){
        Lazer* l = new Lazer(this, verticalBounds, backgroundRipple, this->lazerColor); // need to delete later
        lazers.push_back(*l->shoot());
    }
    
}

void Ship::thruster(float t){
    float r = rotation - HALF_PI; // sync the rotation up with the starting direction of the ship
    
    float f1 = cos(r) * t;  // sin is actually the y component
    float f2 = sin(r) * t;  // cos is actually the x component
    
    // thrust must go in the opposite direction
    f2 *= -1;
    f1 *= -1;
    acceleration->x += f1;
    acceleration->y += f2;
    
}

void Ship::changeAcceleration(float x, float y){
    
    //acceleration.add(x, y, 0);
    acceleration->x += x;
    acceleration->y += y;
    
}

void Ship::halfAccelerate(){
    if (acceleration->x != 0.0){
        acceleration->x = acceleration->x / 2;
    } if (acceleration->y != 0.0){
        acceleration->y = acceleration->y / 2;
    }
    
}

void Ship::increaseRotation(float rad){
    rotation += rad;
    if (rotation > TWO_PI){
        rotation = rotation - TWO_PI; // reset
    } if (rotation < 0 ){
        rotation = rotation + TWO_PI;
    }
}

void Ship::setAcceleration(float x, float y){
    acceleration->set(x,y);
}

/**
 Handle the collision with a lazer
 
 Will either reduce strength, shield life, or destroy ship
 */
void Ship::handleCollision(Lazer* collidedLazer){
    if(shield->on){
        shield->strength -= 1;
    } else{
        strength -= 1;
    }
    
    if (strength <= 0.0){
        active = false; // lose!
        // TODO, handle ripple with GLSL
        //backgroundRipple.causeRipple((int)position.x, (int)position.y, 4096);
    }
    
}
