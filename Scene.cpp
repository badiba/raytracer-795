#include "Scene.h"
#include "Camera.h"
#include "BVH.h"
#include "Light.h"
#include "Material.h"
#include "Shape.h"
#include "tinyxml2.h"
#include <stdio.h>
#include "Image.h"
#include <algorithm>
#include <thread>
#include <cmath>
#include "happly.h"
#include "Parser.h"
#include "Helper.h"
#include "glm/gtx/string_cast.hpp"

using namespace Eigen;
using namespace tinyxml2;

bool Scene::isDark(Vector3f point, const ReturnVal& ret, PointLight* light)
{
	// Find direction vector from intersection to light.
	Vector3f direction = light->position - point;

	// Create a new ray. Origin is moved with epsilon towards light to avoid self intersection.
	Ray ray(point + ret.normal * shadowRayEps, direction / direction.norm());

	// Find nearest intersection of ray with all objects to see if there is a shadow.
    ReturnVal nearestRet = BVHMethods::FindIntersection(ray, objects, instances);

	if (nearestRet.full)
	{
		bool objectBlocksLight = (point - light->position).norm() > (point - nearestRet.point).norm();
		return objectBlocksLight;
	}

	return false;
}

Vector3f Scene::specular(Ray ray, ReturnVal ret, Material* mat, PointLight* light)
{
	// Compute specular color at given point with given light.
	Vector3f wo = -ray.direction;
	Vector3f wi = (light->position - ret.point) / (light->position - ret.point).norm();
	Vector3f h = (wo + wi) / (wo + wi).norm();
	float nh = ret.normal.dot(h);
	float alpha = std::max(0.0f, nh);

	Vector3f specularColor = light->computeLightContribution(ret.point).cwiseProduct(
			mat->specularRef * pow(alpha, mat->phongExp));
	return specularColor;
}

Vector3f Scene::diffuse(ReturnVal ret, Material* mat, Ray ray, PointLight* light)
{
	// Compute diffuse color at given point with given light.
	Vector3f wi = (light->position - ret.point) / (light->position - ret.point).norm();
	float nh = ret.normal.dot(wi);
	float alpha = std::max(0.0f, nh);

	Vector3f diffuseColor = (light->computeLightContribution(ret.point).cwiseProduct(mat->diffuseRef * alpha));
	return diffuseColor;
}

Vector3f Scene::ambient(Material* mat)
{
	// Create new ambient raw color (not bounded to 255).
	Vector3f ambientColor(0, 0, 0);

	// Compute ambient color with given material coefficient and return.
	ambientColor += ambientLight.cwiseProduct(mat->ambientRef);
	return ambientColor;
}

ShadingComponent Scene::MirrorReflectance(const Ray& ray, const ReturnVal& ret)
{
	// Angle computations.
	Vector3f wo = -ray.direction;
	float n_wo = ret.normal.dot(wo);
	Vector3f wr = -wo + ret.normal * 2 * n_wo;
	wr = wr / wr.norm();

	// Check intersection of new ray.
	Ray reflectedRay(ret.point + ret.normal * shadowRayEps, wr);
    ReturnVal nearestRet = BVHMethods::FindIntersection(reflectedRay, objects, instances);
	int materialIndex = nearestRet.matIndex - 1;

	return ShadingComponent{ reflectedRay, nearestRet, materials[materialIndex] };
}

