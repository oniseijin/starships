#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    verticalBounds = ofGetHeight() - controls;
    starField.load("starfield-6x6.jpg");
    backgroundRipple = new Ripple(&starField);
    //244, 187, 255,
    //127,229,238
    rgb* pink = new rgb(244, 187, 255);
    rgb* blue = new rgb(127, 229, 238);
    shipA = new Ship((int)ofGetWidth()/2, (int) verticalBounds /2, verticalBounds,  backgroundRipple, blue);
    shipB = new Ship((int)ofGetWidth()/2, (int) verticalBounds /2, verticalBounds,  backgroundRipple, pink);
    shipB->starship.load("starship_purple.png");
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
   
   
    
    
   

}

//--------------------------------------------------------------
void ofApp::update(){
    // prevent resize
    int w = ofGetWidth();
	int h = ofGetHeight();
	if(w != 600 || h != 700) ofSetWindowShape(600, 700);
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
    }
    
    // ship b
    if (fourDown && ofGetFrameNum()%2==0){
        shipB->increaseRotation(-0.07);
    } if (sixDown && ofGetFrameNum()%2==0){
        shipB->increaseRotation(0.07);
    } if (fiveDown && ofGetFrameNum()%2==0) {
        shipB->thruster(-0.0125);
    }
    
}


void ofApp::restart(){
    shipA->strength = 5;
    shipA->active = true;
    shipB->strength = 5;
    shipB->active = true;
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    // TODO, want a way to continue taking some action until key released, even if another key is pressed (multiple actions at once) (would handle in update, and keep going until key is released)
    if (key == 'a'){
        aDown = true;
        //shipA->increaseRotation(-0.15);
    } if (key == 'd'){
        dDown = true;
        //shipA->increaseRotation(0.15);
    } if (key == 's') {
        sDown = true;
        //shipA->thruster(-0.05);
    } if(key == 'x'){
        xDown = true;
        shipA->shield->toggleShield();
    } if(key == 'w'){
        wDown = true;
        shipA->shoot();
    }
    if (key == '4'){
        fourDown = true;
        //shipA->increaseRotation(-0.15);
    } if (key == '6'){
        sixDown = true;
        //shipA->increaseRotation(0.15);
    } if (key == '5') {
        fiveDown = true;
        //shipA->thruster(-0.05);
    } if(key == '2'){
        twoDown = true;
        shipB->shield->toggleShield();
    } if(key == '8'){
        eightDown = true;
        shipB->shoot();
    } if(key == 'g'){
        //loop(); // TODO
    } if(key == 'r'){ // restart
        restart(); // TODO
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if (key == 'a'){
        aDown = false;
        
    } if (key == 'd'){
        dDown = false;
        
    } if (key == 's') {
        sDown = false;
        
    } if(key == 'x'){
        xDown = false;
        
    } if(key == 'w'){
        wDown = false;
    } if(key == '4'){
        fourDown = false;
    }if(key == '5'){
        fiveDown = false;
    } if (key == '6') {
        sixDown = false;
        
    } if(key == '8'){
        eightDown = false;
        
    } if(key == '2'){
        twoDown = false;
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
