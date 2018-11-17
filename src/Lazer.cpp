//
//  Lazer.cpp
//  Starships
//
//  Created by Ryan Mills on 2014-04-19.
//
//

#include "Lazer.h"
#include "CollisionDetection.h"



rgb::rgb(int r, int g, int b){
    this->r = r;
    this->g = g;
    this->b = b;
}


Lazer::Lazer(Ship* ship, int verticalBounds, Ripple* ripple, rgb* color ){
    this->ship = ship;
    velocity = new ofVec2f();
    position = new ofVec2f();
    endPoint = new ofVec2f();
    framesAlive = 0;
    this->verticalBounds = verticalBounds;
    backgroundRipple = ripple;
    shoot();
    this->color= color;
    
}

void Lazer::update(){
    framesAlive++; // TODO, not incrementing?
    if (framesAlive > KEEP_ALIVE){
        active = false;
    }
    *position += *velocity;
    
    
    if (position->x < 0){
        position->x = ofGetWidth();
    } if (position->x > ofGetWidth()){
        position->x = 0;
    }if (position->y < 0){
        position->y = verticalBounds;
    } if (position->y > verticalBounds){
        position->y = 0;
    }
    
    endPoint->x = position->x + (cos(angle) * LENGTH);
    endPoint->y = position->y + (sin(angle) * LENGTH);
    
}

void Lazer::display(){
    
    ofPushStyle();
    ofSetColor(this->color->r, this->color->g, this->color->b, 255);
    //244, 187, 255,
    //127,229,238
    ofDrawLine(position->x, position->y, endPoint->x, endPoint->y);
    //ellipse(position.x, position.y, 15, 15); // to check the direction
    ofPopStyle();
    
}

bool Lazer::collision(Shield* shield){
    if(!shield->on) return false;
    
    bool collided = false;
    // TODO: collision based on simple two points and a circle: enhance the line check, and check multiple circles
    //ellipse(collideShip.position.x, collideShip.position.y+3, 17, 17); // collision circle
    
    if (pointBall(position->x, position->y, shield->ship->position->x, shield->ship->position->y, shield->size)){
        collided = true;
    } if (pointBall(endPoint->x, endPoint->y, shield->ship->position->x, shield->ship->position->y, shield->size)){
        collided = true;
    }
    if(framesAlive < 20 && shield == ship->shield){ // not hit oneself in the first second (why does extend the life?)
        collided = false;
        
    }
    if (collided == true){
        handleCollision();
        
    }
    return collided;
    
}
bool Lazer::collision(Ship* collideShip){
    if(!collideShip->active) return false;
    // ballLine collision is best
    bool collided = false;
    // TODO: collision based on simple two points and a circle: enhance the line check, and check multiple circles
    //ellipse(collideShip.position.x, collideShip.position.y+3, 30, 30); // collision circle
    
    if (pointBall(position->x, position->y, collideShip->position->x, collideShip->position->y+3, 30)){
        collided = true;
    } if (pointBall(endPoint->x, endPoint->y, collideShip->position->x, collideShip->position->y+3, 30)){
        collided = true;
    }
    if(framesAlive < 20 && collideShip == ship){ // not hit oneself in the first second (why does extend the life?)
        collided = false;
        
    }
    if (collided == true){
        handleCollision();
        
    }
    return collided;
}
void Lazer::handleCollision(){
    active = false;   // need to wait for one more frame
    // TODO put in ripple using GLGL
    backgroundRipple->causeRipple((int)position->x, (int)position->y, 256);
}

Lazer* Lazer::shoot(){
    angle = ship->rotation - HALF_PI;
    velocity->x = cos(angle) *SPEED;  // sin is actually the y component
    velocity->y = sin(angle) *SPEED; // cos is actually the x component
    position->set(ship->position->x, ship->position->y);
    active = true;
    return this;
}
