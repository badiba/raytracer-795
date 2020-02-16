#include "Light.h"

PointLight::PointLight(const Eigen::Vector3f& position, const Eigen::Vector3f& intensity)
        : position(position), intensity(intensity)
{
}

Eigen::Vector3f PointLight::computeLightContribution(const Eigen::Vector3f& p)
{
    return intensity / ((p - position).norm() * (p - position).norm());
}
