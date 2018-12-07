#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "Ship.h"
#include "Ripple.h"
#include "Buttons.h"
#include "Sizes.h"
#include "Images.h"


class ofApp : public ofBaseApp{

public:
    // global config
    bool RIPPLE = false;
    // end config
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void handleControls(); 
		
    Ship* shipA;
    Ship* shipB; 
    ofImage starField;
    int controls = 100;
    int verticalBounds;
    int w;
    int h; 
    bool mouseDown = false;
    Ripple* backgroundRipple;
    ofColor buttoncolor;
    ofColor highlight;
    /*
    ofxButton btnAShoot;
    ofxButton btnAThruster;
    ofxButton btnALeft;
    ofxButton bfnARight;
    ofxButton bntAShield;
    vector<ofxButton> buttons;
     */
    
    RectButton* btnAShoot;
    RectButton* btnAThruster;
    RectButton* btnALeft;
    RectButton* btnARight;
    RectButton* btnAShield;
    vector<Button *> buttons;
    
    // A key trackers
    bool aDown = false;
    bool sDown = false;
    bool wDown = false;
    bool dDown = false;
    bool xDown = false;
    
    // B key trackers
    bool fourDown = false;
    bool twoDown = false;
    bool fiveDown = false;
    bool eightDown = false;
    bool sixDown = false;
    // B alternative
    bool pDown = false;
    bool lDown = false;
    bool colonDown = false;
    bool apposDown = false;
    bool slashDown = false;
    
    
       
    void restart();
    
    // gui
    
    bool mHide; //menu hiding
    
    ofParameter<ofColor> aLazer;
    ofParameter<ofColor> bLazer; 
    ofxPanel menu;
    
    
};