DielectricComponent Scene::DielectricRefraction(const Ray& ray, const ReturnVal& ret, Material* mat)
{
	// Compute refracted ray.
	float dotProduct = ray.direction.dot(ret.normal);
	float nt = mat->refractionIndex;

	// -- Check entering or exiting.
	float snell = 0;
	Vector3f normal;
	bool isEntering = false;
	float n_t = 0;
	float n_i = 0;

	if (dotProduct < 0)
	{
		// entering.
		snell = 1.0f / nt;
		normal = ret.normal;
		n_t = mat->refractionIndex;
		n_i = 1;
		isEntering = true;
	}
	else
	{
		// exiting.
		snell = nt;
		normal = -ret.normal;
		n_t = 1;
		n_i = mat->refractionIndex;
		isEntering = false;
	}

	float cosTheta = -ray.direction.dot(normal);
	Vector3f leftPart = (ray.direction + normal * cosTheta) * snell;
	float squareRootPart = 1 - pow(snell, 2) * (1 - pow(cosTheta, 2));

	bool isTir = false;
	if (squareRootPart < 0)
	{
		isTir = true;
	}

	squareRootPart = sqrt(squareRootPart);
	Vector3f tDirection = leftPart - (normal * squareRootPart);
	tDirection = tDirection.normalized();
	Ray tRay(ret.point - intTestEps * normal, tDirection);

	// Return dielectric component.
    ReturnVal nearestRet = BVHMethods::FindIntersection(tRay, objects, instances);
	int materialIndex = nearestRet.matIndex - 1;

	// Fresnel
	float fresnel = FresnelReflectance(n_t, n_i, ray, tRay, normal);
	float beerDistance = (nearestRet.point - ret.point).norm();

	float beerRed = BeerLaw(mat->absorptionCoefficient[0], beerDistance);
	float beerGreen = BeerLaw(mat->absorptionCoefficient[1], beerDistance);
	float beerBlue = BeerLaw(mat->absorptionCoefficient[2], beerDistance);
	Vector3f beer = {beerRed, beerGreen, beerBlue};

	return DielectricComponent{ tRay, nearestRet, materials[materialIndex], fresnel, beer, isEntering, isTir };
}

float Scene::FresnelReflectance(float n_t, float n_i, const Ray& iRay, const Ray& tRay, const Vector3f& normal)
{
	float cos_t = -tRay.direction.dot(normal);
	float cos_i = -iRay.direction.dot(normal);

	float rParallel = (n_t * cos_i - n_i * cos_t) / (n_t * cos_i + n_i * cos_t);
	float rPerpendicular = (n_i * cos_i - n_t * cos_t) / (n_i * cos_i + n_t * cos_t);
	return (0.5f) * (pow(rParallel, 2) + pow(rPerpendicular, 2));
}

float Scene::BeerLaw(float sigma_t, float distance)
{
	return exp(-sigma_t * distance);
}

float Scene::ConductorFresnel(float n_t, float k_t, const Ray& ray, const Vector3f& normal)
{
	float cos_t = -ray.direction.dot(normal);
	float twoNtCost = 2 * n_t * cos_t;
	float cosSquared = pow(cos_t, 2);
	float nt_ktSquare = pow(n_t, 2) + pow(k_t, 2);

	float rs = (nt_ktSquare - twoNtCost + cosSquared) / (nt_ktSquare + twoNtCost + cosSquared);
	float rp = (nt_ktSquare * cosSquared - twoNtCost + 1) / (nt_ktSquare * cosSquared + twoNtCost + 1);

	return (0.5f) * (rs + rp);
}

