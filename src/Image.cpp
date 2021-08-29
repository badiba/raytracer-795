#include "Image.h"
#include "Helper.h"

Image::Image(int width, int height)
        : width(width), height(height)
{
    //data = new Color* [height];
    _data = new float[width*height*3];
    /*
    for (int y = 0; y < height; ++y)
    {
        data[y] = new Color[width];
    }*/
}

void Image::setPixelValue(int col, int row, const Eigen::Vector3f& pixelColor)
{
    //data[row][col] = color;

    int startIndex = (row * width + col) * 3;
    _data[startIndex] = pixelColor[0];
    _data[startIndex+1] = pixelColor[1];
    _data[startIndex+2] = pixelColor[2];
}

void Image::saveImage(const char* imageName)
{
    if (IsPNG(imageName)){
        SavePng(imageName);
    }
    else{
        SaveExr(imageName);
    }
}

bool Image::IsPNG(const char *filename) {
    int i = 0;
    int c = 0;
    while (filename[i] != '\0'){
        if (filename[i] == '.'){
            c = 1;
        }
        else if (filename[i] == 'p' && c == 1){
            c = 2;
        }
        else if (filename[i] == 'n' && c == 2){
            c = 3;
        }
        else if (filename[i] == 'g' && c == 3){
            return true;
        }
        else{
            c = 0;
        }

        i++;
    }

    return false;
}

void Image::SavePng(const char *imageName){
    int size = width*height*3;
    for (int i = 0; i < size; i++){
        if (_data[i] > 255){
            _data[i] = 255;
        }
    }

    FILE* output;

    output = fopen(imageName, "w");
    fprintf(output, "P3\n");
    fprintf(output, "%d %d\n", width, height);
    fprintf(output, "255\n");

    /*for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            for (int c = 0; c < 3; ++c)
            {
                fprintf(output, "%d ", data[y][x].channel[c]);
            }
        }

        fprintf(output, "\n");
    }*/


    int rowLength = width * 3;
    for (int y = 0; y < height; y++)
    {
        int rowBegin = y * width * 3;
        for (int i = 0; i < rowLength; i++){
            fprintf(output, "%d ", (unsigned char)_data[rowBegin + i]);
        }

        fprintf(output, "\n");
    }

    fclose(output);
}

void Image::SaveExr(const char *imageName) {
    ExrLibrary::SaveExr(imageName, _data, width, height);
}