#include "Light.h"
#include "Scene.h"
#include "Helper.h"
#include "Texture.h"

using namespace Eigen;

Light::Light(){

}

// ----------------------------------------------------- //
// -------------------- Point Light -------------------- //
// ----------------------------------------------------- //

PointLight::PointLight(const Vector3f& position, const Vector3f& intensity)
        : position(position), intensity(intensity)
{
    _type = Point;
}

Vector3f PointLight::GetPosition() const {
    return position;
}

Vector3f PointLight::ComputeLightContribution(const Vector3f& p) const
{
    float distance = (p - position).norm();
    return intensity / (distance * distance);
}

LightType PointLight::GetType() const {
    return _type;
}

bool PointLight::IsShadow(const Ray &primeRay, const ReturnVal &ret) const {
    Vector3f direction = position - ret.point;

    // Create a new ray. Origin is moved with epsilon towards light to avoid self intersection.
    Ray ray(ret.point + ret.normal * pScene->shadowRayEps, direction / direction.norm(), primeRay.time);

    // Find nearest intersection of ray with all objects to see if there is a shadow.
    ReturnVal nearestRet = BVHMethods::FindIntersection(ray, pScene->objects, pScene->instances);

    if (nearestRet.full)
    {
        bool objectBlocksLight = (ret.point - position).norm() > (ret.point - nearestRet.point).norm();
        return objectBlocksLight;
    }

    return false;
}

Eigen::Vector3f PointLight::Diffuse(const Ray& primeRay, const ReturnVal& ret, Material* mat) const{
    Vector3f wi = (position - ret.point).normalized();
    float nh = ret.normal.dot(wi);
    float alpha = std::max(0.0f, nh);

    Vector3f diffuseColor;
    if (ret.dm == ReplaceKd){
        diffuseColor = (ComputeLightContribution(ret.point).cwiseProduct((ret.textureColor / ret.textureNormalizer) * alpha));
    }
    else if (ret.dm == BlendKd){
        Vector3f blended = (mat->diffuseRef + (ret.textureColor / ret.textureNormalizer)) * 0.5f;
        diffuseColor = (ComputeLightContribution(ret.point).cwiseProduct(blended * alpha));
    }
    else{
        diffuseColor = (ComputeLightContribution(ret.point).cwiseProduct(mat->diffuseRef * alpha));
    }
    return diffuseColor;
}

Eigen::Vector3f PointLight::Specular(const Ray& primeRay, const ReturnVal& ret, Material* mat) const{
    // Compute specular color at given point with given light.
    Vector3f wo = -primeRay.direction;
    Vector3f wi = (position - ret.point).normalized();
    Vector3f h = (wo + wi) / (wo + wi).norm();
    float nh = ret.normal.dot(h);
    float alpha = std::max(0.0f, nh);

    Vector3f specularColor = ComputeLightContribution(ret.point).cwiseProduct(
            mat->specularRef * pow(alpha, mat->phongExp));
    return specularColor;
}

Eigen::Vector3f PointLight::BasicShading(const Ray& primeRay, const ReturnVal& ret, Material* mat){
    if (IsShadow(primeRay, ret)){
        return {0,0,0};
    }

    return Diffuse(primeRay, ret, mat) + Specular(primeRay, ret, mat);
}

// ----------------------------------------------------------- //
// -------------------- Directional Light -------------------- //
// ----------------------------------------------------------- //

DirectionalLight::DirectionalLight(const Vector3f& direction, const Vector3f& radiance){
    _direction = direction.normalized();
    _radiance = radiance;
    _type = Directional;
}

Vector3f DirectionalLight::ComputeLightContribution(const Vector3f &p) const {
    return _radiance;
}

LightType DirectionalLight::GetType() const {
    return _type;
}

bool DirectionalLight::IsShadow(const Ray &primeRay, const ReturnVal &ret) const {
    Ray ray(ret.point + ret.normal * pScene->shadowRayEps, -_direction, primeRay.time);

    ReturnVal nearestRet = BVHMethods::FindIntersection(ray, pScene->objects, pScene->instances);
    return nearestRet.full;
}

Eigen::Vector3f DirectionalLight::Diffuse(const Ray& primeRay, const ReturnVal& ret, Material* mat) const{
    Vector3f wi = -_direction;
    float nh = ret.normal.dot(wi);
    float alpha = std::max(0.0f, nh);

    Vector3f diffuseColor;
    if (ret.dm == ReplaceKd){
        diffuseColor = (_radiance.cwiseProduct((ret.textureColor / ret.textureNormalizer) * alpha));
    }
    else if (ret.dm == BlendKd){
        Vector3f blended = (mat->diffuseRef + (ret.textureColor / ret.textureNormalizer)) * 0.5f;
        diffuseColor = (_radiance.cwiseProduct(blended * alpha));
    }
    else{
        diffuseColor = (_radiance.cwiseProduct(mat->diffuseRef * alpha));
    }
    return diffuseColor;
}

