#pragma once

#include "ofMain.h"
#include "Mvm.hpp"

class Star {
public:
    float x, y, z;
    float speed;

    Star(float _speed) {
        x = ofRandom(-ofGetWidth(), ofGetWidth());
        y = ofRandom(-ofGetHeight(), ofGetHeight());
        z = ofRandom(ofGetWidth());
        speed = _speed;
    }

    void update(float speedFactor) {
        z -= speed * speedFactor;

        if (z < 1) {
            x = ofRandom(-ofGetWidth(), ofGetWidth());
            y = ofRandom(-ofGetHeight(), ofGetHeight());
            z = ofGetWidth();
        }
    }

    void draw() {
        float px = ofMap(x / z, 0, 1, 0, ofGetWidth());
        float py = ofMap(y / z, 0, 1, 0, ofGetHeight());
        float size = ofMap(z, 0, ofGetWidth(), 8, 0);

        ofDrawCircle(px, py, size);
    }
};

class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;

		void keyPressed(int key) override;
		void keyReleased(int key) override;
		void mouseMoved(int x, int y ) override;
		void mouseDragged(int x, int y, int button) override;
		void mousePressed(int x, int y, int button) override;
		void mouseReleased(int x, int y, int button) override;
		void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
		void mouseEntered(int x, int y) override;
		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
		void dragEvent(ofDragInfo dragInfo) override;
		void gotMessage(ofMessage msg) override;
    
private:
    ofImage img1;
    ofImage img2;
    mvm::Mvm mvm;
    std::vector<ofImage> anim;
    uint32_t animFrame;
    uint32_t animFrameSub;
    
    // starfield
    int numStars;
    float speedFactor;
    std::vector<Star> stars;
		
};
