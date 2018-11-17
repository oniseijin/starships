//
//  Ripple.cpp
//  Starships
//
//  Created by Ryan Mills on 2014-04-28.
//
//

#include "Ripple.h"

//XXX, abandonded as performance is too slow, will need GLGL version

Ripple::Ripple(ofImage* imgSrc){
    img = *imgSrc;
    imgDest.allocate(this->img.getWidth(), this->img.getHeight(), OF_IMAGE_COLOR);
    offset = 3; // need to work in the image offset to deal with actual pixels
    // assume no loadPixels needed for now
    riprad = 3;
    rwidth = (int) img.getWidth() >> 1;
    rheight = (int) img.getHeight() >> 1;
    ssize = img.getWidth() * (img.getHeight() + 2) * 2;
    ripplemap = new short[ssize];
    col = new int[(int)img.getWidth() * (int) img.getHeight() *offset ];
    ttexture = new int[(int)img.getWidth() * (int) img.getHeight()];
    oldind = img.getWidth();
    newind = img.getWidth() * (img.getHeight() + 3);
    
    for(int loc=0; loc < img.getWidth() * img.getHeight() * offset; loc++){
        imgDest.getPixels()[loc] = img.getPixels()[loc];
        col[loc] = img.getPixels()[loc]; // just copy over
        }
    imgDest.update();
    
}
void Ripple::update(){
    newFrame();
    for(int loc=0; loc < img.getWidth() * img.getHeight() * offset; loc++){
        imgDest.getPixels()[loc] = col[loc];
    }
    imgDest.update();
    
}
ofImage* Ripple::getDestImage(){
    return &imgDest;
    
}
void Ripple::causeRipple(int x, int y, int mag){
    for (int j = y - riprad; j < y + riprad; j++) {
        for (int k = x - riprad; k < x + riprad; k++) {
            if (j >= 0 && j < img.getHeight() && k>= 0 && k < img.getWidth()) {
                ripplemap[oldind + (j * (int) img.getWidth()) + k] += mag;
            }
        }
    }
}
void Ripple::newFrame(){
    // update the height map and the image
    i = oldind;
    oldind = newind;
    newind = i;
    
    i = 0;
    mapind = oldind;
    for (int y = 0; y < img.getHeight(); y++) {
        for (int x = 0; x < img.getWidth(); x++) {
            short data = (short)((ripplemap[mapind - (int)img.getWidth()] + ripplemap[mapind + (int) img.getWidth()] +
                                  ripplemap[mapind - 1] + ripplemap[mapind + 1]) >> 1);
            data -= ripplemap[newind + i];
            data -= data >> 5;
            if (x == 0 || y == 0) // avoid the wraparound effect
                ripplemap[newind + i] = 0;
            else
                ripplemap[newind + i] = data;
            
            // where data = 0 then still, where data > 0 then wave
            data = (short)(1024 - data);
            
            // offsets
            a = ((x - rwidth) * data / 1024) + rwidth;
            b = ((y - rheight) * data / 1024) + rheight;
            
            //bounds check
            if (a >= img.getWidth())
                a = img.getWidth() - 1;
            if (a < 0)
                a = 0;
            if (b >= img.getHeight())
                b = img.getHeight()-1;
            if (b < 0) 
                b=0;
            
            //col[i] = img.getPixels()[a + (b * img.width)]; // pure version
            if (i >= (img.getWidth() * img.getHeight()) -3) return;
            for(int z=0; z<3; z++){ 
                col[(i*offset)+z] = img.getPixels()[((a + (b * img.getWidth())) *offset) + z];
            }
            mapind++;
            i++;
        }
    }
}