Eigen::Vector3f DirectionalLight::Specular(const Ray& primeRay, const ReturnVal& ret, Material* mat) const{
    // Compute specular color at given point with given light.
    Vector3f wo = -primeRay.direction;
    Vector3f wi = -_direction;
    Vector3f h = (wo + wi) / (wo + wi).norm();
    float nh = ret.normal.dot(h);
    float alpha = std::max(0.0f, nh);

    Vector3f specularColor = _radiance.cwiseProduct(
            mat->specularRef * pow(alpha, mat->phongExp));
    return specularColor;
}

Eigen::Vector3f DirectionalLight::BasicShading(const Ray& primeRay, const ReturnVal& ret, Material* mat){
    if (IsShadow(primeRay, ret)){
        return {0,0,0};
    }

    return Diffuse(primeRay, ret, mat) + Specular(primeRay, ret, mat);
}

// ---------------------------------------------------- //
// -------------------- Spot Light -------------------- //
// ---------------------------------------------------- //

SpotLight::SpotLight(const Eigen::Vector3f& position, const Eigen::Vector3f& direction, const Eigen::Vector3f& intensity
, float coverage, float fall){
    _position = position;
    _direction = direction.normalized();
    _intensity = intensity;
    _coverage = (coverage * 0.5f) * (M_PI / 180.0f);
    _fall = (fall * 0.5f) * (M_PI / 180.0f);

    _type = Spot;
}

float SpotLight::FindAngle(const ReturnVal &ret) const{
    Vector3f directionToPoint = (ret.point - _position).normalized();
    return acos(directionToPoint.dot(_direction));
}

float SpotLight::FallOf(float angle) const{
    float cf = cos(_fall);
    float cc = cos(_coverage);

    return pow((cos(angle) - cc) / (cf - cc), 4);
}

Vector3f SpotLight::ComputeLightContribution(const Vector3f &p) const {
    float distance = (p - _position).norm();
    return _intensity / (distance * distance);
}

LightType SpotLight::GetType() const {
    return _type;
}

bool SpotLight::IsShadow(const Ray &primeRay, const ReturnVal &ret) const {
    Vector3f direction = _position - ret.point;

    // Create a new ray. Origin is moved with epsilon towards light to avoid self intersection.
    Ray ray(ret.point + ret.normal * pScene->shadowRayEps, direction / direction.norm(), primeRay.time);

    // Find nearest intersection of ray with all objects to see if there is a shadow.
    ReturnVal nearestRet = BVHMethods::FindIntersection(ray, pScene->objects, pScene->instances);

    if (nearestRet.full)
    {
        bool objectBlocksLight = (ret.point - _position).norm() > (ret.point - nearestRet.point).norm();
        return objectBlocksLight;
    }

    return false;
}

Eigen::Vector3f SpotLight::Diffuse(const Ray& primeRay, const ReturnVal& ret, Material* mat) const{
    Vector3f wi = (_position - ret.point).normalized();
    float nh = ret.normal.dot(wi);
    float alpha = std::max(0.0f, nh);

    Vector3f diffuseColor;
    if (ret.dm == ReplaceKd){
        diffuseColor = (ComputeLightContribution(ret.point).cwiseProduct((ret.textureColor / ret.textureNormalizer) * alpha));
    }
    else if (ret.dm == BlendKd){
        Vector3f blended = (mat->diffuseRef + (ret.textureColor / ret.textureNormalizer)) * 0.5f;
        diffuseColor = (ComputeLightContribution(ret.point).cwiseProduct(blended * alpha));
    }
    else{
        diffuseColor = (ComputeLightContribution(ret.point).cwiseProduct(mat->diffuseRef * alpha));
    }
    return diffuseColor;
}

Eigen::Vector3f SpotLight::Specular(const Ray& primeRay, const ReturnVal& ret, Material* mat) const{
    // Compute specular color at given point with given light.
    Vector3f wo = -primeRay.direction;
    Vector3f wi = (_position - ret.point).normalized();
    Vector3f h = (wo + wi) / (wo + wi).norm();
    float nh = ret.normal.dot(h);
    float alpha = std::max(0.0f, nh);

    Vector3f specularColor = ComputeLightContribution(ret.point).cwiseProduct(
            mat->specularRef * pow(alpha, mat->phongExp));
    return specularColor;
}

