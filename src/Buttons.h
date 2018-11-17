//
//  Buttons.h
//  Starships
//  Ported to openframeworks  From http://processingjs.org/learning/topic/buttons/
//  Changes: popstyle, over as boolean
//  Created by Ryan Mills on 2014-05-05.
//
//

#ifndef __Starships__Buttons__
#define __Starships__Buttons__

#include <iostream>
#include "ofMain.h"


class Button{
    
public:
    int x, y;
    int size;
    ofColor basecolor, highlightcolor;
    ofColor currentcolor;
    bool overBoolean = false;
    bool locked = false;
    
    virtual void update();
    bool pressed();
    virtual bool over();
    bool overRect(int x, int y, int width, int height);
    bool overCircle(int x, int y, int diameter);
    virtual void display() = 0; 
    
    
    
};

class CircleButten: public Button{

public:
    CircleButten(int ix, int iy, int isize, ofColor icolor, ofColor ihighlight);
    bool over();
    void display(); 
    
};

class RectButton: public Button{
public:
    RectButton(int ix, int iy, int isize, ofColor icolor, ofColor ihighlight);
    bool over();
    void display(); 
    
};

class ImageButtons: public Button{
public:
    ImageButtons(int ix, int iy, int iw, int ih, ofImage ibase, ofImage iroll, ofImage idown);

    ofImage base;
    ofImage roll;
    ofImage down;
    ofImage currentimage;
    int w;
    int h;
    
    void update();
    bool over();
    void display();

};

#endif /* defined(__Starships__Buttons__) */
