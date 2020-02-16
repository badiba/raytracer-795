#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Ray.h"
#include "defs.h"

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
			const char* imageName,       // Name of the output PPM file
			const Eigen::Vector3f& pos,         // Camera position
			const Eigen::Vector3f& gaze,        // Camera gaze direction
			const Eigen::Vector3f& up,          // Camera up direction
			const ImagePlane& imgPlane); // Image plane parameters

	// Computes the primary ray through pixel (row, col)
	Ray getPrimaryRay(int row, int col) const;

private:
	Eigen::Vector3f pos;         // Camera position
	Eigen::Vector3f gaze;        // Camera gaze direction
	Eigen::Vector3f up;
	Eigen::Vector3f right;
	float nxDivisionAvoided;
	float nyDivisionAvoided;
};

#endif
