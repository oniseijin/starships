#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    this->w = MAX_WIDTH;
    this->h = MAX_HEIGHT;
    // if screen is too small, don't show the controls
    //ofLogToConsole();
    ofLog(OF_LOG_NOTICE, "height:" + ofToString(ofGetScreenHeight()));
    if (ofGetScreenHeight() < MAX_HEIGHT){
        h = ofGetScreenHeight();
        controls = 0; // effectively hide the controls
    }
    if (ofGetScreenWidth() < MAX_WIDTH){
        w = ofGetScreenWidth();
    }
    verticalBounds = h - controls;
    starField.load("starfield-1500.jpg");
    backgroundRipple = new Ripple(&starField);
    //244, 187, 255,
    //127,229,238
    rgb* pink = new rgb(255,192,204);
    rgb* blue = new rgb(127, 229, 238);
    shipA = new Ship((int)ofGetWidth()/2, (int) verticalBounds /2, verticalBounds,  backgroundRipple, blue);
    shipB = new Ship((int)ofGetWidth()/2, (int) verticalBounds /2, verticalBounds,  backgroundRipple, pink);
    shipB->baseImage = "starship_purple.png";
    shipB->starship.load("starship_purple.png");
    shipB->starshipThrust.load("starship_purple_thrust.png");
    shipA->opponentShip = shipB;
    shipB->opponentShip = shipA;
    buttoncolor.set(204);
    highlight.set(153);
    
    
    int offset = ofGetWidth()/2 - 55;
    btnAShoot = new RectButton(offset + 40, verticalBounds +10, 19, buttoncolor, highlight);
    btnALeft = new RectButton(offset + 20, verticalBounds +30, 19, buttoncolor, highlight);
    btnAShield = new RectButton(offset + 40, verticalBounds +50, 19, buttoncolor, highlight);
    btnARight = new RectButton(offset + 60, verticalBounds +30, 19, buttoncolor, highlight);
    btnAThruster = new RectButton(offset + 40, verticalBounds +30, 19, buttoncolor, highlight);
    
    buttons.push_back(btnAShoot);
    buttons.push_back(btnALeft);
    buttons.push_back(btnAShield);
    buttons.push_back(btnARight);
    buttons.push_back(btnAThruster);
    
    /*
    gui.setup();
   
     // doesn't actually shoot unless this is within a panel
    btnAShoot.setup("shoot",  20, 20); // won't work without label
    btnAShoot.addListener(shipA, &Ship::shoot);
    btnAShoot.setPosition(offset + 40, verticalBounds + 10);
    //btnAShoot.setSize(20,20);
    // TODO, no color setting matters
    btnAShoot.setFillColor(buttonColor);
    btnAShoot.setDefaultFillColor(buttonColor);
    btnAShoot.setBackgroundColor(buttonColor);
    btnAShoot.setDefaultBorderColor(buttonColor);
    
    gui.add(&btnAShoot); // works when added to the panel
    buttons.push_back(btnAShoot);

    */
    mHide = true;
    menu.setup("menu");
    menu.add(aLazer.set("aLazer",ofColor(100,100,140),ofColor(0,0),ofColor(255,255)));
    menu.add(bLazer.set("bLazer",ofColor(100,100,140),ofColor(0,0),ofColor(255,255)));
   
   
    
    
   

}

//--------------------------------------------------------------
void ofApp::update(){
    // prevent resize
    int w = ofGetWidth();
	int h = ofGetHeight();
    if(w != this->w || h != this->h ){
        
        ofSetWindowShape(this->w, this->h);
        ofSetWindowPosition((w-this->w)/2, (h-this->h)/2); // center full screen again
        
    }
    shipA->update();
    shipB->update();
    if (RIPPLE)backgroundRipple->update();
    handleControls();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(255);
    if (RIPPLE) {
        backgroundRipple->getDestImage()->draw(0,0, ofGetWidth(), verticalBounds);
    } else {
        starField.draw(0,0, ofGetWidth(), verticalBounds);
    }
    
    if (ofGetFrameNum()%8==0) { // need a way to reset the acceleration after a timer, or reduce first
        shipA->halfAccelerate();
        shipB->halfAccelerate();
        
    }
    shipA->display();
    shipB->display();
    
    // controls area
    Button* button;
    for(int x = 0; x< buttons.size(); x++){
        button = buttons.at(x);
        button->display();
    }
    /*
    ofxButton* button;
    for(int x = 0; x< buttons.size(); x++){
        button = &buttons.at(x);
        button->draw();
    }
     */
    
    //gui.draw();
    
    if( !mHide ){
        menu.draw();
    }
    
    
   
    
}

