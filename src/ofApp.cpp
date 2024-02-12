#include "ofApp.h"
#include "Mvm.hpp"


//--------------------------------------------------------------
void ofApp::setup(){
    // set framerate
    ofSetFrameRate(30);
    // Hide the mouse cursor
    // ofHideCursor();
    // Set the application to fullscreen
    ofSetFullscreen(true);
    ofColor colorBackground = ofColor(0,0,0,255);
    tan = new ThreadAnimation(colorBackground, true, false);
    tan->startThread();

    
    // starfield
    numStars = 400;
    speedFactor = 1.0;
    std::vector<ofColor> colorTable(10);
    colorTable[0] = {144, 41, 27, 255};
    colorTable[1] = {213, 78, 49, 255};
    colorTable[2] = {235, 96, 67, 255};
    colorTable[3] = {39, 89, 84, 255};
    colorTable[4] = {62, 135, 129, 255};
    colorTable[5] = {90, 191, 180, 255};
    colorTable[6] = {243, 63, 51, 255};
    colorTable[7] = {178, 163, 40, 255};
    colorTable[8] = {236, 218, 66, 255};
    colorTable[9] = {255, 255, 255, 255};
    
    for (int i = 0; i < numStars; i++) {
        float speed = ofRandom(1, 10);
        stars.push_back(Star(speed, colorTable));
    }
    
#ifdef USESHADERS
    // shaders
    // ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
    if(ofIsGLProgrammableRenderer()){
        fmt::println("ofIsGLProgrammableRenderer: TRUE");
        shader.load("mvm");
    }else{
        fmt::println("ofIsGLProgrammableRenderer: FALSE");
        shader.load("mvm");
    }
    
    int planeWidth = ofGetWidth();
    int planeHeight = ofGetHeight();
    int planeGridSize = 20;
    int planeColums = planeWidth / planeGridSize;
    int planeRows = planeHeight / planeGridSize;
    
    plane.set(planeWidth, planeHeight, planeColums, planeRows, OF_PRIMITIVE_TRIANGLES);
#endif
    isSetupReady = true;
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
    if (isSetupReady && tan->isReady()) {
        tan->getFrame(imgCanvas);
    }
    
    // required because the source image has the texture switched off ...
    imgCanvas.setUseTexture(true);
    // ... otherwise it would crash in the interpolation thread
    imgCanvas.update();
    // Draw the image at the calculated position
    imgCanvas.draw(x, y);
        
    // Draw stars with parallax scrolling
    for (int i = 0; i < 3; i++) {
        speedFactor = 1.0 + i * 0.5;  // Adjust parallax scrolling speed
        for (auto& star : stars) {
            star.draw(stars);
        }
    }
    
    // stats
    if (config.displayStats) {
        // Display the current frame number
        ofSetColor(255);  // Set text color to white
        
        size_t mappedFrame;
        size_t currentFrame;
        tan->getFrameInfo(currentFrame, mappedFrame);
        ofDrawBitmapString("Frame: " + ofToString(currentFrame)+" "+ofToString(mappedFrame), 20, 20);
    }

    
}

//--------------------------------------------------------------
void ofApp::exit(){
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    switch(key) {
        case 104:
            config.displayStats = !config.displayStats;
            break;
        default:
            fmt::println("key: {}", key);
            break;
    }
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
