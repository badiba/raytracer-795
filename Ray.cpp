#include <iostream>
#include "Ray.h"

using namespace Eigen;

Ray::Ray()
{
}

Ray::Ray(const Vector3f& origin, const Vector3f& direction)
		: origin(origin), direction(direction)
{
}

Vector3f Ray::getPoint(float t) const
{
	Vector3f point = origin + direction * t;
	return point;
}

float Ray::gett(const Eigen::Vector3f &p) const {
    float x_distance = p[0] - origin[0];
    return x_distance / direction[0];
}
