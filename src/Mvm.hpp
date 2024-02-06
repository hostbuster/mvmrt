//
//  Mvm.hpp
//  mvmrt
//
//  Created by Matthias Strohmaier on 03.02.24.
//

#ifndef Mvm_hpp
#define Mvm_hpp

/*
 * maximal visual madness - call me after dark
 *
 ffmpeg -framerate 25 -pattern_type glob -i 'poly_*.png' -i 'shona.ogg' -c:v libx264 -c:a copy -shortest -r 30 -pix_fmt yuv420p mvm_anim__.mp4
 *
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include <utility>
#include <vector>
// #include <png.h>
#include <random>
#include <queue>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>
#include <fmt/core.h>
#include <locale>
#include "sndfile.h"
#include "ofImage.h"
#include "ofPixels.h"




/*
 * maximal visual madness - call me after dark
 *
 ffmpeg -framerate 25 -pattern_type glob -i 'poly_*.png' -i 'shona.ogg' -c:v libx264 -c:a copy -shortest -r 30 -pix_fmt yuv420p mvm_anim__.mp4
 *
 */


// using namespace std;
namespace mvm {

template <typename T> int sgn(T x) {
    //return (T(0) < val) - (val < T(0));
    return ((T)((x) > 0) - (T)((x) < 0));
}

class Config {
public:
    std::string path;
};


struct Pixel {
    uint32_t x;
    uint32_t y;
    ofColor color;
};

class ColorGradient {
private:
    struct ColorPoint  // Internal class used to store colors at different points in the gradient.
    {
        float r, g, b;      // Red, green and blue values of our color.
        float val;        // Position of our color along the gradient (between 0 and 1).
        ColorPoint( float red, float green, float blue, float value )
        : r(red), g(green), b(blue), val(value) {
        }
    };
    std::vector<ColorPoint> color;      // An array of color points in ascending value.
    
public:
    //-- Default constructor:
    ColorGradient() {
        createDefaultHeatMapGradient();
    }
    
    //-- Inserts a new color point into its correct position:
    void addColorPoint( float red, float green, float blue, float value ) {
        for (int i = 0; i < color.size(); i++) {
            if ( value < color[i].val ) {
                color.insert(color.begin() + i, ColorPoint(red, green, blue, value));
                return;
            }
        }
        color.emplace_back(red, green, blue, value);
    }
    
    //-- Inserts a new color point into its correct position:
    void clearGradient() {
        color.clear();
    }
    
    //  col 01
    
    //-- Places a 5 color heapmap gradient into the "color" vector:
    //    void createDefaultHeatMapGradient() {
    //        color.clear();
    //        color.push_back(ColorPoint(0, 0, 0, 0.0f));      // Black.
    //        color.push_back(ColorPoint(0, 0, 1, 0.125f));    // Blue.
    //        color.push_back(ColorPoint(0, 1, 1, 0.25f));     // Cyan.
    //        color.push_back(ColorPoint(0, 1, 0, 0.5f));      // Green.
    //        color.push_back(ColorPoint(1, 1, 0, 0.75f));     // Yellow.
    //        color.push_back(ColorPoint(1, 0, 0, 1.0f));      // Red.
    //    }
    
    //  col 02
    void createDefaultHeatMapGradient() {
        color.clear();
        color.emplace_back(0.00, 0.00, 0.00, 0.000f);                  // Black.
        color.emplace_back(0.00, 0.00, 0.50, 0.250f);                  // Dark Blue.
        color.emplace_back(0.50, 0.00, 0.50, 0.375f);                  // Dark Violet.
        color.emplace_back(1.00, 0.00, 0.00, 0.500f);                  // Red.
        color.emplace_back(1.00, 0.50, 0.00, 0.750f);                  // Orange.
        color.emplace_back(1.00, 1.00, 0.00, 0.875f);                  // Yellow.
        color.emplace_back(1.00, 1.00, 1.00, 1.000f);                  // White.
    }
    
    
    //-- Inputs a (value) between 0 and 1 and outputs the (red), (green) and (blue)
    //-- values representing that position in the gradient.
    void getColorAtValue( const float value, float &red, float &green, float &blue ) {
        if ( color.empty() )
            return;
        
        for (int i = 0; i < color.size(); i++) {
            ColorPoint &currC = color[i];
            if ( value < currC.val ) {
                ColorPoint &prevC = color[std::max(0, i - 1)];
                float valueDiff = ( prevC.val - currC.val );
                float fractBetween = ( valueDiff == 0 ) ? 0 : ( value - currC.val ) / valueDiff;
                red = ( prevC.r - currC.r ) * fractBetween + currC.r;
                green = ( prevC.g - currC.g ) * fractBetween + currC.g;
                blue = ( prevC.b - currC.b ) * fractBetween + currC.b;
                return;
            }
        }
        red = color.back().r;
        green = color.back().g;
        blue = color.back().b;
    }
};

const int NUMBEROFCOLORS = 500;
const uint8_t VALUES_18BPP[] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80, 84,
    88, 92, 96, 100, 104, 108, 112, 116, 120, 126, 130, 136, 140, 144, 148, 152, 156,
    160, 164, 168, 172, 176, 180, 184, 188, 192, 196, 200, 204, 208, 212, 216, 220, 224,
    228, 232, 236, 240, 244, 248, 252, 255};

