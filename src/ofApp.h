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

class ThreadPattern : public ofThread {
public:
    // thread parameters
    ofImage& image;  // Reference to start frame ofImage
    float seed;
    ofColor backgroundColor;
    bool fillImage;

    // thread management
    std::mutex mutex;
    bool dataReady = false;
    
    // Constructor taking references and values
    ThreadPattern(ofImage& _image, float _seed, ofColor _backgroundColor, bool _fillImage)
        : image(_image), seed(_seed), backgroundColor(_backgroundColor), fillImage(_fillImage) {
            fmt::println("TPA constructor()");
        }

    // Setup function to initialize parameters
    void setup(ofImage& _image, float _seed, ofColor _backgroundColor, bool _fillImage) {
        std::lock_guard<std::mutex> lock(mutex);
        fmt::println("TPA setup()");
        // setReady(false);
        image = _image;
        seed = _seed;
        backgroundColor = _backgroundColor;
        fillImage = _fillImage;
        // setReady(true);
    }
    
    void threadedFunction() {
        // Set the flag to signal data readiness
        setReady(false);
        // Produce data
        generateData(image, seed, backgroundColor, fillImage);
        image.update();
        // Set the flag to signal data readiness
        setReady(true);
        
    }
    
    void generateData(ofImage &image, float seed, ofColor backgroundColor, bool fillImage) {
        std::lock_guard<std::mutex> lock(mutex);
        fmt::println("TPA generate start");
        if (fillImage) mvm.fillColor(image, backgroundColor);
        mvm.walker(image, seed, true);
        fmt::println("TPA generate end");
    }
    
    void setReady(bool ready) {
        std::lock_guard<std::mutex> lock(mutex);
        fmt::println("TPA  setReady {}", ready);
        dataReady = ready;
    }
    
    bool isReady() {
        std::lock_guard<std::mutex> lock(mutex);
        fmt::println("TPA  isReady()");
        return dataReady;
    }
    
private:
    mvm::Mvm mvm;
};

class ThreadInterpolate : public ofThread {
public:
    // thread parameters
    ofImage& img1;  // Reference to start frame ofImage
    ofImage& img2;   // Reference to end frame ofImage
    std::vector<ofImage>& anim;  // Reference to vector of ofImage
    size_t frames;
    ofColor backgroundColor;
    bool shuffleStartingPositions;
    //
    std::mutex mutex;
    bool dataReady = false;
    
    // Constructor taking references and values
    ThreadInterpolate(ofImage& _img1, ofImage& _img2, std::vector<ofImage>& _anim, size_t _frames, ofColor _backgroundColor, bool _shuffleStartingPositions)
        : img1(_img1), img2(_img2), anim(_anim), frames(_frames), backgroundColor(_backgroundColor), shuffleStartingPositions(_shuffleStartingPositions) {
            fmt::println("TIP constructor()");
        }

    // Setup function to initialize parameters
    void setup(ofImage& _img1, ofImage& _img2, std::vector<ofImage>& _anim, size_t _frames, ofColor _backgroundColor, bool _shuffleStartingPositions) {
        std::lock_guard<std::mutex> lock(mutex);
        fmt::println("TIP setup()");
        // setReady(false);
        img1 = _img1;
        img2 = _img2;
        anim = _anim;  // Here, ensure you are using the assignment operator, not initializing the reference
        frames = _frames;
        backgroundColor = _backgroundColor;
        shuffleStartingPositions = _shuffleStartingPositions;
        // setReady(true);
    }
    
    void threadedFunction() {
        // Set the flag to signal data readiness
        setReady(false);
        // Produce data
        generateData(img1, img2, anim, anim.size(), backgroundColor, shuffleStartingPositions);
        // Set the flag to signal data readiness
        setReady(true);
        
    }
    
    void generateData(ofImage &img1, ofImage& img2, std::vector<ofImage> &anim, size_t frames, ofColor backgroundColor, bool shuffleStaringPositions) {
        std::lock_guard<std::mutex> lock(mutex);
        fmt::println("TIP generate start");
        mvm.interpolate(img1, img2, anim, anim.size(), backgroundColor, shuffleStaringPositions);
        fmt::println("TIP generate end");
    }
    
    void setReady(bool ready) {
        std::lock_guard<std::mutex> lock(mutex);
        fmt::println("TIP  setReady {}", ready);
        dataReady = ready;
    }
    
    bool isReady() {
        std::lock_guard<std::mutex> lock(mutex);
        fmt::println("TIP  isReady()");
        return dataReady;
    }
    
private:
    mvm::Mvm mvm;
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
    ofColor colorBackground = {0,0,0, 255};
    
    ofImage img1;
    ofImage img2;
    ofImage imgCanvas;
    mvm::Mvm mvm;
    std::vector<ofImage> anim;
    uint32_t animFrame;
    uint32_t animFrameSub;
    
    // threads
    ThreadPattern* tpa1;
    ThreadPattern* tpa2;
    ThreadInterpolate* tip;
    
    // for starfield animation
    int numStars;
    float speedFactor;
    std::vector<Star> stars;
    
};
