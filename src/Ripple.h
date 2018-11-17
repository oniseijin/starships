//
//  Ripple.h
//  Starships
//
//  Created by Ryan Mills on 2014-04-28.
//
//

#ifndef __Starships__Ripple__
#define __Starships__Ripple__

#include <iostream>
#include "ofMain.h"

/**
 This is meant to create a library out of the Ripple affect, to allow it to be customized and work in pass in images
 
 Original used loadPixels and pixels[] array, but this is meant to work on images themselves
 
 This works on an image level, rather than on an entire background drawing (but could be adapted to ripple the entire drawing)
 Originally from :
 
 http://www.openprocessing.org/sketch/7715
 
 Modified by Ryan Mills
 
 // A simple ripple effect. Click on the image to produce a ripple
 // Author: radio79
 // Code adapted from http://www.neilwallis.com/java/water.html
 
 
 */

class Ripple{
public:
    ofImage img;
    ofImage imgDest;
    int i, a, b;
    int oldind, newind, mapind;
    short* ripplemap; // the height map array
    int* col; // the actual pixels array
    int riprad;
    int rwidth, rheight;
    int* ttexture; // texture array
    int ssize;
    int offset; 
    Ripple(ofImage* imgSrc);
    void update();
    ofImage* getDestImage();
    void causeRipple(int x, int y, int mag);
    void newFrame();
    
    
    
};

#endif /* defined(__Starships__Ripple__) */