Vector3f Scene::RecursiveShading(const Ray& ray, const ReturnVal& ret, Material* mat, int depth)
{
	if (!ret.full)
	{
		return Vector3f{ 0, 0, 0 };
	}

	if (mat->type == Normal || depth <= 0)
	{
		return BasicShading(ray, ret, mat);
	}
	else if (mat->type == Mirror)
	{
		ShadingComponent sc = MirrorReflectance(ray, ret);
		Vector3f reflectedColor = RecursiveShading(sc.ray, sc.ret, sc.mat, depth - 1);
		reflectedColor = mat->mirrorRef.cwiseProduct(reflectedColor);
		return BasicShading(ray, ret, mat) + reflectedColor;
	}
	else if (mat->type == Dielectric)
	{
		DielectricComponent dc = DielectricRefraction(ray, ret, mat);
		if (dc.isEntering)
		{
			Vector3f insideColor = RecursiveShading(dc.ray, dc.ret, dc.mat, depth - 1);
			insideColor = (1 - dc.fresnel) * insideColor;
			insideColor = dc.beer.cwiseProduct(insideColor);

			ShadingComponent sc = MirrorReflectance(ray, ret);
			Vector3f reflectedColor = RecursiveShading(sc.ray, sc.ret, sc.mat, depth - 1);
			reflectedColor = dc.fresnel * reflectedColor;

			insideColor = NanCheck(insideColor);
			reflectedColor = NanCheck(reflectedColor);
			return BasicShading(ray, ret, mat) + insideColor + reflectedColor;
		}
		else
		{
			if (dc.isTir)
			{
				ShadingComponent sc = MirrorReflectance(ray, ret);
				Vector3f internalReflection = RecursiveShading(sc.ray, sc.ret, sc.mat, depth - 1);
				internalReflection = dc.beer.cwiseProduct(internalReflection);

				internalReflection = NanCheck(internalReflection);
				return internalReflection;
			}
			else
			{
				Vector3f outsideColor = RecursiveShading(dc.ray, dc.ret, dc.mat, depth - 1);
				outsideColor = (1 - dc.fresnel) * outsideColor;

				ShadingComponent sc = MirrorReflectance(ray, ret);
				Vector3f reflectedColor = RecursiveShading(sc.ray, sc.ret, sc.mat, depth - 1);
				reflectedColor = dc.fresnel * reflectedColor;
				reflectedColor = dc.beer.cwiseProduct(reflectedColor);

				outsideColor = NanCheck(outsideColor);
				reflectedColor = NanCheck(reflectedColor);
				return outsideColor + reflectedColor;
			}
		}
	}
	else
	{
		float fresnel = ConductorFresnel(mat->refractionIndex, mat->absorptionIndex, ray, ret.normal);
		ShadingComponent sc = MirrorReflectance(ray, ret);
		Vector3f reflectedColor = RecursiveShading(sc.ray, sc.ret, sc.mat, depth - 1);
		reflectedColor = fresnel * reflectedColor;
		reflectedColor = mat->mirrorRef.cwiseProduct(reflectedColor);
		return BasicShading(ray, ret, mat) + reflectedColor;
	}
}

Vector3f Scene::NanCheck(Vector3f checkVector){
	if (checkVector[0] != checkVector[0] || checkVector[1] != checkVector[1] || checkVector[2] != checkVector[2])
	{
		return Vector3f{ 0, 0, 0 };
	}

	return checkVector;
}

Color Scene::Shading(const Ray& ray, const ReturnVal& ret, Material* mat)
{
	// Create a new rawColor (not bounded to 255).
	Vector3f rawColor = RecursiveShading(ray, ret, mat, maxRecursionDepth);

	// Clamp and return.
	rawColor = rawColor.cwiseMin(255);
	Color clampedColor{ (unsigned char)(rawColor(0)),
			(unsigned char)(rawColor(1)),
			(unsigned char)(rawColor(2)) };

	return clampedColor;
}

Vector3f Scene::BasicShading(const Ray& ray, const ReturnVal& ret, Material* mat)
{
	// Create a new rawColor (not bounded to 255).
	Vector3f rawColor(0, 0, 0);

	// Compute ambient color (no shadow check).
	rawColor = ambient(mat);

	// Check shadows for diffuse and specular shading (for every light source).
	for (int i = 0; i < lights.size(); i++)
	{
		if (isDark(ret.point, ret, lights[i]))
		{
			continue;
		}

		// No shadow for this light: Add diffuse and specular shading color.
		rawColor += diffuse(ret, mat, ray, lights[i]);
		rawColor += specular(ray, ret, mat, lights[i]);
	}

	return rawColor;
}

// TODO: MAKE THIS FASTER.
void
Scene::ThreadedRendering(int heightStart, int heightEnd, Image& image, Camera* cam)
{
	Ray ray;
	ReturnVal nearestRet;
	Color bgColor = Color{ (unsigned char)backgroundColor(0),
			(unsigned char)backgroundColor(1),
			(unsigned char)backgroundColor(2) };

	// For every pixel create a ray.
	for (int y = heightStart; y < heightEnd; y++)
	{
		for (int x = 0; x < image.width; x++)
		{


			// Find intersection of given ray using BVH.
			ray = cam->getPrimaryRay(x, y);
            nearestRet = BVHMethods::FindIntersection(ray, objects, instances);

			// If any intersection happened, compute shading.
			if (nearestRet.full)
			{
				image.setPixelValue(x, y, Shading(ray, nearestRet,
						materials[nearestRet.matIndex - 1]));
			}
				// Else paint with background color.
			else
			{
				image.setPixelValue(x, y, bgColor);
			}
		}
	}
}

