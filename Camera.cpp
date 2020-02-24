#include "Camera.h"
#include <string.h>
#include <iostream>

using namespace Eigen;

Camera::Camera(int id,                      // Id of the camera
		const char* imageName,       // Name of the output PPM file
		const Vector3f& pos,         // Camera position
		const Vector3f& gaze,        // Camera gaze direction
		const Vector3f& up,          // Camera up direction
		const ImagePlane& imgPlane)  // Image plane parameters
{
	this->id = id;
	this->imgPlane = imgPlane;
	strcpy(this->imageName, imageName);

	this->gaze = gaze.normalized();
	Vector3f w = (-gaze).normalized();
	right = (up.cross(w)).normalized();
	this->up = w.cross(right);

	this->pos = pos;
	nxDivisionAvoided = (float)1.0f / imgPlane.nx;
	nyDivisionAvoided = (float)1.0f / imgPlane.ny;
}

Ray Camera::getPrimaryRay(int col, int row) const
{
	float u = imgPlane.left + (imgPlane.right - imgPlane.left) * (col + 0.5f) * nxDivisionAvoided;
	float v = imgPlane.top - (imgPlane.top - imgPlane.bottom) * (row + 0.5f) * nyDivisionAvoided;
	Vector3f m = pos + gaze * imgPlane.distance;
	m += u * right;
	m += v * up;
	Ray ray(pos, (m - pos) / ((m - pos).norm()));
	return ray;

}
