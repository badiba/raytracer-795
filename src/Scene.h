#ifndef _SCENE_H_
#define _SCENE_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <random>

#include "Ray.h"
#include "defs.h"
#include "Eigen/Dense"
#include "Image.h"
#include "Material.h"
#include "Transformation.h"

typedef struct ShadingComponent
{
	Ray ray;
	ReturnVal ret;
	Material* mat;
} ShadingComponent;

typedef struct DielectricComponent
{
	Ray ray;
	ReturnVal ret;
	Material* mat;
	float fresnel;
	Eigen::Vector3f beer;
	bool isEntering;
	bool isTir;
} DielectricComponent;

// Forward declarations to avoid cyclic references
class Camera;

class Light;

class Material;

class Texture;

class Shape;

class BVH;

class Perlin;

class Instance;

class Scene
{
public:
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_real_distribution<float> dist;

    int environmentLightIndex;
	int maxRecursionDepth;
	int backgroundTexture;
	float intTestEps;
	float shadowRayEps;
	Eigen::Vector3f backgroundColor;
	Eigen::Vector3f ambientLight;
    Perlin* perlin;

    std::vector<std::string> images;
	std::vector<Camera*> cameras;
	std::vector<Light*> lights;
	std::vector<Material*> materials;
	std::vector<Transformation*> translations;
    std::vector<Transformation*> scalings;
    std::vector<Transformation*> rotations;
    std::vector<Transformation*> composites;
	std::vector<Eigen::Vector3f> vertices;
	std::vector<Eigen::Vector2f> textureCoordinates;
	std::vector<Shape*> objects;
	std::vector<Instance*> instances;
	std::vector<Eigen::Vector3f> vertexNormals;
	std::vector<Texture*> textures;

	//BVH *bvh;

	Scene(const char* xmlPath);

	void renderScene(void);

private:
	void PutMarkAt(int x, int y, Image& image);

	Eigen::Vector3f NanCheck(Eigen::Vector3f checkVector);

	Eigen::Vector3f diffuse(ReturnVal ret, Material* mat, Ray ray, Light* light);

	bool isDark(const Ray& primeRay, Eigen::Vector3f point, const ReturnVal& ret, Light* light);

	Eigen::Vector3f specular(Ray ray, ReturnVal ret, Material* mat, Light* light);

	Eigen::Vector3f ambient(Material* mat);

	Eigen::Vector3f BasicShading(const Ray& ray, const ReturnVal& ret, Material* mat);

	void ThreadedRendering(int threadIndex, Image& image, Camera* cam);

	ShadingComponent MirrorReflectance(const Ray& ray, const ReturnVal& ret, Material* mat);

	Eigen::Vector3f RecursiveShading(const Ray& ray, const ReturnVal& ret, Material* mat, int depth);

    Eigen::Vector3f Shading(const Ray& ray, const ReturnVal& ret, Material* mat);

	DielectricComponent DielectricRefraction(const Ray& ray, const ReturnVal& ret, Material* mat);

	float FresnelReflectance(float n_t, float n_i, const Ray& iRay, const Ray& tRay, const Eigen::Vector3f& normal);

	float BeerLaw(float sigma_t, float distance);

	float ConductorFresnel(float n_t, float k_t, const Ray& ray, const Eigen::Vector3f& normal);

    Eigen::Vector3f MultiSample(int row, int col, Camera* cam);

    Eigen::Vector3f SingleSample(int row, int col, Camera* cam);

	Color RawColorToColor(Eigen::Vector3f color);

    Eigen::Vector3f GetBackgroundColor(int row, int col, Camera* cam, const Ray &ray);
};

#endif
