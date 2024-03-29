#pragma once

#include "ofMain.h"
#include "Mvm.hpp"


class Config {
public:
    bool displayStats = false;
};

class Particle {
public:
    ofVec2f position;
    ofVec2f velocity;
    ofVec2f acceleration;
    float radius;
    float lifespan;
    ofColor color;
    vector<ofVec2f> trail;
    size_t trailsize;
    uint32_t trailOpacity = 50;


    Particle(ofVec2f _position, ofColor _color, size_t _trailsize) {
        position = _position;
        velocity = ofVec2f(ofRandom(-5, 5), ofRandom(-5, 5));
        acceleration = ofVec2f(0, 0.2); // New: Gravity acceleration
        radius = ofRandom(1, 3);
        lifespan = 255.0; // Initial lifespan
        color = _color;
        trailsize = _trailsize;
    }

    void update() {
        velocity += acceleration; // New: Apply gravity
        position += velocity;
        lifespan -= 2.0;
        
        // Store the current position for the trail
        trail.push_back(position);

        // Remove older positions to limit the length of the trail
        while (trail.size() > trailsize) {
            trail.erase(trail.begin());
        }

        // Bounce off the walls (optional)
        if (position.x < 0 || position.x > ofGetWidth()) {
           velocity.x *= -0.8;
        }
        if (position.y < 0 || position.y > ofGetHeight()) {
           velocity.y *= -0.8;
        }
    }

    void draw() const {
        ofPushStyle();
        ofSetColor(color, lifespan);
        ofDrawCircle(position, radius);

        // Draw the particle trail
        ofSetColor(color, trailOpacity); // Adjust the alpha value for the trail
        ofNoFill();
        ofBeginShape();
        for (const auto& point : trail) {
            ofVertex(point.x, point.y);
        }
        ofEndShape(false);
        ofFill();
        ofPopStyle();
    }

    bool isDead() const {
        return (lifespan < 0);
    }
};

class ParticleSystem {
public:
    vector<Particle> particles;

    void addParticles(int num, ofVec2f position, ofColor color, size_t trailsize) {
        for (int i = 0; i < num; i++) {
            particles.push_back(Particle(position, color, trailsize));
        }
    }

    void update() {
        for (int i = particles.size() - 1; i >= 0; i--) {
            particles[i].update();
            if (particles[i].isDead()) {
                particles.erase(particles.begin() + i); // Remove dead particles
            }
        }
    }

    void draw() {
        for (const auto& particle : particles) {
            particle.draw();
        }
    }
};

class Star {
public:
    float x, y, z;
    float speed;
    ofColor colorWhite;
    
    struct Circle {
            float x, y, radius;
        };
    
    Star(float _speed, std::vector<ofColor> _colorTable) {
        x = ofRandom(-ofGetWidth(), ofGetWidth());
        y = ofRandom(-ofGetHeight(), ofGetHeight());
        z = ofRandom(ofGetWidth());
        speed = _speed;
        colorTable = _colorTable;
        colorStart = colorTable[ofRandom(0, colorTable.size()-1)];
        colorEnd = colorTable[ofRandom(0, colorTable.size()-1)];
        colorWhite = ofColor(255, 255, 255);
        // Load your sound file (replace "collision_sound.wav" with your actual sound file)
        /*
        collisionSound.load("16-bit-explosion_120bpm_C_major.wav");
        collisionSound.setVolume(1.0); // Adjust the volume if needed
        collisionSound.setMultiPlay(true); // Allow multiple instances of the sound to play simultaneously
         */
    }
    
    void update(float speedFactor) {
        z -= speed * speedFactor;
        
        if (z < 1) {
            x = ofRandom(-ofGetWidth(), ofGetWidth());
            y = ofRandom(-ofGetHeight(), ofGetHeight());
            z = ofGetWidth();
        }
        
        // explosion
        explosion.update();
    }
    
