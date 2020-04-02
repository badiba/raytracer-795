#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Ray.h"
#include "defs.h"
#include <random>

typedef struct ImagePlane
{
	float left;     // "u" coordinate of the left edge
	float right;    // "u" coordinate of the right edge
	float bottom;   // "v" coordinate of the bottom edge
	float top;      // "v" coordinate of the top edge
	float distance; // distance to the camera (always positive)
	int nx;         // number of pixel columns
	int ny;         // number of pixel rows
} ImagePlane;

class Camera
{
public:
	char imageName[32];
	int id;
	ImagePlane imgPlane;     // Image plane

	Camera(int id,                      // Id of the camera
	        int sampleCount,
			const char* imageName,       // Name of the output PPM file
			const Eigen::Vector3f& pos,         // Camera position
			const Eigen::Vector3f& gaze,        // Camera gaze direction
			const Eigen::Vector3f& up,          // Camera up direction
			const ImagePlane& imgPlane,
			float focusDistance,
			float apertureSize,
			bool isDof); // Image plane parameters

	// Computes the primary ray through pixel (row, col)
	Ray getPrimaryRay(int row, int col) const;
	Eigen::Vector3f PixelCenterOnImagePlane(int row, int col) const;
	Eigen::Vector3f PixelLBCorner(int row, int col) const;
	Ray getSampleRay(Eigen::Vector3f &lbCorner, int sampleCount);
	Ray AddDepthOfField(Eigen::Vector3f &s, Ray &r);
	int GetTotalSampleCount();

private:
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_real_distribution<float> dist;

	Eigen::Vector3f pos;         // Camera position
	Eigen::Vector3f gaze;        // Camera gaze direction
	Eigen::Vector3f up;
	Eigen::Vector3f right;
	float nxDivisionAvoided;
	float nyDivisionAvoided;
	float pixelWidth;
	float pixelHeight;
	float sampleWidth;
	float sampleHeight;
	int sampleCount;
	int totalSampleCount;

	float focusDistance;
	float apertureSize;
	bool isDof;
};

#endif
