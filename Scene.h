#ifndef _SCENE_H_
#define _SCENE_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

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

class PointLight;

class Material;

class Shape;

class BVH;

class Instance;

class Scene
{
public:
	int maxRecursionDepth;
	float intTestEps;
	float shadowRayEps;
	Eigen::Vector3f backgroundColor;
	Eigen::Vector3f ambientLight;

	std::vector<Camera*> cameras;
	std::vector<PointLight*> lights;
	std::vector<Material*> materials;
	std::vector<Transformation*> translations;
    std::vector<Transformation*> scalings;
    std::vector<Transformation*> rotations;
	std::vector<Eigen::Vector3f> vertices;
	std::vector<Shape*> objects;
	std::vector<Instance*> instances;

	//BVH *bvh;

	Scene(const char* xmlPath);

	void renderScene(void);

private:
	void PutMarkAt(int x, int y, Image& image);

	Eigen::Vector3f NanCheck(Eigen::Vector3f checkVector);

	Eigen::Vector3f diffuse(ReturnVal ret, Material* mat, Ray ray, PointLight* light);

	bool isDark(Eigen::Vector3f point, const ReturnVal& ret, PointLight* light);

	Eigen::Vector3f specular(Ray ray, ReturnVal ret, Material* mat, PointLight* light);

	Eigen::Vector3f ambient(Material* mat);

	Eigen::Vector3f BasicShading(const Ray& ray, const ReturnVal& ret, Material* mat);

	void ThreadedRendering(int heightStart, int heightOffset, Image& image, Camera* cam);

	ShadingComponent MirrorReflectance(const Ray& ray, const ReturnVal& ret);

	Eigen::Vector3f RecursiveShading(const Ray& ray, const ReturnVal& ret, Material* mat, int depth);

	Color Shading(const Ray& ray, const ReturnVal& ret, Material* mat);

	DielectricComponent DielectricRefraction(const Ray& ray, const ReturnVal& ret, Material* mat);

	float FresnelReflectance(float n_t, float n_i, const Ray& iRay, const Ray& tRay, const Eigen::Vector3f& normal);

	float BeerLaw(float sigma_t, float distance);

	float ConductorFresnel(float n_t, float k_t, const Ray& ray, const Eigen::Vector3f& normal);

	Color MultiSample(int row, int col);
};

#endif
