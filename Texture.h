#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "defs.h"
#include <iostream>
#include <string>
#include <fstream>
#include <jpeglib.h>
#include <png.h>

class Texture {
public:
    Texture(std::string& filename, DecalMode dm, Interpolation interpolation, TextureType type, int normalizer, float bumpFactor);
    Texture(DecalMode dm, Interpolation interpolation, TextureType type, NoiseConversion nc, int normalizer, float noiseScale, float bumpFactor);
    Eigen::Vector3f GetColorAtPixel(int i, int j);
    Eigen::Vector2f GetChangeAtCoordinates(float u, float v);
    Eigen::Vector3f GetColorAtCoordinates(float u, float v);

    DecalMode decalMode;
    Interpolation interpolation;
    TextureType type;
    NoiseConversion nc;
    float noiseScale;
    float bumpFactor;
    int normalizer;

private:
    // common variables
    int width, height;

    // png variables
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *rowPointers;

    // jpg variables
    int numComponents;
    unsigned char *rawImage;

    void ReadPNG(const char *filename);
    void ReadJPG(const char *filename);
    bool IsPNG(const char* filename);
    bool isPng;
};


#endif
