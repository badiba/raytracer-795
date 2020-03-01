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

// Forward declarations to avoid cyclic references
class Camera;

class PointLight;

class Material;

class Shape;

class BVH;

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
	std::vector<Eigen::Vector3f> vertices;
	std::vector<Shape*> objects;

	BVH *bvh;

	Scene(const char* xmlPath);

	void renderScene(void);

private:
	Eigen::Vector3f diffuse(ReturnVal ret, Material* mat, Ray ray, PointLight* light);

	bool isDark(Eigen::Vector3f point, PointLight* light);

	Eigen::Vector3f specular(Ray ray, ReturnVal ret, Material* mat, PointLight* light);

	Eigen::Vector3f ambient(Material* mat);

	Color shading(Ray ray, ReturnVal ret, Material* mat);

	void ThreadedRendering(int widthStart, int heightStart, int widthOffset, int heightOffset, Image& image, Camera* cam);

	//Eigen::Vector3f Mirror(Ray ray, ReturnVal ret, Material* mat, int depth);
};

#endif
