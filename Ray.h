#ifndef _RAY_H_
#define _RAY_H_

#include "defs.h"
#include "Eigen/Dense"

class Ray
{
public:
	Eigen::Vector3f origin;
	Eigen::Vector3f direction;
	float time;

	Ray(float time);
	Ray(const Eigen::Vector3f& origin, const Eigen::Vector3f& direction, float time);
	void SetTime(float time);

	Eigen::Vector3f getPoint(float t) const;
	float gett(const Eigen::Vector3f & p) const;

private:

};

#endif
