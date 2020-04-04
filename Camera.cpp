#include "Camera.h"
#include <string.h>
#include <iostream>

using namespace Eigen;

Camera::Camera(int id,                      // Id of the camera
        int sampleCount,
		const char* imageName,       // Name of the output PPM file
		const Vector3f& pos,         // Camera position
		const Vector3f& gaze,        // Camera gaze direction
		const Vector3f& up,          // Camera up direction
		const ImagePlane& imgPlane,
		float focusDistance,
		float apertureSize,
		bool isDof)  // Image plane parameters
{
	this->id = id;

	int i = 1;
	while (i < 1000){
	    if (i*i >= sampleCount){
	        this->sampleCount = i;
	        break;
	    }
	    i++;
	}

	this->imgPlane = imgPlane;
	strcpy(this->imageName, imageName);

	this->gaze = gaze.normalized();
	Vector3f w = (-gaze).normalized();
	right = (up.cross(w)).normalized();
	this->up = w.cross(right);

	this->pos = pos;
	nxDivisionAvoided = (float)1.0f / imgPlane.nx;
	nyDivisionAvoided = (float)1.0f / imgPlane.ny;

	pixelWidth = (this->imgPlane.right - this->imgPlane.left) * nxDivisionAvoided;
	pixelHeight = (this->imgPlane.top - this->imgPlane.bottom) * nyDivisionAvoided;

	sampleWidth = pixelWidth / this->sampleCount;
	sampleHeight = pixelHeight / this->sampleCount;
	totalSampleCount = sampleCount;

	this->isDof = isDof;
	this->focusDistance = focusDistance;
	this->apertureSize = apertureSize;

	mt = std::mt19937 (rd());
	dist = std::uniform_real_distribution<float>(0.0, 1.0);
}

Ray Camera::getPrimaryRay(int col, int row) const
{
	float u = imgPlane.left + (imgPlane.right - imgPlane.left) * (col + 0.5f) * nxDivisionAvoided;
	float v = imgPlane.top - (imgPlane.top - imgPlane.bottom) * (row + 0.5f) * nyDivisionAvoided;
	Vector3f m = pos + gaze * imgPlane.distance;
	m += u * right;
	m += v * up;
	Ray ray(pos, (m - pos) / ((m - pos).norm()), 0);
	return ray;
}

Vector3f Camera::PixelCenterOnImagePlane(int row, int col) const {
    float u = imgPlane.left + (imgPlane.right - imgPlane.left) * (col + 0.5f) * nxDivisionAvoided;
    float v = imgPlane.top - (imgPlane.top - imgPlane.bottom) * (row + 0.5f) * nyDivisionAvoided;
    Vector3f m = pos + gaze * imgPlane.distance;
    m += u * right;
    m += v * up;

    return m;
}

Vector3f Camera::PixelLBCorner(int row, int col) const {
    float u = imgPlane.left + col * pixelWidth;
    float v = imgPlane.top - (row + 1) * pixelHeight;
    Vector3f m = pos + gaze * imgPlane.distance;
    m += u * right;
    m += v * up;

    return m;
}

Ray Camera::getSampleRay(Eigen::Vector3f &lbCorner, int sampleIndex){
    int i = sampleIndex % sampleCount;
    int j = sampleIndex / sampleCount;

    float xChi = dist(mt);
    float yChi = dist(mt);

    Vector3f m = lbCorner;
    m += (i + xChi) * sampleWidth * right;
    m += (j + yChi) * sampleHeight * up;

    Ray ray(pos, (m - pos) / ((m - pos).norm()), 0);
    if (isDof){
        return AddDepthOfField(m, ray);
    }

    float rayTime = dist(mt);
    ray.SetTime(rayTime);
    return ray;
}

int Camera::GetTotalSampleCount() {
    return totalSampleCount;
}

Ray Camera::AddDepthOfField(Vector3f &s, Ray &r){
    Vector3f q = pos;
    Vector3f p;
    Vector3f dir;
    float t_fd;

    float xChi = dist(mt) - 0.5f;
    float yChi = dist(mt) - 0.5f;

    // q is a random point on camera.
    q += apertureSize * xChi * right;
    q += apertureSize * yChi * up;

    // p is the point on the focus plane.
    dir = (s - pos).normalized();
    t_fd = focusDistance / dir.dot(gaze);
    p = r.getPoint(t_fd);

    // direction of new ray will be p - q
    return Ray(q, (p - q).normalized(), 0);
}