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





/*
 * maximal visual madness - call me after dark
 *
 ffmpeg -framerate 25 -pattern_type glob -i 'poly_*.png' -i 'shona.ogg' -c:v libx264 -c:a copy -shortest -r 30 -pix_fmt yuv420p mvm_anim__.mp4
 *
 */


// using namespace std;
namespace mvm {

struct Pixel {
    uint32_t x;
    uint32_t y;
    ofColor color;
};

std::vector<Pixel> undoBuffer;
std::vector<ofColor> heatmapColors;

// --- timing & benchmark functions ---

std::string formatBytes(size_t bytes)
{
    const float tb = 1099511627776;
    const float gb = 1073741824;
    const float mb = 1048576;
    const float kb = 1024;
    std::string converted;
    if (bytes >= tb)
        converted = fmt::format("{:.2f} TB", (float)bytes / tb);
    else if (bytes >= gb && bytes < tb)
        converted = fmt::format("{:.2f} GB", (float)bytes / gb);
    else if (bytes >= mb && bytes < gb)
        converted = fmt::format("{:.2f} MB", (float)bytes / mb);
    else if (bytes >= kb && bytes < mb)
        converted = fmt::format("{:.2f} KB", (float)bytes / kb);
    else if (bytes < kb)
        converted = fmt::format("{} Bytes", bytes);
    return converted;
}

std::string timeToFinish(size_t items_done, size_t items_total, size_t seconds_elapsed) {
    if (items_done==0) return "~";
    int input_seconds=((double)seconds_elapsed/items_done)*items_total;
    int days = input_seconds / 60 / 60 / 24;
    int hours = (input_seconds / 60 / 60) % 24;
    int minutes = (input_seconds / 60) % 60;
    int seconds = input_seconds % 60;
    std::string eta=fmt::format("{}d {}h {}m {}s", days, hours ,minutes, seconds);
    return eta;
}

std::string timeElapsed(size_t items_done, size_t seconds_elapsed) {
    if (items_done==0) return "~";
    int input_seconds=(double)seconds_elapsed;
    int days = input_seconds / 60 / 60 / 24;
    int hours = (input_seconds / 60 / 60) % 24;
    int minutes = (input_seconds / 60) % 60;
    int seconds = input_seconds % 60;
    std::string elapsed=fmt::format("{}d {}h {}m {}s", days, hours ,minutes, seconds);
    return elapsed;
}

std::string formatEta(size_t items_done, size_t items_total, const std::chrono::steady_clock::time_point time_start, size_t bytes_read) {
    std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
    int seconds=std::chrono::duration_cast<std::chrono::seconds>(time_now - time_start).count();
    std::string formated = fmt::format("RUNTIME: {} ::: ETA {} :::", timeElapsed(items_done, seconds),
                                       timeToFinish(items_done, items_total, seconds));
    if (bytes_read!=0) {
        // read times
        float bytes_read_second;
        if (seconds>0) bytes_read_second = (float)bytes_read / seconds;
        else bytes_read_second = bytes_read;
        formated+= " " + formatBytes(bytes_read) + " ::: " + formatBytes(bytes_read_second) + "/s :::";
    }
    return formated;
}

template <typename T> int sgn(T x) {
    //return (T(0) < val) - (val < T(0));
    return ((T)((x) > 0) - (T)((x) < 0));
}

size_t getImageSizeInBytes(const ofImage& image) {
    // Calculate the size based on width, height, and pixel format
    int width = image.getWidth();
    int height = image.getHeight();
    return width * height * 4;
}

// Get the index of a pixel in the image vector given its (x, y) coordinates
[[nodiscard]] inline size_t pixelIndex(ofImage image, size_t x, size_t y)  {
    size_t index = (y * image.getWidth() + x) * 4;
    if (index > mvm::getImageSizeInBytes(image))
        fmt::println("pixelIndex out range at {},{}: index {} > {}", x, y, index, getImageSizeInBytes(image));
    return index;
}

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
class Config {
public:
    std::string path;
};


bool isFileEmpty(const std::string& filename) {
    std::filesystem::path filePath(filename);
    std::error_code ec;
    std::uintmax_t fileSize = std::filesystem::file_size(filePath, ec);
    if (ec) {
        // handle the error
        // for example, throw an exception or return -1
        return true;
    }
    return (fileSize == 0);
}

bool fileExists(const std::string &filename) {
    return std::filesystem::exists(filename);
}



inline ofColor mixColors(ofColor color1, ofColor color2, float weight) {
    ofColor result;
    result.r = (unsigned char) (color1.r * (1 - weight) + color2.r * weight);
    result.g = (unsigned char) (color1.g * (1 - weight) + color2.g * weight);
    result.b = (unsigned char) (color1.b * (1 - weight) + color2.b * weight);
    result.a = (unsigned char) (color1.a * (1 - weight) + color2.a * weight);
    return result;
}

inline void blendColor(ofColor &c1, ofColor c2)
{
    uint32_t r1 = c1.r;
    uint32_t g1 = c1.g;
    uint32_t b1 = c1.b;
    uint32_t a1 = c1.a;
    
    uint32_t r2 = c2.r;
    uint32_t g2 = c2.g;
    uint32_t b2 = c2.b;
    uint32_t a2 = c2.a;
    
    r1 = (r1*(255 - a2) + r2*a2)/255; if (r1 > 255) r1 = 255;
    g1 = (g1*(255 - a2) + g2*a2)/255; if (g1 > 255) g1 = 255;
    b1 = (b1*(255 - a2) + b2*a2)/255; if (b1 > 255) b1 = 255;
    
    
    c1.r = static_cast<unsigned char>(r1);
    c1.g = static_cast<unsigned char>(g1);
    c1.b = static_cast<unsigned char>(b1);
    c1.a = static_cast<unsigned char>(a1);
}

bool barycentric(int x1, int y1, int x2, int y2, int x3, int y3, int xp, int yp, int &u1, int &u2, int &det)
{
    det = ((x1 - x3)*(y2 - y3) - (x2 - x3)*(y1 - y3));
    u1  = ((y2 - y3)*(xp - x3) + (x3 - x2)*(yp - y3));
    u2  = ((y3 - y1)*(xp - x3) + (x1 - x3)*(yp - y3));
    int u3 = det - u1 - u2;
    return (
            (sgn(u1) == sgn(det) || sgn(u1) == 0) &&
            (sgn(u2) == sgn(det) || sgn(u2) == 0) &&
            (sgn(u3) == sgn(det) || u3 == 0)
            );
}

// stolen from https://github.com/tsoding/olive.c
inline ofColor mixColors(ofColor c1, ofColor c2, ofColor c3, int u1, int u2, int det)
{
    // TODO: estimate how much overflows are an issue in integer only environment
    int64_t r1 = c1.r;
    int64_t g1 = c1.g;
    int64_t b1 = c1.b;
    int64_t a1 = c1.a;
    
    int64_t r2 = c2.r;
    int64_t g2 = c2.g;
    int64_t b2 = c2.b;
    int64_t a2 = c2.a;
    
    int64_t r3 = c3.r;
    int64_t g3 = c3.g;
    int64_t b3 = c3.b;
    int64_t a3 = c3.a;
    
    if (det != 0) {
        int u3 = det - u1 - u2;
        ofColor result;
        result.r = (r1*u1 + r2*u2 + r3*u3)/det;
        result.g = (g1*u1 + g2*u2 + g3*u3)/det;
        result.b = (b1*u1 + b2*u2 + b3*u3)/det;
        result.a = (a1*u1 + a2*u2 + a3*u3)/det;
        return result;
    }
    return { 0, 0, 0, 0};
}

inline ofColor addColors(ofColor color1, ofColor color2) {
    ofColor result;
    result.r = std::clamp(color1.r + color2.r, 0, 255);
    result.g = std::clamp(color1.g + color2.g, 0, 255);
    result.b = std::clamp(color1.b + color2.b, 0, 255);
    result.a = std::clamp(color1.a + color2.a, 0, 255);
    // fmt::print("{} {} {}\n", color1.r, color2.r, result.r);
    return result;
}

// Define function to interpolate between two RGBA colors
ofColor interpolate(ofColor c1, ofColor c2, float fraction) {
    float r = c1.r * (1 - fraction) + c2.r * fraction;
    float g = c1.g * (1 - fraction) + c2.g * fraction;
    float b = c1.b * (1 - fraction) + c2.b * fraction;
    float alpha = c1.a * (1 - fraction) + c2.a * fraction;
    
    ofColor result;
    result.r = round(r);
    result.g = round(g);
    result.b = round(b);
    result.a = round(alpha);
    return result;
}




// Define function to interpolate between two pixels
Pixel interpolate(const Pixel &p1, const Pixel &p2, float fraction) {
    float x = p1.x * (1 - fraction) + p2.x * fraction;
    float y = p1.y * (1 - fraction) + p2.y * fraction;
    Pixel result;
    result.x = round(x);
    result.y = round(y);
    result.color = interpolate(p1.color, p2.color, fraction);
    return result;
}

std::vector<ofColor> generate_heatmap_color_table(int n) {
    std::vector<ofColor> color_table;
    for (int i = 0; i < n; i++) {
        double t = (double) i / (double) (n - 1);
        ofColor color = {0, 0, 0, 255};
        color.r = round(255 * pow(1.0 - t, 2.0));
        color.g = round(255 * 2.0 * (1.0 - t) * t);
        color.b = round(255 * pow(t, 2.0));
        
        color_table.push_back(color);
    }
    return color_table;
}

class Image {
public:
    Image(uint32_t width, uint32_t height) {
        this->width = width;
        this->height = height;
        data.resize(width * height * 4);
        this->heatmapColors = generate_heatmap_color_table(NUMBEROFCOLORS);
    }
    
