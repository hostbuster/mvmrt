#include "ofApp.h"
#include "Mvm.hpp"


//--------------------------------------------------------------
void ofApp::setup(){
    // set framerate
    ofSetFrameRate(30);
    // Hide the mouse cursor
    ofHideCursor();
    // Set the application to fullscreen
    ofSetFullscreen(true);
    ofColor colorBackground = ofColor(0,0,0,255);
    tan = new ThreadAnimation(colorBackground, true, false);
    tan->startThread();

    
    // starfield
    numStars = 100;
    speedFactor = 1.0;
    
    for (int i = 0; i < numStars; i++) {
        stars.push_back(Star(ofRandom(1, 10)));
    }
    
}

//--------------------------------------------------------------
void ofApp::update(){
    // starfield
    for (auto& star : stars) {
        star.update(speedFactor);
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    // set background to black
    ofBackground(0);
    // Calculate the position to center the image on the screen
    float x = 0; // (ofGetWidth() ) / 2.0;
    float y = 0; // (ofGetHeight() ) / 2.0;
    
    // copy animation frame to canvas image
    tan->getFrame(imgCanvas);
    // required because the source image has the texture switched off ...
    imgCanvas.setUseTexture(true);
    // ... otherwise it would crash in the interpolation thread
    imgCanvas.update();
    // Draw the image at the calculated position
    imgCanvas.draw(x, y);
    // anim[animFrame].setUseTexture(false);
    
    // Draw stars with parallax scrolling
    for (int i = 0; i < 3; i++) {
        speedFactor = 1.0 + i * 0.5;  // Adjust parallax scrolling speed
        for (auto& star : stars) {
            star.draw();
        }
    }
}

//--------------------------------------------------------------
void ofApp::exit(){
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
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
