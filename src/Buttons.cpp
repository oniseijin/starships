//
//  Buttons.cpp
//  Starships
//
//  Created by Ryan Mills on 2014-05-05.
//
//

#include "Buttons.h"
#include <cmath>

// begin Button
void Button::update()
{
    if(over()) {
        currentcolor = highlightcolor;
    }
    else {
        currentcolor = basecolor;
    }
}

bool Button::pressed()
{
    if(over()) {
        locked = true;
        
        return true;
    }
    else {
        locked = false;
        
        return false;
    }
}

bool Button::over(){
    return false; 
}

bool Button::overRect(int x, int y, int width, int height)
{
    if (ofGetMouseX() >= x && ofGetMouseX() <= x+width &&
        ofGetMouseY() >= y && ofGetMouseY() <= y+height) {
        return true;
    }
    else {
        return false;
    }
}

bool Button::overCircle(int x, int y, int diameter)
{
    float disX = x - ofGetMouseX();
    float disY = y - ofGetMouseY();
    if(sqrt(pow(disX, 2) + pow(disY, 2)) < diameter/2 ) {
        return true;
    }
    else {
        return false;
    }
}
// end Button

// begin CircleButton

CircleButten::CircleButten(int ix, int iy, int isize, ofColor icolor, ofColor ihighlight){
    x = ix;
    y = iy;
    size = isize;
    basecolor = icolor;
    highlightcolor = ihighlight;
    currentcolor = basecolor;
};

bool CircleButten::over()
{
    if( overCircle(x, y, size) ) {
        overBoolean = true;
        return true;
    }
    else {
        overBoolean = false;
        return false;
    }
}

void CircleButten::display()
{
    ofPushStyle();
    
    ofSetColor(currentcolor); // TODO, confirm if functionally the same
    ofFill(); // this is the above color
    //ofSetColor(255); // XXX use currentColor? (future colors, not the fill?)
    ofDrawEllipse(x, y, size, size);
    ofPopStyle();
}

// end CircleButton

// begin RectButton

RectButton::RectButton(int ix, int iy, int isize, ofColor icolor, ofColor ihighlight){
    x = ix;
    y = iy;
    size = isize;
    basecolor = icolor;
    highlightcolor = ihighlight;
    currentcolor = basecolor;
    
}

bool RectButton::over()
{
    if( overRect(x, y, size, size) ) {
        overBoolean = true;
        return true;
    }
    else {
        overBoolean = false;
        return false;
    }
}

void RectButton::display()
{
    ofPushStyle();
    ofSetColor(currentcolor); // TODO, confirm if functionally the same
    ofFill(); // this is the above color
    //ofSetColor(255); // XXX use currentColor? (future colors, not the fill?)
    ofDrawRectangle(x, y, size, size);
    ofPopStyle();
    
   
}

// end RectButton

// begin ImageButtons

ImageButtons::ImageButtons(int ix, int iy, int iw, int ih, ofImage ibase, ofImage iroll, ofImage idown){
    x = ix;
    y = iy;
    w = iw;
    h = ih;
    base = ibase;
    roll = iroll;
    down = idown;
    currentimage = base;
    
}

void ImageButtons::update()
{
    over();
    pressed();
    if(locked) {
        currentimage = down;
    } else if (overBoolean){
        currentimage = roll;
    } else {
        currentimage = base;
    }
}

bool ImageButtons::over()
{
    if( overRect(x, y, w, h) ) {
        overBoolean = true;
        return true;
        
    } else {
        overBoolean = false;
        return false;
        
    }
}

void ImageButtons::display()
{
    currentimage.draw(x, y);
}

// end ImageButtons