    Image(uint32_t width, uint32_t height, ofColor colorBackground) {
        this->width = width;
        this->height = height;
        data.resize(width * height * 4);
        this->heatmapColors = generate_heatmap_color_table(NUMBEROFCOLORS);
        this->fill(colorBackground);
    }
    
    
    
    void setAlpha(unsigned char alpha) {
        for (int i = 3; i < this->data.size(); i += 4) {
            this->data.at(i) = alpha;
        }
    }
    
    void fill(ofColor color) {
        for (int i = 0; i < this->data.size(); i += 4) {
            this->data.at(i) = color.r;
            this->data.at(i + 1) = color.g;
            this->data.at(i + 2) = color.b;
            this->data.at(i + 3) = color.a;
        }
    }
    
    // Get the index of a pixel in the image vector given its (x, y) coordinates
    [[nodiscard]] inline size_t pixelIndex(size_t x, size_t y) const {
        size_t index = (y * width + x) * 4;
        if (index > data.size())
            fmt::println("pixelIndex out range at {},{}: index {} > {}", x, y, index, data.size());
        return index;
    }
    
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<unsigned char> data;
    std::vector<Pixel> undoBuffer;
    std::vector<ofColor> heatmapColors;
};

inline bool getPixel(ofImage &image, uint32_t x, uint32_t y, ofColor &color) {
    if (x >= image.getWidth() || y >= image.getHeight()) return false; // Out of bounds
    color = image.getColor(x, y);
    return true;
}


inline ofColor getPixel(ofImage &image, uint32_t index) {
    
    return image.getColor(index);
}

// Function to plot a pixel at a given x and y position with a given color
inline void setPixel(ofImage &image, uint32_t x, uint32_t y, ofColor color) {
    image.setColor(x, y, color);
}

inline void setPixel(ofImage &image, uint32_t x, uint32_t y, ofColor colorNew, float weight) {
    size_t index = (y * image.getWidth() + x);
    ofColor colorCurrent = image.getColor(index);
    ofColor colorMixed = mixColors(colorCurrent, colorNew, weight);
    image.setColor(x, y, colorMixed);
}

inline void setPixelAdd(ofImage &image, uint32_t x, uint32_t y, ofColor colorNew) {
    size_t index = 4 * (y * image.getWidth() + x);
    ofColor colorCurrent = getPixel(image, index);
    ofColor colorMixed = addColors(colorCurrent, colorNew);
    image.setColor(x, y, colorMixed);
}


void gaussianBlurRGBA(ofImage &image) {
    // Create a 3x3 Gaussian kernel
    std::vector<float> kernel = {1.f / 16.f, 2.f / 16.f, 1.f / 16.f,
        2.f / 16.f, 4.f / 16.f, 2.f / 16.f,
        1.f / 16.f, 2.f / 16.f, 1.f / 16.f};
    
    // Iterate over each pixel in the image
    for (uint32_t y = 1; y < image.getHeight() - 1; ++y) {
        for (uint32_t x = 1; x < image.getWidth() - 1; ++x) {
            // Calculate the weighted sum of neighboring pixels
            float sumR = 0.f, sumG = 0.f, sumB = 0.f, sumA = 0.f;
            for (int j = -1; j <= 1; ++j) {
                for (int i = -1; i <= 1; ++i) {
                    uint32_t index = ((y + j) * image.getWidth() + x + i) * 4;
                    float weight = kernel[(j + 1) * 3 + i + 1];
                    ofColor color = image.getColor(index);
                    sumR += weight * static_cast<float>(color.r);
                    sumG += weight * static_cast<float>(color.b);
                    sumB += weight * static_cast<float>(color.g);
                    sumA += weight * static_cast<float>(color.a);
                }
            }
            
            // Set the pixel to the average of the neighboring pixels
            ofColor color;
            color.r =static_cast<unsigned char>(std::round(sumR));
            color.g =static_cast<unsigned char>(std::round(sumG));
            color.b =static_cast<unsigned char>(std::round(sumB));
            color.a =static_cast<unsigned char>(std::round(sumA));
            image.setColor(x, y, color);
            
        };
        
    }
}
}

void replaceColor(ofImage &image, ofColor colorFind, ofColor colorReplace) {
    // Iterate over each pixel in the image
    for (uint32_t y = 0; y < image.getHeight(); ++y) {
        for (uint32_t x = 0; x < image.getWidth(); ++x) {
            if (image.getColor(x, y) == colorFind) {
                image.setColor(x, y, colorReplace);
            }
        }
    }
}