    // Function to check collision between two circles
    bool checkCollision(float x1, float y1, float r1, float x2, float y2, float r2) {
        // Calculate the distance between the centers of the circles
        float distance = ofDist(x1, y1, x2, y2);

        // Check if the distance is less than the sum of the radii
        return (distance < (r1 + r2));
    }
    
    // Function to check collision with existing stars
    bool checkCollision(const Circle& newStar, std::vector<Star>& stars) {
        for (auto& existingStar : stars) {
            // Calculate the distance between the centers of the circles
            // float distance = ofDist(newStar.x, newStar.y, existingStar.x, existingStar.y);
            
            // Calculate squared distance between the centers of the circles
            float dx = newStar.x - existingStar.x;
            float dy = newStar.y - existingStar.y;
            float squaredDistance = dx * dx + dy * dy;

            // Check if the squared distance is less than the squared sum of the radii
            float radius = ofMap(existingStar.z, 0, ofGetWidth(), 8, 0);
            float sumRadii = newStar.radius + radius;
            float squaredSumRadii = sumRadii * sumRadii;

            // Check if the distance is less than the sum of the radii
            if (squaredDistance < squaredSumRadii) {
                // ofColor colorEnd = colorTable[colorTable.size()-1];
                existingStar.colorEnd = colorWhite;
                // add explosion

                // Trigger the explosion by adding particles at the collision position
                int probability = ofRandom(0,10);
                if (!probability) {
                    size_t trailsize = ofRandom(10,50);
                    explosion.addParticles(ofRandom(3,5), ofVec2f(newStar.x, newStar.y), existingStar.colorStart, trailsize);
                }
                return true; // Collision detected
            }
        }
        return false; // No collision
    }
    
    void draw() {
        float px = ofMap(x / z, 0, 1, 0, ofGetWidth());
        float py = ofMap(y / z, 0, 1, 0, ofGetHeight());
        float size = ofMap(z, 0, ofGetWidth(), 8, 0);
        // interpolate Color from 0 to ofGetWidth
        float fraction = px / ofGetWidth();
        ofColor color = mvm.interpolate(colorStart, colorEnd, fraction);
        ofPushStyle();
        ofSetColor(color);
        ofDrawCircle(px, py, size);
        ofPopStyle();
    }
    
    void draw(std::vector<Star>& stars) {
        float px = ofMap(x / z, 0, 1, 0, ofGetWidth());
        float py = ofMap(y / z, 0, 1, 0, ofGetHeight());
        float size = ofMap(z, 0, ofGetWidth(), 8, 0);
        
        ofPushStyle();
        // check if the the current position does collide with any other star
        Circle circle = {px, py, size};
        if (checkCollision(circle, stars)) {
            // collision - set star to white
            ofColor color = {255, 255, 255, 255};
            ofSetColor(color);
            // collisionSound.play();
        } else {
            // interpolate Color from 0 to ofGetWidth
            float fraction = px / ofGetWidth();
            ofColor color = mvm.interpolate(colorStart, colorEnd, fraction);
            ofSetColor(color);
        };
        
        ofDrawCircle(px, py, size);
        ofPopStyle();
        
        // Draw explosions
        explosion.draw();
    }
    
private:
    std::vector<ofColor> colorTable;
    ofColor colorStart;
    ofColor colorEnd;
    mvm::Mvm mvm;
    
    // explosions
    // explosion particle system
    ParticleSystem explosion;
    ofSoundPlayer collisionSound;
};

class ThreadPattern : public ofThread {
public:
    // thread parameters
    ofImage& image;  // Reference to start frame ofImage
    float seed;
    ofColor backgroundColor;
    bool fillImage;
    
    // thread management
    std::recursive_mutex mutex;
    bool dataReady = false;
    
    // Constructor taking references and values
    ThreadPattern(ofImage& _image, float _seed, ofColor _backgroundColor, bool _fillImage)
    : image(_image), seed(_seed), backgroundColor(_backgroundColor), fillImage(_fillImage) {
        fmt::println("TPA constructor()");
    }
    
