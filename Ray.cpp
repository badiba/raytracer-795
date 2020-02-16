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