//    Color Floyd-Steinberg dither using 18 bits per pixel (6 bit per color plane)
//  https://www.codeproject.com/Articles/5259216/Dither-Ordered-and-Floyd-Steinberg-Monochrome-Colo
void makeDitherFSRgb18bpp(ofImage &image) noexcept {
    const int size = image.getWidth() * image.getHeight();
    
    int *errorB = (int *) malloc(size * sizeof(int));
    int *errorG = (int *) malloc(size * sizeof(int));
    int *errorR = (int *) malloc(size * sizeof(int));
    
    //    Clear the errors buffer.
    memset(errorB, 0, size * sizeof(int));
    memset(errorG, 0, size * sizeof(int));
    memset(errorR, 0, size * sizeof(int));
    
    //~~~~~~~~
    
    int i = 0;
    ofPixels pixels = image.getPixels();
    for (int y = 0; y < image.getHeight(); y++) {
        uint8_t *prow = &pixels[y * image.getWidth() * 4];
        
        for (int x = 0; x < image.getWidth(); x++, i++) {
            const int blue = prow[x * 4 + 0];
            const int green = prow[x * 4 + 1];
            const int red = prow[x * 4 + 2];
            
            int newValB = (int) blue + (errorB[i] >> 8);    //    PixelBlue  + error correctionB
            int newValG = (int) green + (errorG[i] >> 8);    //    PixelGreen + error correctionG
            int newValR = (int) red + (errorR[i] >> 8);    //    PixelRed   + error correctionR
            
            //    The error could produce values beyond the borders, so need to keep the color in range
            int idxR = std::clamp(newValR, 0, 255);
            int idxG = std::clamp(newValG, 0, 255);
            int idxB = std::clamp(newValB, 0, 255);
            
            int newcR = mvm::VALUES_18BPP[idxR >> 2];    //    x >> 2 is the same as x / 4
            int newcG = mvm::VALUES_18BPP[idxG >> 2];    //    x >> 2 is the same as x / 4
            int newcB = mvm::VALUES_18BPP[idxB >> 2];    //    x >> 2 is the same as x / 4
            
            prow[x * 4 + 0] = newcB;
            prow[x * 4 + 1] = newcG;
            prow[x * 4 + 2] = newcR;
            
            int cerrorB = newValB - newcB;
            int cerrorG = newValG - newcG;
            int cerrorR = newValR - newcR;
            
            int idx = i + 1;
            if (x + 1 < image.getWidth()) {
                errorR[idx] += (cerrorR * f7_16);
                errorG[idx] += (cerrorG * f7_16);
                errorB[idx] += (cerrorB * f7_16);
            }
            
            idx += image.getWidth() - 2;
            if (x - 1 > 0 && y + 1 < image.getHeight()) {
                errorR[idx] += (cerrorR * f3_16);
                errorG[idx] += (cerrorG * f3_16);
                errorB[idx] += (cerrorB * f3_16);
            }
            
            idx++;
            if (y + 1 < image.getHeight()) {
                errorR[idx] += (cerrorR * f5_16);
                errorG[idx] += (cerrorG * f5_16);
                errorB[idx] += (cerrorB * f5_16);
            }
            
            idx++;
            if (x + 1 < image.getWidth() && y + 1 < image.getHeight()) {
                errorR[idx] += (cerrorR * f1_16);
                errorG[idx] += (cerrorG * f1_16);
                errorB[idx] += (cerrorB * f1_16);
            }
        }
    }
    
    free(errorB);
    free(errorG);
    free(errorR);
}

bool hasNeighborOfColorC(ofImage &image, uint32_t x, uint32_t y, ofColor color) {
    // Check each neighboring pixel
    ofColor colorCurrent;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int neighborX = x + i;
            int neighborY = y + j;
            
            // Check if neighbor is within image bounds
            if (neighborX >= 0 && neighborX < image.getWidth() && neighborY >= 0 && neighborY < image.getHeight() ) {
                // Check if neighbor has colorC
                if (mvm::getPixel(image, neighborX, neighborY, colorCurrent)) {
                    if (colorCurrent == color) return true;
                }
            }
        }
    }
    
    // No neighboring pixel has colorC
    return false;
}

size_t edgeFill(ofImage &image, uint32_t x, uint32_t y, ofColor newColor, ofColor oldColor, ofColor edgeColor) {
    size_t pixelsFilled = 0;
    // std::cout << "ff " << x << "," << y << " " << pixelsFilled << "\n";
    // Base cases
    ofColor color;
    std::queue<std::pair<int, int>> pixels;
    pixels.push({x, y});
    
    while (!pixels.empty()) {
        std::pair<int, int> currPixel = pixels.front();
        pixels.pop();
        int i = currPixel.first;
        int j = currPixel.second;
        
        if (!mvm::getPixel(image, i, j, color)) {
            std::cout << "filled\n";
            continue;
        }
        
        if (color != oldColor) { continue; }
        
        pixelsFilled++;
        // newColor = image.heatmapColors[pixelsFilled % NUMBEROFCOLORS];
        mvm::setPixel(image, i, j, newColor);
        // std::cout << "ff2 " << i << "," << j << " " << pixelsFilled << "\n";
        if (!hasNeighborOfColorC(image, i, j, edgeColor)) {
            mvm::Pixel pixel = {(uint32_t) i, (uint32_t) j, color};
            mvm::undoBuffer.push_back(pixel);
        }
        
        // Check neighboring pixels for the old color before pushing them onto the stack
        if ((mvm::getPixel(image, i + 1, j, color)) && (color == oldColor)) pixels.push({i + 1, j});
        if ((mvm::getPixel(image, i - 1, j, color)) && (color == oldColor)) pixels.push({i - 1, j});
        
        if ((mvm::getPixel(image, i, j + 1, color)) && (color == oldColor)) pixels.push({i, j + 1});
        if ((mvm::getPixel(image, i, j - 1, color)) && (color == oldColor)) pixels.push({i, j - 1});
        
    }
    
    // std::cout << "ffe " << x << "," << y << " " << pixelsFilled << "\n";
    return pixelsFilled;
}

size_t edgeFill(ofImage &image, uint32_t x, uint32_t y, ofColor newColor, ofColor edgeColor, bool verbose) {
    mvm::undoBuffer.clear();
    mvm::undoBuffer.reserve(image.getWidth() * image.getHeight());
    if (verbose) std::cout << "Edge Fill: " << x << "," << y << "\n";
    ofColor oldColor;
    size_t pixelsFilled = 0;
    if (mvm::getPixel(image, x, y, oldColor)) {
        if ((oldColor != newColor) && (oldColor != edgeColor)) {
            pixelsFilled = mvm::edgeFill(image, x, y, newColor, oldColor, edgeColor);
            if (verbose) {
                auto edgePixels = pixelsFilled - mvm::undoBuffer.size();
                std::cout << "Reverting non neighbours at (" << x << "," << y << ") :" << mvm::undoBuffer.size()
                << "\n";
                std::cout << "Edge pixels:" << edgePixels << "\n";
            }
            for (const auto &pixel: mvm::undoBuffer) {
                mvm::setPixel(image, pixel.x, pixel.y, pixel.color);
            }
            
        } else {
            if (verbose) std::cout << "Fill skipped at (" << x << "," << y << ")\n";
        }
    }
    
    return pixelsFilled;
}


size_t
floodFill(ofImage &image, uint32_t x, uint32_t y, ofColor newColor, ofColor oldColor, const bool test, const size_t maxPixels) {
    size_t pixelsFilled = 0;
    // std::cout << "ff " << x << "," << y << " " << pixelsFilled << "\n";
    // Base cases
    ofColor color;
    std::queue<std::pair<int, int>> pixels;
    pixels.push({x, y});
    
    while (!pixels.empty()) {
        std::pair<int, int> currPixel = pixels.front();
        pixels.pop();
        int i = currPixel.first;
        int j = currPixel.second;
        
        if (!mvm::getPixel(image, i, j, color)) {
            std::cout << "filled\n";
            continue;
        }
        
        if (color != oldColor) { continue; }
        
        pixelsFilled++;
        // newColor = image.heatmapColors[pixelsFilled % NUMBEROFCOLORS];
        mvm::setPixel(image, i, j, newColor);
        // std::cout << "ff2 " << i << "," << j << " " << pixelsFilled << "\n";
        if (test) {
            mvm::Pixel pixel = {(uint32_t) i, (uint32_t) j, color};
            mvm::undoBuffer.push_back(pixel);
            if (pixelsFilled >= maxPixels) return pixelsFilled;
        }
        
        // Check neighboring pixels for the old color before pushing them onto the stack
        if ((mvm::getPixel(image, i + 1, j, color)) && (color == oldColor)) pixels.push({i + 1, j});
        if ((mvm::getPixel(image, i - 1, j, color)) && (color == oldColor)) pixels.push({i - 1, j});
        
        if ((mvm::getPixel(image, i, j + 1, color)) && (color == oldColor)) pixels.push({i, j + 1});
        if ((mvm::getPixel(image, i, j - 1, color)) && (color == oldColor)) pixels.push({i, j - 1});
        
    }
    
    // std::cout << "ffe " << x << "," << y << " " << pixelsFilled << "\n";
    return pixelsFilled;
}