Eigen::Vector3f SpotLight::BasicShading(const Ray& primeRay, const ReturnVal& ret, Material* mat){
    if (IsShadow(primeRay, ret)){
        return {0,0,0};
    }

    float angle = FindAngle(ret);
    if (angle < _fall){
        return Diffuse(primeRay, ret, mat) + Specular(primeRay, ret, mat);
    }
    else if (angle < _coverage){
        return (Diffuse(primeRay, ret, mat) + Specular(primeRay, ret, mat)) * FallOf(angle);
    }
    else{
        return {0,0,0};
    }
}

// ---------------------------------------------------- //
// -------------------- Area Light -------------------- //
// ---------------------------------------------------- //

AreaLight::AreaLight(const Eigen::Vector3f& position, const Eigen::Vector3f& normal, const Eigen::Vector3f& radiance,
        float size){
    _position = position;
    _normal = normal.normalized();
    _radiance = radiance;
    _size = size;
    _type = Area;

    _u = GeometryHelpers::GetOrthonormalUVector(_normal);
    _v = _normal.cross(_u);

    mt = std::mt19937 (rd());
    dist = std::uniform_real_distribution<float>(0.0, 1.0);
}

float AreaLight::FindAreaFactor(const Eigen::Vector3f& p, const Eigen::Vector3f& sample) const {
    Vector3f directionToPoint = (p - sample).normalized();
    float cosTheta = abs(directionToPoint.dot(_normal));
    float dSquare = (p - sample).norm();
    dSquare = dSquare * dSquare;
    return (_size * _size) * (cosTheta / dSquare);
}

Vector3f AreaLight::ComputeLightContribution(const Vector3f &p, const Eigen::Vector3f& sample) const {
    return _radiance * FindAreaFactor(p, sample);
}

LightType AreaLight::GetType() const {
    return _type;
}

bool AreaLight::IsShadow(const Ray &primeRay, const ReturnVal &ret, const Eigen::Vector3f& sample) const {
    Vector3f direction = sample - ret.point;
    // Create a new ray. Origin is moved with epsilon towards light to avoid self intersection.
    Ray ray(ret.point + ret.normal * pScene->shadowRayEps, direction / direction.norm(), primeRay.time);

    // Find nearest intersection of ray with all objects to see if there is a shadow.
    ReturnVal nearestRet = BVHMethods::FindIntersection(ray, pScene->objects, pScene->instances);

    if (nearestRet.full)
    {
        bool objectBlocksLight = (ret.point - sample).norm() > (ret.point - nearestRet.point).norm();
        return objectBlocksLight;
    }

    return false;
}

Eigen::Vector3f AreaLight::Diffuse(const Ray& primeRay, const ReturnVal& ret, Material* mat, const Eigen::Vector3f& sample) const{
    Vector3f wi = (sample - ret.point).normalized();
    float nh = ret.normal.dot(wi);
    float alpha = std::max(0.0f, nh);

    Vector3f diffuseColor;
    if (ret.dm == ReplaceKd){
        diffuseColor = (ComputeLightContribution(ret.point, sample).cwiseProduct((ret.textureColor / ret.textureNormalizer) * alpha));
    }
    else if (ret.dm == BlendKd){
        Vector3f blended = (mat->diffuseRef + (ret.textureColor / ret.textureNormalizer)) * 0.5f;
        diffuseColor = (ComputeLightContribution(ret.point, sample).cwiseProduct(blended * alpha));
    }
    else{
        diffuseColor = (ComputeLightContribution(ret.point, sample).cwiseProduct(mat->diffuseRef * alpha));
    }
    return diffuseColor;
}

Eigen::Vector3f AreaLight::Specular(const Ray& primeRay, const ReturnVal& ret, Material* mat, const Eigen::Vector3f& sample) const{
    // Compute specular color at given point with given light.
    Vector3f wo = -primeRay.direction;
    Vector3f wi = (sample - ret.point).normalized();
    Vector3f h = (wo + wi) / (wo + wi).norm();
    float nh = ret.normal.dot(h);
    float alpha = std::max(0.0f, nh);

    Vector3f specularColor = ComputeLightContribution(ret.point, sample).cwiseProduct(
            mat->specularRef * pow(alpha, mat->phongExp));
    return specularColor;
}

Eigen::Vector3f AreaLight::BasicShading(const Ray& primeRay, const ReturnVal& ret, Material* mat){
    float uChi = dist(mt) - 0.5f;
    float vChi = dist(mt) - 0.5f;
    Vector3f sample = _position + (_u * _size * uChi) + (_v * _size * vChi);

    if (IsShadow(primeRay, ret, sample)){
        return {0,0,0};
    }

    return Diffuse(primeRay, ret, mat, sample) + Specular(primeRay, ret, mat, sample);
}