/////////////////////////////////////////////////////////////////////////////
//    Floyd-Steinberg dither
/////////////////////////////////////////////////////////////////////////////

//    Floyd-Steinberg dither uses constants 7/16 5/16 3/16 and 1/16
//    But instead of using real arythmetic, I will use integer on by
//    applying shifting ( << 8 )
//    When use the constants, don't foget to shift back the result ( >> 8 )
#define    f7_16    112        //const int    f7    = (7 << 8) / 16;
#define    f5_16     80        //const int    f5    = (5 << 8) / 16;
#define    f3_16     48        //const int    f3    = (3 << 8) / 16;
#define    f1_16     16        //const int    f1    = (1 << 8) / 16;



class Mvm {
public:
    std::string formatBytes(size_t bytes);
    std::string timeToFinish(size_t items_done, size_t items_total, size_t seconds_elapsed);
    std::string timeElapsed(size_t items_done, size_t seconds_elapsed);
    std::string formatEta(size_t items_done, size_t items_total, const std::chrono::steady_clock::time_point time_start, size_t bytes_read);
    size_t getImageSizeInBytes(const ofImage& image);
    [[nodiscard]] inline size_t pixelIndex(ofImage image, size_t x, size_t y);
    bool isFileEmpty(const std::string& filename);
    bool fileExists(const std::string &filename);
    // color manipulation
    inline ofColor mixColors(ofColor color1, ofColor color2, float weight);
    inline void blendColor(ofColor &c1, ofColor c2);
    bool barycentric(int x1, int y1, int x2, int y2, int x3, int y3, int xp, int yp, int &u1, int &u2, int &det);
    inline ofColor mixColors(ofColor c1, ofColor c2, ofColor c3, int u1, int u2, int det);
    inline ofColor addColors(ofColor color1, ofColor color2);
    ofColor interpolate(ofColor c1, ofColor c2, float fraction);
    mvm::Pixel interpolate(const mvm::Pixel &p1, const mvm::Pixel &p2, float fraction);
    
    std::vector<ofColor> generate_heatmap_color_table(int n);
    inline bool getPixel(ofImage &image, uint32_t x, uint32_t y, ofColor &color);
    
    ofColor getPixel(ofImage &image, uint32_t index);
    void setPixel(ofImage &image, uint32_t x, uint32_t y, ofColor color);
    void setPixel(ofImage &image, uint32_t x, uint32_t y, ofColor colorNew, float weight);
    void setPixelAdd(ofImage &image, uint32_t x, uint32_t y, ofColor colorNew);
    void gaussianBlurRGBA(ofImage &image);
    void replaceColor(ofImage &image, ofColor colorFind, ofColor colorReplace);
    void makeDitherFSRgb18bpp(ofImage &image) noexcept;
    bool hasNeighborOfColorC(ofImage &image, uint32_t x, uint32_t y, ofColor color);
    