void Scene::renderScene(void)
{
    // Compute object transformation matrices.
    Transforming::ComputeObjectTransformations(objects, instances, translations, scalings, rotations);

    // Create BVH for all objects.
    int objectSize = objects.size();
    for (int i = 0; i < objectSize; i++){
        objects[i]->bvh = new BVH(objects[i]);
    }

	// Create BVH.
	//bvh = new BVH();
	std::cout << "BVH construction complete." << std::endl;

	// Save an image for all cameras.
	for (int i = 0; i < cameras.size(); i++)
	{
		Camera* cam = cameras[i];
		int width, height;
		width = cam->imgPlane.nx;
		height = cam->imgPlane.ny;
		Image image(width, height);

		int offset_height = height / 8;
		int heightStart = 0;
		int heightEnd = offset_height;
		std::thread threadObj1(&Scene::ThreadedRendering, this, heightStart, heightEnd, std::ref(image), cam);
		heightStart += offset_height;
		heightEnd += offset_height;
		std::thread threadObj2(&Scene::ThreadedRendering, this, heightStart, heightEnd, std::ref(image), cam);
		heightStart += offset_height;
		heightEnd += offset_height;
		std::thread threadObj3(&Scene::ThreadedRendering, this, heightStart, heightEnd, std::ref(image), cam);
		heightStart += offset_height;
		heightEnd += offset_height;
		std::thread threadObj4(&Scene::ThreadedRendering, this, heightStart, heightEnd, std::ref(image), cam);
		heightStart += offset_height;
		heightEnd += offset_height;
		std::thread threadObj5(&Scene::ThreadedRendering, this, heightStart, heightEnd, std::ref(image), cam);
		heightStart += offset_height;
		heightEnd += offset_height;
		std::thread threadObj6(&Scene::ThreadedRendering, this, heightStart, heightEnd, std::ref(image), cam);
		heightStart += offset_height;
		heightEnd += offset_height;
		std::thread threadObj7(&Scene::ThreadedRendering, this, heightStart, heightEnd, std::ref(image), cam);
		heightStart += offset_height;
		heightEnd = height;
		std::thread threadObj8(&Scene::ThreadedRendering, this, heightStart, heightEnd, std::ref(image), cam);

		threadObj1.join();
		threadObj2.join();
		threadObj3.join();
		threadObj4.join();
		threadObj5.join();
		threadObj6.join();
		threadObj7.join();
		threadObj8.join();

		// Save image.
		image.saveImage(cam->imageName);
	}
}

Color Scene::MultiSample(int row, int col){

}

void Scene::PutMarkAt(int x, int y, Image& image)
{
	image.setPixelValue(x - 1, y, Color{ 255, 0, 0 });
	image.setPixelValue(x + 1, y, Color{ 255, 0, 0 });
	image.setPixelValue(x, y - 1, Color{ 255, 0, 0 });
	image.setPixelValue(x, y + 1, Color{ 255, 0, 0 });
}

// Parses XML file.
Scene::Scene(const char* xmlPath)
{
	XMLDocument xmlDoc;

    XMLError eResult = xmlDoc.LoadFile(xmlPath);

	XMLNode* pRoot = xmlDoc.FirstChild();

    std::cout << "Parsing scene attributes." << std::endl;
    Parser::ParseSceneAttributes(pRoot, maxRecursionDepth, backgroundColor, shadowRayEps, intTestEps);

    std::cout << "Parsing cameras." << std::endl;
	Parser::ParseCameras(pRoot, cameras);

    std::cout << "Parsing materials." << std::endl;
	Parser::ParseMaterials(pRoot, materials);

    std::cout << "Parsing transformations." << std::endl;
    Parser::ParseTransformations(pRoot, translations, scalings, rotations);

    std::cout << "Parsing vertices." << std::endl;
    Parser::ParseVertices(pRoot, vertices);

    std::cout << "Parsing objects." << std::endl;
	Parser::ParseObjects(pRoot, xmlPath, objects, instances, vertices);

    std::cout << "Parsing lights." << std::endl;
	Parser::ParseLights(pRoot, ambientLight, lights);

	std::cout << "Parsing complete." << std::endl;
}
