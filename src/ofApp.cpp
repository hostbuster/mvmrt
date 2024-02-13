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
    ofDisableAntiAliasing();
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
void drawThreadStatus(bool isTANReady, bool isTIPReady, size_t mappedFrame) {
    
    ofPushStyle();
    
#ifdef SHOW_THREADSTATS
    // Draw the filled rectangle based on the status
    if (isTANReady) {
        ofSetColor(ofColor::green);     // Set color to green
    } else {
        ofSetColor(ofColor::red);       // Set color to red
    }
    ofFill();  // Fill the rectangle
    ofDrawRectangle(20, 50, 60, 20);    // Draw a rectangle at position (100, 100) with width 200 and height 100
    
    // Draw the filled rectangle based on the status
    if (isTANReady) {
        ofSetColor(ofColor::green);     // Set color to green
    } else {
        ofSetColor(ofColor::red);       // Set color to red
    }
    ofFill();  // Fill the rectangle
    ofDrawRectangle(20, 80, 60, 20);    // Draw a rectangle at position (100, 100) with width 200 and height 100
#endif
    
    // frame index visualization
    int numRows = 3;
    int numCols = 60;
    int rectSize = 10;
    int borderSize = 1;
    
    int padding = ofGetWidth() - (numCols*(rectSize+borderSize)) / 2;
    // Loop through rows and columns
    for (int i = 0; i < numRows; ++i) {
        for (int j = 0; j < numCols; ++j) {
            int x = j * (rectSize + borderSize) + 390;
            int y = i * (rectSize + borderSize) + 20;
            
            // Set the border color to white
            ofSetColor(255);

            // Draw the border
            ofNoFill();
            ofDrawRectangle(x, y, rectSize + borderSize, rectSize + borderSize);


            // Set the interior color based on the filledIndex
            if ((i * numCols + j) == mappedFrame) {
                ofSetColor(255); // Solid white for the interior
            } else {
                ofSetColor(0);   // Black for the interior
            }

            // Draw the interior rectangle
            ofFill();
            ofDrawRectangle(x + borderSize, y + borderSize, rectSize - borderSize, rectSize - borderSize);
        }
    }
    
    
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::draw(){
    // set background to black
    ofBackground(0);
    
    // Calculate the position to center the image on the screen
    float x = 0; // (ofGetWidth() ) / 2.0;
    float y = 0; // (ofGetHeight() ) / 2.0;
    
    bool isTANReady;
    bool isTIPReady;
    // copy animation frame to canvas image
    if (isSetupReady) {
        tan->getFrame(imgCanvas, isTANReady, isTIPReady);
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
        
        drawThreadStatus(isTANReady, isTIPReady, mappedFrame);
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