// ----------------------------------------------------------- //
// -------------------- Environment Light -------------------- //
// ----------------------------------------------------------- //

EnvironmentLight::EnvironmentLight(std::string imageName){
    _image = new Texture(imageName, NoDecal, Bilinear, ImageTexture, 1, 1);
    _type = Environment;

    mt = std::mt19937 (rd());
    dist = std::uniform_real_distribution<float>(0.0, 1.0);
}

Texture* EnvironmentLight::GetTexture() {
    return _image;
}

Vector3f EnvironmentLight::ComputeLightContribution(const ReturnVal& ret, const Eigen::Vector3f& direction) const {
    Vector3f u = GeometryHelpers::GetOrthonormalUVector(ret.normal);
    Vector3f w = ret.normal.cross(u);
    //float theta = acos(direction.dot(ret.normal));
    //float phi = atan2(direction.dot(w), direction.dot(u));
    float theta = acos(direction[1]);
    float phi = atan2(direction[2], direction[0]);
    float textureU = (-phi + M_PI) / (2 * M_PI);
    float textureV = theta / M_PI;
    Vector3f radiance = _image->GetColorAtCoordinates(textureU, textureV);
    //std::cout << radiance << std::endl << std::endl;
    return radiance * 2 * M_PI;
}

LightType EnvironmentLight::GetType() const {
    return _type;
}

bool EnvironmentLight::IsShadow(const Ray &primeRay, const ReturnVal &ret, const Eigen::Vector3f& direction) const {
    // Create a new ray. Origin is moved with epsilon towards light to avoid self intersection.
    Ray ray(ret.point + ret.normal * pScene->shadowRayEps, direction, primeRay.time);

    // Find nearest intersection of ray with all objects to see if there is a shadow.
    ReturnVal nearestRet = BVHMethods::FindIntersection(ray, pScene->objects, pScene->instances);

    if (nearestRet.full)
    {
        return true;
    }

    return false;
}

Eigen::Vector3f EnvironmentLight::Diffuse(const Ray& primeRay, const ReturnVal& ret, Material* mat,
        const Eigen::Vector3f& direction) const{
    float nh = ret.normal.dot(direction);
    float alpha = std::max(0.0f, nh);

    Vector3f diffuseColor;
    if (ret.dm == ReplaceKd){
        diffuseColor = (ComputeLightContribution(ret, direction).cwiseProduct((ret.textureColor / ret.textureNormalizer) * alpha));
    }
    else if (ret.dm == BlendKd){
        Vector3f blended = (mat->diffuseRef + (ret.textureColor / ret.textureNormalizer)) * 0.5f;
        diffuseColor = (ComputeLightContribution(ret, direction).cwiseProduct(blended * alpha));
    }
    else{
        diffuseColor = (ComputeLightContribution(ret, direction).cwiseProduct(mat->diffuseRef * alpha));
    }
    return diffuseColor;
}

Eigen::Vector3f EnvironmentLight::Specular(const Ray& primeRay, const ReturnVal& ret, Material* mat,
        const Eigen::Vector3f& direction) const{
    // Compute specular color at given point with given light.
    Vector3f wo = -primeRay.direction;
    Vector3f h = (wo + direction) / (wo + direction).norm();
    float nh = ret.normal.dot(h);
    float alpha = std::max(0.0f, nh);

    Vector3f specularColor = ComputeLightContribution(ret, direction).cwiseProduct(
            mat->specularRef * pow(alpha, mat->phongExp));
    return specularColor;
}

Eigen::Vector3f EnvironmentLight::BasicShading(const Ray& primeRay, const ReturnVal& ret, Material* mat){
    Vector3f direction;
    Vector3f sample;
    Vector3f u = GeometryHelpers::GetOrthonormalUVector(ret.normal);
    Vector3f w = ret.normal.cross(u);

    while(true){
        float x = dist(mt) * 2 - 1.0f;
        float y = dist(mt) * 2 - 1.0f;
        float z = dist(mt) * 2 - 1.0f;

        sample = ret.point + u * x + ret.normal * y + w * z;
        // check these random numbers.
        //direction = {x,y,z};
        direction = sample - ret.point;

        if (direction.dot(ret.normal) > 0 && direction.norm() <= 1){
            direction = direction.normalized();
            break;
        }
    }

    if (IsShadow(primeRay, ret, direction)){
        return {0, 0, 0};
    }

    return Diffuse(primeRay, ret, mat, direction) + Specular(primeRay, ret, mat, direction);
}