size_t floodFill(ofImage &image, uint32_t x, uint32_t y, ofColor newColor, ofColor excludeColor, bool test, size_t maxPixels,
                 bool verbose) {
    mvm::undoBuffer.clear();
    mvm::undoBuffer.reserve(maxPixels);
    if (verbose) std::cout << "Flood Fill: " << x << "," << y << "\n";
    ofColor oldColor;
    size_t pixelsFilled = 0;
    if (mvm::getPixel(image, x, y, oldColor)) {
        if ((oldColor != newColor) && (oldColor != excludeColor)) {
            pixelsFilled = mvm::floodFill(image, x, y, newColor, oldColor, test, maxPixels);
            if (test && (pixelsFilled >= maxPixels)) {
                // revert fill operation
                if (verbose)
                    std::cout << "Reverting fill at (" << x << "," << y << ") :" << maxPixels << " / "
                    << mvm::undoBuffer.size() << "\n";
                for (const auto &pixel: mvm::undoBuffer) {
                    mvm::setPixel(image, pixel.x, pixel.y, pixel.color);
                }
            } else {
                if (verbose) std::cout << "Filled at (" << x << "," << y << ") :" << pixelsFilled << "\n";
            }
            
        } else {
            if (verbose) std::cout << "Fill skipped at (" << x << "," << y << ")\n";
        }
    }
    
    return pixelsFilled;
}