    // fills
    void fillColor(ofImage& image, const ofColor& color);
    size_t edgeFill(ofImage &image, uint32_t x, uint32_t y, ofColor newColor, ofColor oldColor, ofColor edgeColor);
    size_t edgeFill(ofImage &image, uint32_t x, uint32_t y, ofColor newColor, ofColor edgeColor, bool verbose);
    size_t
    floodFill(ofImage &image, uint32_t x, uint32_t y, ofColor newColor, ofColor oldColor, const bool test, const size_t maxPixels);
    size_t floodFill(ofImage &image, uint32_t x, uint32_t y, ofColor newColor, ofColor excludeColor, bool test, size_t maxPixels, bool verbose);
    // lines
    inline void drawLineGradient(ofImage &image, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, ColorGradient &gradient, float gradientEnd, unsigned char alpha = 255);
    inline void drawLine(ofImage &image, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, ofColor color, uint32_t threshold_manhattan_length = 0);
    void drawLinesAroundCenter(ofImage &image, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, ofColor color,
                               uint32_t threshold_manhattan_length = 0);
    // triangles
    inline void normalizeTrianglePoints(int& x1, int& y1, int& x2, int& y2, int& x3, int& y3);
    inline bool normalizeTriangle(size_t width, size_t height, int x1, int y1, int x2, int y2, int x3, int y3, int &lx, int &hx, int &ly, int &hy);
    void drawSolidTriangle(ofImage &image, int x1, int y1, int x2, int y2, int x3, int y3,
                           ofColor c1, ofColor c2, ofColor c3);
    void drawSolidTriangle(ofImage &image, int x1, int y1, int x2, int y2, int x3, int y3, ofColor &color);
    std::vector<mvm::Pixel> displacePixels(ofImage &image, uint32_t n);
    size_t getPixelsNotHavingBackground(ofImage &image, ofColor colorBackground, std::vector<mvm::Pixel> &pixels);
    void drawPixelsIfNotProtected(ofImage &image, std::vector<mvm::Pixel> &pixels, std::vector<ofColor> colorsProtected,
                                  float weight);
    // bounds checking
    inline bool fixBounds(ofImage &image, int &x, int &y);
    inline bool fixBounds(ofImage &image, uint32_t &x, uint32_t &y);
    inline bool fixBounds(ofImage &image, float &x, float &y);
    
    // squares
    void drawSquare(ofImage &image, float centerX, float centerY, float width, ofColor &color);
    
    // algorithms
    int turtle_square(int x, int y, int stepsize, ofImage &image, int maxiterations, ofColor color, int threshold,
                      double fibfraction);
   
    std::vector<unsigned char> random_walker(uint64_t n);
    std::vector<unsigned char> fib(int numbers, double fraction);
    
    int turtle(int x, int y, int stepsize, ofImage &image, int maxiterations, ofColor color, int threshold, double fibfraction,
               int drawingMode);
    ofImage walker(ofImage &image, double fibFraction, bool regenerate = false);
    bool isInWaveForm(std::vector<mvm::Pixel> &waveForm, uint32_t x, uint32_t y);
    int interpolate(mvm::Config config, std::string file1, std::string file2, const std::string fileTarget, size_t frames, size_t startFrame,
                    ofColor colorBackground, bool rotate, bool shuffleStartingPositions = false);
    void interpolate(ofImage &image1, ofImage &image2, std::vector<ofImage> &imageTarget, size_t frames, ofColor colorBackground, bool shuffleStartingPositions);
    
    void test_squares();
    int example_walker2();
    
    std::vector<Pixel> undoBuffer;
    std::vector<ofColor> heatmapColors;
};

/*
 int main() {
 float fc1 = 0.01f;      // Low-pass filter cutoff frequency
 float fl = 0.1f;        // Mid-pass filter lower cutoff frequency
 float fh = 0.5f;        // Mid-pass filter higher cutoff frequency
 float fc2 = 0.5f;       // High-pass filter cutoff frequency
 
 mvm::Config config;
 config.path="/Users/ulli/Documents/mvm/";
 std::string audioFileName ="super8.mp3";
 std::string videoFramePrefix = "super8_";
 
 mvm::Dsp dsp( config,25);
 if (dsp.loadAudio(audioFileName)) {
 dsp.generateDefaultBands(fc1, fl, fh, fc2);
 }
 
 size_t samplesPerFrame = dsp.sampleRate / dsp.framesPerSecond;
 uint32_t framesToGenerate = dsp.bands[0].size() / samplesPerFrame;
 
 size_t resolution = 33;
 
 // create a gradient
 mvm::ColorGradient heatmap;
 // create a buffer to store the audio data for the duration of one videoFrame
 std::vector<float> audioFrame(samplesPerFrame);
 
 // generate frames based on the audio samples in the mono conversion
 fmt::println("MVM - generating {:L} video frames", framesToGenerate);
 // std::string img1 = this->effectPoly(config, dsp, 143, audioFrame, resolution, videoFramePrefix, heatmap, false);
 // return 0;
 
 std::chrono::steady_clock::time_point timeStart = std::chrono::steady_clock::now();
 size_t bytesProcessed = 0;
 for (size_t frame=1; frame < framesToGenerate; frame++) {
 std::string img1 = this->effectPoly(config, dsp, frame, audioFrame, resolution, videoFramePrefix, heatmap, false);
 bytesProcessed+=1000*1000*4;
 std::string progress = fmt::format("::: {} / {} :::", frame, framesToGenerate);
 std::string etaStr = this->formatEta(frame, framesToGenerate, timeStart, bytesProcessed);
 fmt::println("{} {}", progress, etaStr);
 }
 
 return 0;
 }
 */
 
 } // end namespace mvm
 
 #endif /* Mvm_hpp */


