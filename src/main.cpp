#include "ofMain.h"
#include "ofApp.h"


//========================================================================
int main( ){
    int w = MAX_WIDTH;
    int h = MAX_HEIGHT;
	ofSetupOpenGL(w,h,OF_WINDOW);			// <-------- setup the GL context
    
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