inline void drawLineGradient(ofImage &image, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, mvm::ColorGradient &gradient, float gradientEnd, unsigned char alpha = 255) {
    
    int dx = std::abs((int)(x2 - x1));
    int dy = std::abs((int)(y2 - y1));
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    int xs = x1;
    int ys = y1;
    float distance = std::sqrt(dx*dx + dy*dy);
    gradientEnd = std::clamp(gradientEnd, 0.0f, 1.0f);
    while (true) {
        // get distance to current pixel
        int dcx = std::abs((int)(xs - x1));
        int dcy = std::abs((int)(ys - y1));
        float currentDistance = std::sqrt(dcx*dcx + dcy*dcy);
        float fraction = (currentDistance / distance) * gradientEnd;
        float r,g,b;
        gradient.getColorAtValue(fraction, r, g, b);
        ofColor color = { (255 * r), (255 * g), (255 * b), static_cast<float>(alpha)};
        mvm::setPixel(image, x1, y1, color);
        
        if (x1 == x2 && y1 == y2) {
            break;
        }
        
        int e2 = 2 * err;
        
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

inline void drawLine(ofImage &image, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, ofColor color, uint32_t threshold_manhattan_length = 0) {
    
    int dx = std::abs((int)(x2 - x1));
    int dy = std::abs((int)(y2 - y1));
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    int manhattanDistance = (dx + dy) / 2;
    
    if (threshold_manhattan_length) {
        if (manhattanDistance > threshold_manhattan_length) return;
    }
    
    
    while (true) {
        image.setColor(x1, y1, color);
        
        if (x1 == x2 && y1 == y2) {
            break;
        }
        
        int e2 = 2 * err;
        
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void drawLinesAroundCenter(ofImage &image, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, ofColor color,
                           uint32_t threshold_manhattan_length = 0) {
    
    if ((x1>=image.getWidth()) || (y1>=image.getHeight()) || (x2>=image.getWidth()) || (y2>=image.getHeight()) ) return;
    int center_x = image.getWidth() / 2;
    int center_y = image.getHeight() / 2;
    
    int x1m, x2m, y1m, y2m;
    int x1_offset = x1 - center_x;
    int y1_offset = y1 - center_y;
    int x2_offset = x2 - center_x;
    int y2_offset = y2 - center_y;
    if (x1_offset < 0) x1m = center_x + abs(x1_offset); else x1m = center_x - x1_offset;
    if (y1_offset < 0) y1m = center_y + abs(y1_offset); else y1m = center_y - y1_offset;
    if (x2_offset < 0) x2m = center_x + abs(x2_offset); else x2m = center_x - x2_offset;
    if (y2_offset < 0) y2m = center_y + abs(y2_offset); else y2m = center_y - y2_offset;
    
    if ((x1m>=image.getWidth()) || (y1m>=image.getHeight()) || (x2m>=image.getWidth()) || (y2m>=image.getHeight()) ) return;
    drawLine(image, x1, y1, x2, y2, color, threshold_manhattan_length);
    drawLine(image, x1, y1m, x2, y2m, color, threshold_manhattan_length);
    drawLine(image, x1m, y1, x2m, y2, color, threshold_manhattan_length);
    drawLine(image, x1m, y1m, x2m, y2m, color, threshold_manhattan_length);
}
inline void normalizeTrianglePoints(int& x1, int& y1, int& x2, int& y2, int& x3, int& y3) {
    // Compute the cross product of the vectors (x2-x1,y2-y1) and (x3-x1,y3-y1)
    int crossProduct = (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1);
    
    // If the cross product is negative, swap the second and third points
    if (crossProduct < 0) {
        std::swap(x2, x3);
        std::swap(y2, y3);
    }
}

inline bool normalizeTriangle(size_t width, size_t height, int x1, int y1, int x2, int y2, int x3, int y3, int &lx, int &hx, int &ly, int &hy)
{
    normalizeTrianglePoints(x1, y1, x2, y2, x3, y3);
    lx = x1;
    hx = x1;
    if (lx > x2) lx = x2;
    if (lx > x3) lx = x3;
    if (hx < x2) hx = x2;
    if (hx < x3) hx = x3;
    if (lx < 0) lx = 0;
    if ((size_t) lx >= width) return false;;
    if (hx < 0) return false;;
    if ((size_t) hx >= width) hx = width-1;
    
    ly = y1;
    hy = y1;
    if (ly > y2) ly = y2;
    if (ly > y3) ly = y3;
    if (hy < y2) hy = y2;
    if (hy < y3) hy = y3;
    if (ly < 0) ly = 0;
    if ((size_t) ly >= height) return false;;
    if (hy < 0) return false;;
    if ((size_t) hy >= height) hy = height-1;
    
    return true;
}


void drawSolidTriangle(ofImage &image, int x1, int y1, int x2, int y2, int x3, int y3,
                       ofColor c1, ofColor c2, ofColor c3)
{
    int lx, hx, ly, hy;
    if (normalizeTriangle(image.getWidth(), image.getHeight(), x1, y1, x2, y2, x3, y3, lx, hx, ly, hy)) {
        ofColor color;
        // fmt::println("{},{},{},{},{},{} - {},{},{},{}", x1,y1,x2,y2,x3,y3,lx,ly,hx,hy);
        for (int y = ly; y <= hy; ++y) {
            bool hasEntered = false;
            for (int x = lx; x <= hx; ++x) {
                int u1, u2, det;
                if (mvm::barycentric(x1, y1, x2, y2, x3, y3, x, y, u1, u2, det)) {
                    hasEntered = true;
                    color = image.getColor(x, y);
                    mvm::blendColor(color, mvm::mixColors(c1, c2, c3, u1, u2, det));
                    image.setColor(x, y, color);
                } else if (hasEntered) break;
            }
        }
    }
}

void drawSolidTriangle(ofImage &image, int x1, int y1, int x2, int y2, int x3, int y3, ofColor &color) {
    // Check if the coordinates of the triangle are within the image bounds
    if (x1 < 0 || x1 >= image.getWidth() || y1 < 0 || y1 >= image.getHeight() || x2 < 0 || x2 >= image.getWidth() || y2 < 0 || y2 >= image.getHeight() || x3 < 0 || x3 >= image.getWidth() || y3 < 0 || y3 >= image.getHeight()) {
        return;
    }
    
    // Normalize the order of the points
    normalizeTrianglePoints(x1, y1, x2, y2, x3, y3);
    
    // Find the bounding box of the triangle
    int minX = std::min({x1, x2, x3});
    int minY = std::min({y1, y2, y3});
    int maxX = std::max({x1, x2, x3});
    int maxY = std::max({y1, y2, y3});
    
    // Iterate over each pixel in the bounding box
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            // Check if the pixel is inside the triangle
            if (((x2 - x1) * (y - y1) - (y2 - y1) * (x - x1)) >= 0 && ((x3 - x2) * (y - y2) - (y3 - y2) * (x - x2)) >= 0 && ((x1 - x3) * (y - y3) - (y1 - y3) * (x - x3)) >= 0) {
                // Set the pixel color
                image.setColor(x,y, color);
                
            }
        }
    }
}

std::vector<mvm::Pixel> displacePixels(ofImage &image, uint32_t n) {
    std::cout << "displacing pixels by radius " << n << "\n";
    std::random_device rd;  // obtain a random seed from hardware
    std::mt19937 gen(rd());  // seed the generator
    std::uniform_int_distribution<> distr(-n, n);  // define the range
    std::vector<mvm::Pixel> pdata;
    pdata.reserve(image.getWidth() * image.getHeight());
    ofColor color;
    for (uint32_t y = 0; y < image.getHeight(); y++) {
        for (uint32_t x = 0; x < image.getWidth(); x++) {
            int x_offset = distr(gen);
            int y_offset = distr(gen);
            int x_new = x + x_offset;
            int y_new = y + y_offset;
            if (x_new > 0 && x_new < image.getWidth() && y_new > 0 && y_new < image.getHeight()) {
                mvm::getPixel(image, x, y, color);
                mvm::Pixel pixel = {(uint32_t) x_new, (uint32_t) y_new, color};
                pdata.emplace_back(pixel);
            }
        }
    }
    return pdata;
}

size_t getPixelsNotHavingBackground(ofImage &image, ofColor colorBackground, std::vector<mvm::Pixel> &pixels) {
    ofColor color;
    size_t count = 0;
    for (uint32_t y = 0; y < image.getHeight(); y++) {
        for (uint32_t x = 0; x < image.getWidth(); x++) {
            ofColor color = image.getColor(x, y);
            // getPixel(image, x, y, color);
            if (color != colorBackground) {
                mvm::Pixel pixel = {x, y, color};
                pixels.emplace_back(pixel);
                count++;
            }
        }
    }
    return count;
}

void drawPixelsIfNotProtected(ofImage &image, std::vector<mvm::Pixel> &pixels, std::vector<ofColor> colorsProtected,
                              float weight) {
    for (const auto &pixel: pixels) {
        ofColor currentColor = image.getColor(pixel.x, pixel.y);
        bool pixelProtected = false;
        for (const auto &colorCheck: colorsProtected) {
            if (currentColor == colorCheck) pixelProtected = true;
        }
        
        if (!pixelProtected) mvm::setPixel(image, pixel.x, pixel.y, pixel.color, weight);
    }
}


inline bool fixBounds(ofImage &image, int &x, int &y) {
    bool valueWasFixed = false;
    if (x < 0 ) {
        x = 0;
        valueWasFixed = true;
    }
    if (x >= image.getWidth()) {
        x = image.getWidth()-1;
        valueWasFixed = true;
    }
    if (y < 0 ) {
        y = 0;
        valueWasFixed = true;
    }
    if (y >= image.getHeight()) {
        y = image.getHeight()-1;
        valueWasFixed = true;
    }
    return valueWasFixed;
}

inline bool fixBounds(ofImage &image, uint32_t &x, uint32_t &y) {
    bool valueWasFixed = false;
    if (x >= image.getWidth()) {
        x = image.getWidth()-1;
        valueWasFixed = true;
    }
    if (y >= image.getHeight()) {
        y = image.getHeight()-1;
        valueWasFixed = true;
    }
    return valueWasFixed;
}

inline bool fixBounds(ofImage &image, float &x, float &y) {
    bool valueWasFixed = false;
    if (x < 0 ) {
        x = 0;
        valueWasFixed = true;
    }
    if (x >= image.getWidth()) {
        x = image.getWidth()-1;
        valueWasFixed = true;
    }
    if (y < 0 ) {
        y = 0;
        valueWasFixed = true;
    }
    if (y >= image.getHeight()) {
        y = image.getHeight()-1;
        valueWasFixed = true;
    }
    return valueWasFixed;
}



// TODO: make this more accurate & clean up unused code
void drawSquare(ofImage &image, float centerX, float centerY, float width, ofColor &color) {
    // Calculate the subpixel coordinates of the four corners of the square
    float halfWidth = width / 2.0f;
    float topLeftX = std::floor(centerX - halfWidth) + 0.5f;
    float topLeftY = std::floor(centerY - halfWidth) + 0.5f;
    float topRightX = std::floor(centerX + halfWidth) + 0.5f;
    float topRightY = std::floor(centerY - halfWidth) + 0.5f;
    float bottomLeftX = std::floor(centerX - halfWidth) + 0.5f;
    float bottomLeftY = std::floor(centerY + halfWidth) + 0.5f;
    float bottomRightX = std::floor(centerX + halfWidth) + 0.5f;
    float bottomRightY = std::floor(centerY + halfWidth) + 0.5f;
    
    // Calculate the subpixel offsets of the center point
    float offsetX = centerX - std::floor(centerX) - 0.5f;
    float offsetY = centerY - std::floor(centerY) - 0.5f;
    
    // Calculate the subpixel color values for the four corners of the square
    float topLeftR = color.r * (1.0f - offsetX) * (1.0f - offsetY);
    float topLeftG = color.g * (1.0f - offsetX) * (1.0f - offsetY);
    float topLeftB = color.b * (1.0f - offsetX) * (1.0f - offsetY);
    float topLeftA = color.a * (1.0f - offsetX) * (1.0f - offsetY);
    
    float topRightR = color.r * offsetX * (1.0f - offsetY);
    float topRightG = color.g * offsetX * (1.0f - offsetY);
    float topRightB = color.b * offsetX * (1.0f - offsetY);
    float topRightA = color.a * offsetX * (1.0f - offsetY);
    
    float bottomLeftR = color.r * (1.0f - offsetX) * offsetY;
    float bottomLeftG = color.g * (1.0f - offsetX) * offsetY;
    float bottomLeftB = color.b * (1.0f - offsetX) * offsetY;
    float bottomLeftA = color.a * (1.0f - offsetX) * offsetY;
    
    float bottomRightR = color.r * offsetX * offsetY;
    float bottomRightG = color.g * offsetX * offsetY;
    float bottomRightB = color.b * offsetX * offsetY;
    float bottomRightA = color.a * offsetX * offsetY;
    
    fixBounds(image, topLeftX, topLeftY);
    fixBounds(image, topRightX, bottomLeftY);
    
    for (size_t y = topLeftY; y <= bottomLeftY; y++) {
        drawLine(image, topLeftX, y, topRightX, y, color);
    }
}

/*
 // Rotate an image represented by a vector of bytes by a given angle (in degrees)
 void rotateImage(ofImage &image, ofColor colorBackground, float degrees) {
 // Convert the angle to radians
 float radians = degrees * M_PI / 180.0f;
 float cosTheta = std::cos(radians);
 float sinTheta = std::sin(radians);
 
 // Get the dimensions of the image
 size_t width = image.getWidth();
 size_t height = image.getHeight();
 
 // Find the center of the image
 float centerX = static_cast<float>(width) / 2.0f;
 float centerY = static_cast<float>(height) / 2.0f;
 
 // Create a new image buffer to store the rotated image
 ofImage rotatedImage;
 rotatedImage.allocate(image.getWidth(), image.getHeight(), OF_IMAGE_COLOR_ALPHA);
 // fill
 for (size_t y = 0; y < height; y++) {
 for (size_t x = 0; x < width; x++) {
 rotatedImage.setColor(x, y, colorBackground);
 }
 }
 // rotatedImage.fill(colorBackground);
 // Loop over each pixel in the output image
 for (size_t y = 0; y < height; y++) {
 for (size_t x = 0; x < width; x++) {
 // Compute the coordinates of the pixel in the input image
 float newX = cosTheta * (x - centerX) + sinTheta * (y - centerY) + centerX;
 float newY = -sinTheta * (x - centerX) + cosTheta * (y - centerY) + centerY;
 
 // Get the pixel value at the new coordinates using bilinear interpolation
 if (newX >= 0 && newX < width - 1 && newY >= 0 && newY < height - 1) {
 size_t x0 = static_cast<size_t>(newX);
 size_t y0 = static_cast<size_t>(newY);
 size_t x1 = x0 + 1;
 size_t y1 = y0 + 1;
 float alpha = newX - x0;
 float beta = newY - y0;
 ofPixels data = image.getPixels();
 unsigned char *pixel00 = &data[mvm::pixelIndex(rotatedImage, x0, y0)];
 unsigned char *pixel01 = &data[mvm::pixelIndex(rotatedImage, x0, y1)];
 unsigned char *pixel10 = &data[mvm::pixelIndex(rotatedImage, x1, y0)];
 unsigned char *pixel11 = &data[mvm::pixelIndex(rotatedImage, x1, y1)];
 for (size_t c = 0; c < 4; c++) {
 float top =
 static_cast<float>(pixel00[c]) * (1 - alpha) + static_cast<float>(pixel10[c]) * alpha;
 float bottom =
 static_cast<float>(pixel01[c]) * (1 - alpha) + static_cast<float>(pixel11[c]) * alpha;
 float value = top * (1 - beta) + bottom * beta;
 auto targetPixelIndex = mvm::pixelIndex(image, x, y) + c;
 if (targetPixelIndex < getImageSizeInBytes(image))
 rotatedImage.setColor[targetPixelIndex] = static_cast<unsigned char>(value);
 }
 }
 }
 }
 
 // Copy the rotated image data back to the input vector
 image.data = std::move(rotatedImage.data);
 }
 */

std::vector<unsigned char> random_walker(uint64_t n) {
    std::random_device rd;  // obtain a random seed from hardware
    std::mt19937 gen(rd());  // seed the generator
    std::uniform_int_distribution<> distr(0, 7);  // define the range
    std::vector<unsigned char> pdata(n);
    for (size_t i = 0; i < n; i++) {
        pdata[i] = distr(gen);
        // std::cout << pdata[i]<< "\n";
    }
    std::cout << "random walker: " << pdata.size() << "\n";
    return pdata;
}


std::vector<unsigned char> fib(int numbers, double fraction) {
    std::vector<unsigned char> pdata(numbers);
    int count = 0;
    int sum = 0;
    int index = 0;
    
    for (int i = 0; i < numbers; i++) {
        sum += i * fraction;
        pdata[i] = sum % 255;
    }
    
    return pdata;
}

int turtle_square(int x, int y, int stepsize, ofImage &image, int maxiterations, ofColor color, int threshold,
                  double fibfraction) {
    auto numbers = fib(maxiterations, fibfraction);
    int xo, yo = 0;
    for (auto &number: numbers) {
        int direction = number % 4;
        // std::cout << (int) number << " " << direction << "\n";
        switch (direction) {
            case 0:
                xo = 0;
                yo = -1 * stepsize;
                break;
            case 1:
                xo = 1 * stepsize;
                yo = 0;
                break;
            case 2:
                xo = 0;
                yo = 1 * stepsize;
                break;
            case 3:
                xo = -1 * stepsize;
                yo = 0;
                break;
        }
        int x2 = abs(int((x + xo) % (int)image.getWidth()));
        int y2 = abs(int((y + yo) % (int)image.getHeight()));
        mvm::drawLine(image, x, y, x2, y2, color, threshold);
        x = x2;
        y = y2;
    }
    return 0;
}

int
turtle(int x, int y, int stepsize, ofImage &image, int maxiterations, ofColor color, int threshold, double fibfraction,
       int drawingMode) {
    auto numbers = fib(maxiterations, fibfraction);
    
    int xo, yo = 0;
    int alpha = 0;
    size_t index = 0;
    for (auto &number: numbers) {
        int direction = number % 8;
        // std::cout << "d: " << direction << "\n";
        switch (direction) {
            case 0:
                xo = 0;
                yo = -stepsize;
                break;
            case 1:
                xo = stepsize;
                yo = -stepsize;
                break;
            case 2:
                xo = stepsize;
                yo = 0;
                break;
            case 3:
                xo = stepsize;
                yo = stepsize;
                break;
            case 4:
                xo = 0;
                yo = stepsize;
                break;
            case 5:
                xo = -stepsize;
                yo = stepsize;
                break;
            case 6:
                xo = -stepsize;
                yo = 0;
                break;
            case 7:
                xo = -stepsize;
                yo = -stepsize;
                break;
        }
        
        int x2 = abs(int((x + xo) % static_cast<int>(image.getWidth() ) ));
        int y2 = abs(int((y + yo) % static_cast<int>(image.getHeight() ) ));
        // color = colors[(index % NUMBEROFCOLORS)];
        switch (drawingMode) {
            case 0:
                mvm::drawLine(image, x, y, x2, y2, color, threshold);
                break;
            case 1:
                mvm::drawLinesAroundCenter(image, x, y, x2, y2, color, threshold);
                break;
        }
        x = x2;
        y = y2;
        
        index++;
    }
    return 0;
}

std::string walker(mvm::Config config, double fibFraction, bool regenerate = false) {
    std::cout << "walker 2.0\n";
    
    const int stepSize = 10;
    const int iterations = 1000;
    const int fillThreshold = 1200;
    const bool verbose = false;
    const int drawingMode = 1;
    
    std::string filename = config.path+"walker2_";
    filename += std::to_string(fibFraction);
    filename += "_" + std::to_string(iterations);
    filename += "_" + std::to_string(fillThreshold);
    filename += "D" + std::to_string(drawingMode);
    filename += ".png";
    
    // check if the file needs to be generated
    if (mvm::fileExists(filename) && !(mvm::isFileEmpty(filename)) && !(regenerate)) {
        fmt::println("{} exists - skipping creature generation", filename);
        return filename;
    }
    ofImage image;
    image.allocate(1000, 1000, OF_IMAGE_COLOR_ALPHA);
    std::cout << "allocated " << mvm::getImageSizeInBytes(image) / (1024 * 1024) << "MB Ram for Image\n";
    std::cout << "generating RGBA image\n";
    
    ofColor color = {255, 255, 255, 255};
    ofColor excludeColor = color;
    std::cout << "turtle ....\n";
    std::cout << "width: " << image.getWidth() << "\n";
    std::cout << "height: " << image.getHeight() << "\n";
    std::cout << "fraction: " << fibFraction << "\n";
    std::cout << "step size: " << stepSize << "\n";
    
    mvm::turtle(image.getWidth() / 2, image.getHeight() / 2, stepSize, image, iterations, color, stepSize * 2.5, fibFraction,
                drawingMode);
    // turtle_square(image.width / 2, image.height / 2, stepSize, image, 5000, color, stepSize*2.5, fibFraction);
    
    std::cout << "alpha 100% for all pixels ....\n";
    // image.setAlpha(255);
    
    color = {255, 0, 0, 255};
    std::vector<ofColor> colorTable(9);
    colorTable[0] = {144, 41, 27, 255};
    colorTable[1] = {213, 78, 49, 255};
    colorTable[2] = {235, 96, 67, 255};
    colorTable[3] = {39, 89, 84, 255};
    colorTable[4] = {62, 135, 129, 255};
    colorTable[5] = {90, 191, 180, 255};
    colorTable[6] = {243, 63, 51, 255};
    colorTable[7] = {178, 163, 40, 255};
    colorTable[8] = {236, 218, 66, 255};
    std::cout << "filling magic ....\n";
    
    std::random_device rd;  // obtain a random seed from hardware
    std::mt19937 gen(0);  // seed the generator
    std::uniform_int_distribution<> distr(0, colorTable.size() - 1);  // define the range
    
    int verticalFillIndex = 0;
    for (int y = 0; y < image.getHeight(); y += stepSize) {
        verticalFillIndex++;
        color = colorTable[verticalFillIndex % colorTable.size()];
        for (int x = 0; x < image.getWidth(); x += stepSize) {
            floodFill(image, x + 2, y + 2, color, excludeColor, true, 1200, verbose);
        }
    }
    std::cout << "edge filling magic ....\n";
    
    ofColor edgeColor = {255, 255, 255, 255};
    edgeFill(image, 0, 0, color, edgeColor, verbose);
    color = {255, 255, 0, 255};
    edgeColor = {255, 0, 0, 255};
    edgeFill(image, 0, 0, color, edgeColor, verbose);
    color = {0, 0, 0, 254};
    edgeColor = {255, 255, 0, 255};
    edgeFill(image, 0, 0, color, edgeColor, verbose);
    edgeColor = {0, 0, 0, 254};
    color = {255, 255, 0, 255};
    edgeFill(image, 0, 0, color, edgeColor, verbose);
    
    
    // ----- pixel displacement -------------------------------------
    std::vector<ofColor> colorsProtected;
    colorsProtected.push_back(colorTable[0]);
    colorsProtected.push_back(colorTable[1]);
    colorsProtected.push_back(colorTable[2]);
    colorsProtected.push_back(colorTable[3]);
    colorsProtected.push_back(colorTable[4]);
    colorsProtected.push_back(colorTable[5]);
    colorsProtected.push_back(colorTable[6]);
    colorsProtected.push_back(colorTable[7]);
    colorsProtected.push_back(colorTable[8]);
    colorsProtected.push_back({255, 255, 255, 255});
    colorsProtected.push_back({255, 255, 0, 255});
    
    auto pixels = displacePixels(image, 5);
    mvm::drawPixelsIfNotProtected(image, pixels, colorsProtected, 0.8f);
    
    pixels = displacePixels(image, 20);
    mvm::drawPixelsIfNotProtected(image, pixels, colorsProtected, 0.6f);
    
    pixels = displacePixels(image, 40);
    mvm::drawPixelsIfNotProtected(image, pixels, colorsProtected, 0.4f);
    
    // replace yellow with black
    ofColor colorFind = {255, 255, 0, 255};
    ofColor colorReplace = {0, 0, 0, 255};
mvm:replaceColor(image, colorFind, colorReplace);
    
    
    // std::cout << "gaussian blur 3x3 ....\n";
    // gaussianBlurRGBA(image);
    
    color = {255, 255, 255, 255};
    turtle(image.getWidth() / 2, image.getHeight() / 2, stepSize, image, iterations, color, stepSize * 2.5, fibFraction,
           drawingMode);
    
    std::cout << "writing file\n";
    
    // Write the image to a file in PNG format
    if (1==1) {
        std::cout << "Das Bild " << filename << " wurde erfolgreich gespeichert." << std::endl;
    } else {
        std::cout << "Fehler beim Speichern des Bildes: " << filename << std::endl;
    }
    return filename;
}



bool isInWaveForm(std::vector<mvm::Pixel> &waveForm, uint32_t x, uint32_t y) {
    for (auto const &pixel: waveForm) {
        // early exit if the waveform.x > x (since the waveform is sorted in x ascending order)
        if (pixel.x > x) return false;
        if ( (pixel.x == x) && (pixel.y == y)) {
            return true;
        }
    }
    return false;
}

int
interpolate(mvm::Config config, std::string file1, std::string file2, const std::string fileTarget, size_t frames, size_t startFrame,
            ofColor colorBackground, bool rotate, bool shuffleStartingPositions = false) {
    
    // skipp generation if files exist
    for (size_t s = 0; s < frames; s++) {
        std::string fileOut = config.path+fileTarget;
        size_t frameNumber = startFrame + s;
        std::string frameNumberString = fmt::format("{:0{}d}", frameNumber, 9);
        fileOut += frameNumberString + ".png";
        if (mvm::fileExists(fileOut) && !(mvm::isFileEmpty(fileOut))) {
            fmt::println("block for {} detected", fileOut);
            return 0;
        }
    }
    
    try {
        ofImage image1(file1);
        ofImage image2(file2);
        // get a list of all pixels not having background color
        std::vector<std::vector<mvm::Pixel>> pixels(2);
        fmt::println("Image 1: {} pixels", mvm::getPixelsNotHavingBackground(image1, colorBackground, pixels[0]));
        fmt::println("Image 2: {} pixels", mvm::getPixelsNotHavingBackground(image2, colorBackground, pixels[1]));
        // now we need to make sure the list have the same number of elements
        size_t index_to_resize = 0;
        size_t resizeToElements = pixels[1].size();
        if (pixels[0].size() > pixels[1].size()) {
            index_to_resize = 1;
            resizeToElements = pixels[0].size();
        }
        
        std::random_device rd;  // obtain a random seed from hardware
        std::mt19937 gen(rd());  // seed the generator
        std::uniform_int_distribution<> distr(0, pixels[index_to_resize].size() - 1);  // define the range
        
        
        fmt::println("Adding {} random pixels for interpolation",
                     std::abs((int) (pixels[0].size() - pixels[1].size())));
        while (pixels[index_to_resize].size() < resizeToElements) {
            size_t copyIndex = distr(gen);
            pixels[index_to_resize].emplace_back(pixels[index_to_resize][copyIndex]);
        }
        
        // random shuffle of starting positions
        if (shuffleStartingPositions) {
            std::shuffle(std::begin(pixels[0]), std::end(pixels[0]), gen);
        }
        
        float stepSize = 1.0f / frames;
        float rotateStepSize = 360.0f / frames;
        std::vector<float> scaledSignal (image1.getWidth());
        std::vector<mvm::Pixel> waveFormPixels (image1.getWidth());
        for (uint32_t s = 0; s < frames; s++) {
            std::string fileOut = config.path + fileTarget;
            float fraction = 0.0f + (stepSize * s);
            float degrees = 0.0f + (rotateStepSize * s);
            size_t frameNumber = startFrame + s;
            std::string frameNumberString = fmt::format("{:0{}d}", frameNumber, 9);
            fileOut += frameNumberString + ".png";
            fmt::println("generating {}", fileOut);
            // Interpolate all pixel colors and pixel positions into a new pixels vector / image
            ofImage imageTarget;
            imageTarget.allocate(image1.getWidth(), image1.getHeight(), OF_IMAGE_COLOR_ALPHA);
            
            // acquire signal and set it to waveFormPixels for later processing
            size_t samplesPerFrame = dsp.sampleRate / dsp.framesPerSecond;
            int amplitude = 400;
            size_t offset=(frameNumber-1)*samplesPerFrame;
            
            dsp.resample(dsp.bands[0], scaledSignal, offset, samplesPerFrame, image1.width);
            for (size_t d=0; d < image1.width; d++) {
                uint32_t y = image1.height/2 + (scaledSignal[d] * amplitude);
                // setPixel(imageTarget, d, y, {255,255,0,255});
                waveFormPixels[d] = {(uint32_t )d,(uint32_t)y,{255,255,0,255} };
            }
            
            // set all pixels of the image to an alpha of 255 (since it is transparent by default)
            imageTarget.setAlpha(255);
            // check for edge cases where we don't need to interpolate
            bool sourceIsTarget = false;
            if (s == 0) {
                // first frame copy content
                imageTarget = image1;
                sourceIsTarget = true;
            } else if (s == frames - 1) {
                // last frame copy content
                imageTarget = image2;
                sourceIsTarget = true;
            } else {
                // interpolation required
                for (size_t i = 0; i < pixels[0].size(); i++) {
                    Pixel pixel = interpolate(pixels[0][i], pixels[1][i], fraction);
                    setPixelAdd(imageTarget, pixel.x, pixel.y, pixel.color);
                }
                std::vector<Pixel> interpolatedPixels;
                // store Pixels
                getPixelsNotHavingBackground(imageTarget, colorBackground, interpolatedPixels);
                // add motion blur
                gaussianBlurRGBA(imageTarget);
                // redraw interpolated Pixels
                for (size_t i = 0; i < interpolatedPixels.size(); i++) {
                    setPixel(imageTarget, interpolatedPixels[i].x, interpolatedPixels[i].y,
                             interpolatedPixels[i].color);
                    // additionally scale pixels that are also set in scaled waveForm data
                    if (isInWaveForm(waveFormPixels, interpolatedPixels[i].x, interpolatedPixels[i].y)) {
                        float scalingFactor = scaledSignal[interpolatedPixels[i].x] * 7;
                        drawSquare(imageTarget, interpolatedPixels[i].x, interpolatedPixels[i].y, 4, interpolatedPixels[i].color );
                    }
                }
            }
            if (rotate && !sourceIsTarget) {
                rotateImage(imageTarget, colorBackground, degrees);
            }
            // draw VU meters
            /*
             ofColor colorVUmeter = { 255, 255, 0, 255};
             int vux1 = 20;
             int vux2 = vux1 + (40 * dsp.vumeters[2][frameNumber] );
             int vuy = 20;
             drawLine(imageTarget, vux1, vuy, vux2, vuy, colorVUmeter);
             drawLine(imageTarget, vux1, vuy+1, vux2, vuy+1, colorVUmeter);
             
             vux2 = vux1 + (40 * dsp.vumeters[1][frameNumber] );
             vuy = 30;
             drawLine(imageTarget, vux1, vuy, vux2, vuy, colorVUmeter);
             drawLine(imageTarget, vux1, vuy+1, vux2, vuy+1, colorVUmeter);
             
             vux2 = vux1 + (40 * dsp.vumeters[0][frameNumber] );
             vuy = 40;
             drawLine(imageTarget, vux1, vuy, vux2, vuy, colorVUmeter);
             drawLine(imageTarget, vux1, vuy+1, vux2, vuy+1, colorVUmeter);
             fmt::println("VU: {} {} {}", dsp.vumeters[2][frameNumber], dsp.vumeters[1][frameNumber], dsp.vumeters[0][frameNumber]);
             */
            
            // imageTarget.savePNG(fileOut);
        }
        
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
}

void test_squares() {
    mvm::Image ti (500, 500);
    ti.setAlpha(255);
    ofColor color = {255, 255,255, 255};
    
    for (uint32_t y=0; y < ti.height; y+=2) {
        mvm::drawLine(ti, 0, y, ti.width, y, color);
    }
    for (uint32_t x=0; x < ti.width; x+=2) {
        mvm::drawLine(ti, x, 0, x, ti.height, color);
    }
    color = {255, 255,0, 255};
    mvm::drawSquare(ti, 50 ,10, 0.5, color);
    mvm::drawSquare(ti, 50 ,20, 1.0, color);
    mvm::drawSquare(ti, 50 ,30, 1.1, color);
    mvm::drawSquare(ti, 50 ,40, 1.5, color);
    mvm::drawSquare(ti, 50 ,50, 2.0, color);
    mvm::drawSquare(ti, 50 ,60, 3.0, color);
    mvm::drawSquare(ti, 50 ,70, 4.5, color);
    ti.savePNG("!squares.png");
}


int example_walker2() {
    
    
    mvm::Config config;
    config.path="/Users/ulli/Documents/mvm/";
    
    mvm::Dsp dsp( config,25);
    if (dsp.loadAudio("shona.ogg")) {
        dsp.generateDefaultBands(fc1, fl, fh, fc2);
        // dsp.generateVuMeters(25);
        // dsp.saveToWav("shona_b0.wav", dsp.bands[0], dsp.sampleRate, 1);
        // dsp.saveToWav("shona_b1.wav", dsp.bands[1], dsp.sampleRate, 1);
        // dsp.saveToWav("shona_b2.wav", dsp.bands[2], dsp.sampleRate, 1);
    }
    
    size_t samplesPerFrame = dsp.sampleRate / dsp.framesPerSecond;
    uint32_t framesToGenerate = dsp.bands[0].size() / samplesPerFrame;
    
    float average = dsp.absAverage(dsp.bands[0], 0, samplesPerFrame);
    
    std::string img1, img2 = mvm::walker(config,average);
    std::string imgInterpolated = "mvm_anim_";
    ofColor colorBackground = { 0, 0, 0, 255};
    uint32_t frames = 100;
    uint32_t startFrame = 1;
    
    std::random_device rd;  // obtain a random seed from hardware
    std::mt19937 gen(0);  // seed the generator so the sequence is predictable
    std::uniform_int_distribution<> distr(1, 6);  // define  the range for frames generation
    std::uniform_int_distribution<> distr_rotate(0, 1);  // define the rang for bool rotatee
    
    // generate keyframe images based on the audio sample in the lowpass band
    
    fmt::println("MVM - generating {:L} frames", framesToGenerate);
    for (uint32_t i=0; i<framesToGenerate; i++) {
        frames=distr(gen)*50;
        bool rotate=distr_rotate(gen);
        img1 = img2;
        size_t sampleIndex = std::min(samplesPerFrame*(startFrame-1)+((frames-1)*samplesPerFrame), dsp.samplesPerChannel);
        average = dsp.absAverage(dsp.bands[0], sampleIndex, samplesPerFrame);
        img2 = mvm::walker(config, average);
        fmt::println("transition for frames {} to {}", startFrame, (startFrame+frames)-1);
        mvm::interpolate(config, dsp, img1, img2, imgInterpolated, frames, startFrame, colorBackground, rotate);
        startFrame += frames;
    }
    
    return 0;
}

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
    // std::string img1 = mvm::effectPoly(config, dsp, 143, audioFrame, resolution, videoFramePrefix, heatmap, false);
    // return 0;
    
    std::chrono::steady_clock::time_point timeStart = std::chrono::steady_clock::now();
    size_t bytesProcessed = 0;
    for (size_t frame=1; frame < framesToGenerate; frame++) {
        std::string img1 = mvm::effectPoly(config, dsp, frame, audioFrame, resolution, videoFramePrefix, heatmap, false);
        bytesProcessed+=1000*1000*4;
        std::string progress = fmt::format("::: {} / {} :::", frame, framesToGenerate);
        std::string etaStr = mvm::formatEta(frame, framesToGenerate, timeStart, bytesProcessed);
        fmt::println("{} {}", progress, etaStr);
    }
    
    return 0;
}

} // end namespace mvm

#endif /* Mvm_hpp */
