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
    uint32_t width = ofGetWidth();
    uint32_t height = ofGetHeight();
    img1.setUseTexture(false);
    img2.setUseTexture(false);
    img1.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
    img2.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
    ofColor backgroundColor = {0,0,0, 255};
    anim.resize(5);
    for (int i=0; i< anim.size(); i++) {
        anim[i].setUseTexture(false);
        anim[i].allocate(width, height, OF_IMAGE_COLOR_ALPHA);
    }
    float seed = ofRandom(0, 20);
    ofColor color = {0,0,0, 255};
    mvm.fillColor(img1, color);
    mvm.walker(img1, seed, true);
    img1.update();
    // set the first animation frame
    anim[0] = img1;
    
    seed = ofRandom(0, 20);
    mvm.fillColor(img2, color);
    mvm.walker(img2, seed, true);
    img2.update();
    color = { 255, 0, 0, 255 };
    ofColor color2 (0,255,0,255);
    // mvm.interpolate(img1, img2, anim, anim.size(), backgroundColor, false);
    tip = new ThreadInterpolate(img1, img2, anim, anim.size(), backgroundColor, false);
    // update animation frames before we draw it - otherwise you won't see a thing
    
    tip->startThread();
    tip->setThreadName("tip");
    // tip->waitForThread();

    
    // starfield
    numStars = 100;
    speedFactor = 1.0;
    
    for (int i = 0; i < numStars; i++) {
        stars.push_back(Star(ofRandom(1, 10)));
    }
    
}

//--------------------------------------------------------------
void ofApp::update(){
    if (tip->isReady()) {
        if (animFrame==0) {
            if (animFrameSub > 60) {
                animFrame++;
                animFrameSub=0;
            } else {
                animFrameSub++;
            }
        } else if (animFrame==anim.size()-1) {
            if (animFrameSub > 60) {
                animFrame++;
                animFrameSub=0;
            } else {
                animFrameSub++;
            }
        } else {
            animFrame++;
        }
        
        if (animFrame > anim.size()-1) {
            // calculate next animation
            ofColor backgroundColor = {0,0,0, 255};
            img1 = anim[anim.size()-1];
            img1.update();
            float seed = ofRandom(0, 20);
            ofColor color = {0,0,0, 255};
            mvm.fillColor(img2, color);
            mvm.walker(img2, seed, true);
            img2.update();
            
            // clear previous animation frames
            for (int i=0; i< anim.size(); i++) {
                mvm.fillColor(anim[i], backgroundColor);
            }
            // calculate new animation frames
            // mvm.interpolate(img1, img2, anim, anim.size(), backgroundColor, false);
            
            tip->setup(img1, img2, anim, anim.size(), backgroundColor, false);
            // update animation frames before we draw it - otherwise you won't see a thing
            tip->startThread();
            tip->waitForThread();
            
            animFrame = 0;
        }
    } // tip->ready
    
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
    float x = (ofGetWidth() - anim[0].getWidth()) / 2.0;
    float y = (ofGetHeight() - anim[1].getHeight()) / 2.0;
    
    // Draw the image at the calculated position
    imgCanvas = anim[animFrame];
    imgCanvas.setUseTexture(true);
    imgCanvas.update();
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
    tip->stopThread();  // Stop the background thread when exiting
    tip->waitForThread(true);  // Wait for the thread to finish before exiting
    delete tip;
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
