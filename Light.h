#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "defs.h"
#include "Eigen/Dense"
#include "Ray.h"
#include "Shape.h"
#include "Helper.h"
#include "Material.h"
#include <random>

enum LightType{Point, Area, Directional, Spot, Environment};

class Light{
private:

protected:
    LightType _type;

public:
    Light();
    virtual LightType GetType() const = 0;
    virtual Eigen::Vector3f BasicShading(const Ray& primeRay, const ReturnVal& ret, Material* mat) = 0;
};

class PointLight : public Light
{
private:
    Eigen::Vector3f position;
    Eigen::Vector3f intensity;

public:
    PointLight(const Eigen::Vector3f& position, const Eigen::Vector3f& intensity);
    Eigen::Vector3f GetPosition() const;

    Eigen::Vector3f ComputeLightContribution(const Eigen::Vector3f& p) const;
    LightType GetType() const;
    bool IsShadow(const Ray& primeRay, const ReturnVal& ret) const;
    Eigen::Vector3f Diffuse(const Ray& primeRay, const ReturnVal& ret, Material* mat) const;
    Eigen::Vector3f Specular(const Ray& primeRay, const ReturnVal& ret, Material* mat) const;
    Eigen::Vector3f BasicShading(const Ray& primeRay, const ReturnVal& ret, Material* mat);
};

#endif

class DirectionalLight : public Light
{
private:
    Eigen::Vector3f _direction;
    Eigen::Vector3f _radiance;
public:
    DirectionalLight(const Eigen::Vector3f& direction, const Eigen::Vector3f& radiance);

    Eigen::Vector3f ComputeLightContribution(const Eigen::Vector3f& p) const;
    LightType GetType() const;
    bool IsShadow(const Ray& primeRay, const ReturnVal& ret) const;
    Eigen::Vector3f Diffuse(const Ray& primeRay, const ReturnVal& ret, Material* mat) const;
    Eigen::Vector3f Specular(const Ray& primeRay, const ReturnVal& ret, Material* mat) const;
    Eigen::Vector3f BasicShading(const Ray& primeRay, const ReturnVal& ret, Material* mat);
};

class SpotLight : public Light{
private:
    Eigen::Vector3f _position;
    Eigen::Vector3f _direction;
    Eigen::Vector3f _intensity;
    float _coverage;
    float _fall;

    float FindAngle(const ReturnVal& ret) const;
    float FallOf(float angle) const;

public:
    SpotLight(const Eigen::Vector3f& position, const Eigen::Vector3f& direction, const Eigen::Vector3f& intensity
    , float coverage, float fall);

    Eigen::Vector3f ComputeLightContribution(const Eigen::Vector3f& p) const;
    LightType GetType() const;
    bool IsShadow(const Ray& primeRay, const ReturnVal& ret) const;
    Eigen::Vector3f Diffuse(const Ray& primeRay, const ReturnVal& ret, Material* mat) const;
    Eigen::Vector3f Specular(const Ray& primeRay, const ReturnVal& ret, Material* mat) const;
    Eigen::Vector3f BasicShading(const Ray& primeRay, const ReturnVal& ret, Material* mat);
};

class AreaLight : public Light{
private:
    Eigen::Vector3f _position;
    Eigen::Vector3f _normal;
    Eigen::Vector3f _radiance;
    float _size;

    Eigen::Vector3f _u;
    Eigen::Vector3f _v;

    std::random_device rd;
    std::mt19937 mt;
    std::uniform_real_distribution<float> dist;

    float FindAreaFactor(const Eigen::Vector3f& p, const Eigen::Vector3f& sample) const;

public:
    AreaLight(const Eigen::Vector3f& position, const Eigen::Vector3f& normal, const Eigen::Vector3f& radiance, float size);

    Eigen::Vector3f ComputeLightContribution(const Eigen::Vector3f& p, const Eigen::Vector3f& sample) const;
    LightType GetType() const;
    bool IsShadow(const Ray& primeRay, const ReturnVal& ret, const Eigen::Vector3f& sample) const;
    Eigen::Vector3f Diffuse(const Ray& primeRay, const ReturnVal& ret, Material* mat, const Eigen::Vector3f& sample) const;
    Eigen::Vector3f Specular(const Ray& primeRay, const ReturnVal& ret, Material* mat, const Eigen::Vector3f& sample) const;
    Eigen::Vector3f BasicShading(const Ray& primeRay, const ReturnVal& ret, Material* mat);
};

class EnvironmentLight : public Light{
private:
    Texture* _image;

    std::random_device rd;
    std::mt19937 mt;
    std::uniform_real_distribution<float> dist;

public:
    EnvironmentLight(std::string imageName);
    Texture* GetTexture();

    Eigen::Vector3f ComputeLightContribution(const ReturnVal& ret, const Eigen::Vector3f& direction) const;
    LightType GetType() const;
    bool IsShadow(const Ray& primeRay, const ReturnVal& ret, const Eigen::Vector3f& direction) const;
    Eigen::Vector3f Diffuse(const Ray& primeRay, const ReturnVal& ret, Material* mat, const Eigen::Vector3f& direction) const;
    Eigen::Vector3f Specular(const Ray& primeRay, const ReturnVal& ret, Material* mat, const Eigen::Vector3f& direction) const;
    Eigen::Vector3f BasicShading(const Ray& primeRay, const ReturnVal& ret, Material* mat);
};