void ofApp::handleControls(){
    Button* button;
    for(int x = 0; x< buttons.size(); x++){
        button = buttons.at(x);
        button->update();
    }
    
    // additional trigger for mouse controls
    if (mouseDown ){
        if (ofGetFrameNum()%2==0){
            if(btnAThruster->pressed()){
                shipA->thruster(-0.0125);
                // can't get the sound to work here for longer thrust
            }
        }
        if (ofGetFrameNum()%2==0){ // slightly faster
            if(btnALeft->pressed()){
                shipA->increaseRotation(-0.07);
            }
            if(btnARight->pressed()){
                shipA->increaseRotation(0.07);
            } 
            
        }
        
    }
    // ship a
    if (aDown && ofGetFrameNum()%2==0){
        shipA->increaseRotation(-0.07);
    } if (dDown && ofGetFrameNum()%2==0){
        shipA->increaseRotation(0.07);
    } if (sDown && ofGetFrameNum()%2==0) {
        shipA->thruster(-0.0125);
        shipA->thrusterLongSoundStart();
    }
    
    // ship b
    if ((lDown || fourDown) && ofGetFrameNum()%2==0){
        shipB->increaseRotation(-0.07);
    } if ((apposDown || sixDown) && ofGetFrameNum()%2==0){
        shipB->increaseRotation(0.07);
    } if ((colonDown || fiveDown ) && ofGetFrameNum()%2==0) {
        shipB->thruster(-0.0125);
        shipB->thrusterLongSoundStart();
    }
    
}


void ofApp::restart(){
    shipA->strength = 5;
    shipA->active = true;
    shipA->starship.load(shipA->baseImage);
    shipB->strength = 5;
    shipB->active = true;
    shipB->starship.load(shipB->baseImage);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    // TODO, want a way to continue taking some action until key released, even if another key is pressed (multiple actions at once) (would handle in update, and keep going until key is released)
    if (key == 'm'){
        mHide = !mHide;
    }
    
    if (key == 'a'){
        aDown = true;
        //shipA->increaseRotation(-0.15);
    } if (key == 'd'){
        dDown = true;
        //shipA->increaseRotation(0.15);
    } if (key == 's') {
        shipA->thrusterSound();
        sDown = true;
        //shipA->thruster(-0.05);
    } if(key == 'x'){
        xDown = true;
        shipA->shield->toggleShield();
    } if(key == 'w'){
        wDown = true;
        shipA->shoot();
    }
    // B
    if (key == '4'){
        fourDown = true;
    } if (key == '6'){
        sixDown = true;
    } if (key == '5') {
        shipB->thrusterSound();
        fiveDown = true;
    } if(key == '2'){
        twoDown = true;
        shipB->shield->toggleShield();
    } if(key == '8'){
        eightDown = true;
        shipB->shoot();
    // B alternative
    } if (key == ';'){
        shipB->thrusterSound();
        colonDown = true;
    } if (key == '\''){
        apposDown = true;
    } if (key == 'l') {
        lDown = true;
    } if(key == '/'){
        slashDown = true;
        shipB->shield->toggleShield();
    } if(key == 'p'){
        pDown = true;
        shipB->shoot();
    } if(key == 'g'){
        //loop(); // TODO
    } if(key == 'r'){ // restart
        restart(); // TODO
    }if (key == 'g'){
        // flip over the B ship
        shipA->baseImage = SHIP_GREEN_IMAGE;
        shipA->starship.load(SHIP_GREEN_IMAGE);
        shipA->starshipThrust.load(SHIP_GREEN_IMAGE);
        //shipA->increaseRotation(-0.15);
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if (key == 'a'){
        aDown = false;
        
    } if (key == 'd'){
        dDown = false;
        
    } if (key == 's') {
        shipA->thrusterLongSoundStop();
        sDown = false;
        
    } if(key == 'x'){
        xDown = false;
        
    } if(key == 'w'){
        wDown = false;
    } if(key == '4'){
        fourDown = false;
    }if(key == '5'){
        fiveDown = false;
        shipB->thrusterLongSoundStop();
    } if (key == '6') {
        sixDown = false;
        
    } if(key == '8'){
        eightDown = false;
    } if(key == '2'){
        twoDown = false;
    } if(key == 'p'){
        pDown = false;
    }if(key == 'l'){
        lDown = false;
    } if (key == '\'') {
        apposDown = false;
    } if(key == '/'){
        slashDown = false;
    } if(key == ';'){
        shipB->thrusterLongSoundStop();
        colonDown = false;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    mouseDown = true;
    if(btnAShield->pressed()){ // must not miss this, and related to a mouse press, not that it is down
        shipA->shield->toggleShield();
    } if (btnAShoot->pressed()){
        shipA->shoot();
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    mouseDown = false;
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