    // Setup function to initialize parameters
    void setup(ofImage& _image, float _seed, ofColor _backgroundColor, bool _fillImage) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
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
        std::lock_guard<std::recursive_mutex> lock(mutex);
        fmt::println("TPA generate start");
        if (fillImage) mvm.fillColor(image, backgroundColor);
        mvm.walker(image, seed, true);
        fmt::println("TPA generate end");
    }
    
    void setReady(bool ready) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        fmt::println("TPA setReady {}", ready);
        dataReady = ready;
    }
    
    bool isReady() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        fmt::println("TPA isReady()");
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
    std::recursive_mutex mutex;
    bool dataReady = false;
    
    // Constructor taking references and values
    ThreadInterpolate(ofImage& _img1, ofImage& _img2, std::vector<ofImage>& _anim, size_t _frames, ofColor _backgroundColor, bool _shuffleStartingPositions)
    : img1(_img1), img2(_img2), anim(_anim), frames(_frames), backgroundColor(_backgroundColor), shuffleStartingPositions(_shuffleStartingPositions) {
        fmt::println("TIP constructor()");
    }
    
    // Setup function to initialize parameters
    void setup(ofImage& _img1, ofImage& _img2, std::vector<ofImage>& _anim, size_t _frames, ofColor _backgroundColor, bool _shuffleStartingPositions) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
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
        std::lock_guard<std::recursive_mutex> lock(mutex);
        fmt::println("TIP generate start");
        mvm.interpolate(img1, img2, anim, anim.size(), backgroundColor, shuffleStaringPositions);
        fmt::println("TIP generate end");
    }
    
    void setReady(bool ready) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        fmt::println("TIP setReady {}", ready);
        dataReady = ready;
    }
    
    bool isReady() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        fmt::println("TIP isReady()");
        return dataReady;
    }
    
private:
    mvm::Mvm mvm;
};

class ThreadAnimation : public ofThread {
public:
    // thread parameters
    size_t currentFrame;
    std::vector<size_t> frames;
    ofColor backgroundColor;
    bool clearKeyframes;
    bool shuffleStartingPositions;
    //
    bool threadRunning = true;
    std::recursive_mutex mutex;
    bool dataReady = false;
    
    // Constructor taking references and values
    ThreadAnimation(ofColor _backgroundColor, bool _clearKeyframes, bool _shuffleStartingPositions)
    : backgroundColor(_backgroundColor), clearKeyframes(_clearKeyframes), shuffleStartingPositions(_shuffleStartingPositions)
    {
        fmt::println("TAN constructor()");
    }
    
    // Setup function to initialize parameters
    void setup() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        fmt::println("TAN setup()");
        
        uint32_t width = ofGetWidth();
        uint32_t height = ofGetHeight();
        img1.setUseTexture(false);
        img2.setUseTexture(false);
        img1.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
        img2.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
        anim.resize(60);
        for (int i=0; i< anim.size(); i++) {
            anim[i].setUseTexture(false);
            anim[i].allocate(width, height, OF_IMAGE_COLOR_ALPHA);
        }
        float seed = ofRandom(0, 20);
        // mvm.fillColor(img1, color);
        // mvm.walker(img1, seed, true);
        tpa1 = new ThreadPattern(img1, seed, backgroundColor, true);
        tpa1->setThreadName("tpa1");
        tpa1->startThread();
        // img1.update();
        // set the first animation frame
        anim[0] = img1;
        
        seed = ofRandom(0, 20);
        // mvm.fillColor(img2, color);
        // mvm.walker(img2, seed, true);
        tpa2 = new ThreadPattern(img2, seed, backgroundColor, true);
        tpa2->setThreadName("tpa2");
        tpa2->startThread();
        tpa1->waitForThread();
        tpa2->waitForThread();
        // img2.update();
        ofColor color2 (0,255,0,255);
        // mvm.interpolate(img1, img2, anim, anim.size(), backgroundColor, false);
        tip = new ThreadInterpolate(img1, img2, anim, anim.size(), backgroundColor, false);
        // update animation frames before we draw it - otherwise you won't see a thing
        
