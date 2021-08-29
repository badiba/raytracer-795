#include <iostream>
#include "Ray.h"

using namespace Eigen;

Ray::Ray(float time) : time(time)
{
}

Ray::Ray(const Vector3f& origin, const Vector3f& direction, float time)
		: origin(origin), direction(direction), time(time)
{
}

Vector3f Ray::getPoint(float t) const
{
	Vector3f point = origin + direction * t;
	return point;
}

float Ray::gett(const Eigen::Vector3f &p) const {
    float tValue = (p[0] - origin[0]) / direction[0];
    if (tValue == tValue){
        return tValue;
    }

    tValue = (p[1] - origin[1]) / direction[1];
    if (tValue == tValue){
        return tValue;
    }

    tValue = (p[2] - origin[2]) / direction[2];
    if (tValue == tValue){
        return tValue;
    }
}

void Ray::SetTime(float time) {
    this->time = time;
}
