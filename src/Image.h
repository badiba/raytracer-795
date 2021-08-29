#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <cstdio>
#include <cstdlib>
#include "Eigen/Dense"

#include "defs.h"

typedef union Color
{
	struct
	{
		unsigned char red;
		unsigned char grn;
		unsigned char blu;
	};

	unsigned char channel[3];
} Color;

class Image
{
private:
	bool IsPNG(const char* imageName);
	void SavePng(const char* imageName);
	void SaveExr(const char* imageName);

public:
	Color** data;
	float* _data;
	int width;
	int height;

	Image(int width, int height);

	void setPixelValue(int col, int row, const Eigen::Vector3f& pixelColor);
	void saveImage(const char* imageName);
};

#endif