        tip->startThread();
        tip->setThreadName("tip");
        
        // setup keyframe
        for (size_t i=0; i < anim.size(); i++) {
            frames.push_back(0);
        }
        for (size_t i=0; i < anim.size(); i++) {
            frames.push_back(i);
        }
        for (size_t i=0; i < anim.size(); i++) {
            frames.push_back(anim.size()-1);
        }
        
        currentFrame=0;
    }
    
    void animate() {
        // calculate next animation
        // std::lock_guard<std::recursive_mutex> lock(mutex);
        fmt::println("animate");
        img1 = anim[anim.size()-1];
        img1.update();
        float seed = ofRandom(0, 20);
        mvm.fillColor(img2, backgroundColor);
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
    }
    
    void threadedFunction() {
        // Set the flag to signal data readiness
        setReady(false);
        // Produce data
        this->setup();
        // Set the flag to signal data readiness
        setReady(true);
        
        // loop
        // Set the flag to signal data readiness
        while (threadRunning) {
            setReady(false);
            // Produce animation
            if (this->isTIPReady()) this->animate();
            // Set the flag to signal data readiness
            setReady(true);
            // Sleep to avoid busy-waiting and reduce CPU usage
            ofSleepMillis(10); // Sleep for 10 milliseconds (adjust as needed)
        }
    }
    
    void getFrame(ofImage& output, bool& isTANReady, bool& isTIPReady) {
        // fmt::println("TAN getFrame - before mutex");
        if (frames.size()) {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            size_t mappedFrame = currentFrame % frames.size();
            // fmt::println("TAN getFrame: {} {}", currentFrame, mappedFrame);

            if (anim.size() && frames.size()) {
                output = anim[frames[mappedFrame]];
            }
            
            if (mappedFrame == frames.size()-1) {
                fmt::println("TAN getFrame - mappedFrame == frames.size()-1");
                while (!this->isTIPReady()) {
                    ofSleepMillis(30);
                }
                currentFrame++;
            } else {
                currentFrame++;
            }
        }
        isTANReady = this->isReady();
        isTIPReady = this->isTIPReady();
    }
    
    void getFrameInfo(size_t& _currentFrame, size_t& _mappedFrame) {
        _currentFrame = currentFrame;
        if (frames.size()) _mappedFrame = currentFrame % frames.size();
        
    }
    
    void setReady(bool ready) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        fmt::println("TAN setReady {}", ready);
        dataReady = ready;
    }
    
    bool isReady() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        fmt::println("TAN isReady()");
        return dataReady;
    }
    
    bool isTIPReady() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        fmt::println("TAN isTIPReady()");
        return tip->isReady();
    }
    
    void stop() {
        threadRunning = false;
        tip->stopThread();  // Stop the background thread when exiting
        tip->waitForThread(true);  // Wait for the thread to finish before exiting
        delete tip;
    }
    
private:
    mvm::Mvm mvm;
    
    // animation
    std::vector<ofImage> anim;
    
    ofImage img1;
    ofImage img2;
    
    // threads
    ThreadPattern* tpa1;
    ThreadPattern* tpa2;
    ThreadInterpolate* tip;
    
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
    // Configuration
    Config config;
    
    bool isSetupReady = false;
    ofColor colorBackground = {0,0,0, 255};
    
    ThreadAnimation* tan;
    ofImage imgCanvas;
    mvm::Mvm mvm;
    
    uint32_t animFrame;
    uint32_t animFrameSub;
    
    // for starfield animation
    int numStars;
    float speedFactor;
    std::vector<Star> stars;
    
    // shaders
    ofShader shader;
    ofPlanePrimitive plane;
};
