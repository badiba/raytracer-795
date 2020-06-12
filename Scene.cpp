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
#include "Perlin.h"
#include "glm/gtx/string_cast.hpp"

using namespace Eigen;
using namespace tinyxml2;

Vector3f Scene::ambient(Material* mat)
{
	// Create new ambient raw color (not bounded to 255).
	Vector3f ambientColor(0, 0, 0);

	// Compute ambient color with given material coefficient and return.
	ambientColor += ambientLight.cwiseProduct(mat->ambientRef);
	return ambientColor;
}

ShadingComponent Scene::MirrorReflectance(const Ray& ray, const ReturnVal& ret, Material* mat)
{
	// Angle computations.
	Vector3f wo = -ray.direction;
	float n_wo = ret.normal.dot(wo);
	Vector3f wr = -wo + ret.normal * 2 * n_wo;
	wr = wr / wr.norm();

	// Compute roughness
    if (mat->isRough){
        Vector3f uVector = GeometryHelpers::GetOrthonormalUVector(wr);
        Vector3f vVector = wr.cross(uVector);
        float uChi = dist(mt) - 0.5f;
        float vChi = dist(mt) - 0.5f;
        wr = (wr + ((uVector * uChi + vVector * vChi) * mat->roughness)).normalized();
    }

	// Check intersection of new ray.
	Ray reflectedRay(ret.point + ret.normal * shadowRayEps, wr, ray.time);
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
	Ray tRay(ret.point - shadowRayEps * normal, tDirection, ray.time);

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
		ShadingComponent sc = MirrorReflectance(ray, ret, mat);
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

			ShadingComponent sc = MirrorReflectance(ray, ret, mat);
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
				ShadingComponent sc = MirrorReflectance(ray, ret, mat);
				Vector3f internalReflection = RecursiveShading(sc.ray, sc.ret, sc.mat, depth - 1);
				internalReflection = dc.beer.cwiseProduct(internalReflection);

				internalReflection = NanCheck(internalReflection);
				return internalReflection;
			}
			else
			{
				Vector3f outsideColor = RecursiveShading(dc.ray, dc.ret, dc.mat, depth - 1);
				outsideColor = (1 - dc.fresnel) * outsideColor;

				ShadingComponent sc = MirrorReflectance(ray, ret, mat);
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
		ShadingComponent sc = MirrorReflectance(ray, ret, mat);
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

Eigen::Vector3f Scene::Shading(const Ray& ray, const ReturnVal& ret, Material* mat)
{
    if (ret.dm == ReplaceAll){
        return ret.textureColor;
    }

	// Create a new rawColor (not bounded to 255).
	Vector3f color = RecursiveShading(ray, ret, mat, maxRecursionDepth);

	// Clamp and return.
	return color;
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
	    /*
		if (isDark(ray, ret.point, ret, lights[i]))
		{
			continue;
		}

		// No shadow for this light: Add diffuse and specular shading color.
		rawColor += diffuse(ret, mat, ray, lights[i]);
		rawColor += specular(ray, ret, mat, lights[i]);*/
	    rawColor += lights[i]->BasicShading(ray, ret, mat);
	}

	return rawColor;
}

void Scene::ThreadedRendering(int threadIndex, Image& image, Camera* cam)
{
	bool isMultiSample = cam->GetTotalSampleCount() > 1;
	int threadCount = 8;
	int height = image.height;
	if (isMultiSample){
        for (int y = 0; y < height; y++)
        {
            for (int x = threadIndex; x < image.width; x += threadCount)
            {
                image.setPixelValue(x, y, MultiSample(x, y, cam));
            }
        }
	}
	else{
        for (int y = 0; y < height; y++)
        {
            for (int x = threadIndex; x < image.width; x += threadCount)
            {
                image.setPixelValue(x, y, SingleSample(x, y, cam));
            }
        }
	}
}

void Scene::renderScene(void)
{
    // Prepare perlin structure.
    perlin = new Perlin();

    // Compute object transformation matrices.
    Transforming::ComputeObjectTransformations(objects, instances, translations, scalings, rotations, composites);

    // Fill in vertex normals.
    Eigen::Vector3f emptyNormal = {0,0,0};
    int vertexSize = vertices.size();
    for (int i = 0; i < vertexSize; i++){
        vertexNormals.push_back(emptyNormal);
    }

    // Compute vertex normals for smooth objects.
    int objectSize = objects.size();
    for (int i = 0; i < objectSize; i++){
        objects[i]->ComputeSmoothNormals();
    }

    // Normalize vertex normals.
    for (int i = 0; i < vertexSize; i++){
        vertexNormals[i] = vertexNormals[i].normalized();
    }

    // Create BVH for all objects.
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

		//ThreadedRendering(0, std::ref(image), cam);

		std::thread threadObj1(&Scene::ThreadedRendering, this, 0, std::ref(image), cam);
		std::thread threadObj2(&Scene::ThreadedRendering, this, 1, std::ref(image), cam);
		std::thread threadObj3(&Scene::ThreadedRendering, this, 2, std::ref(image), cam);
		std::thread threadObj4(&Scene::ThreadedRendering, this, 3, std::ref(image), cam);
		std::thread threadObj5(&Scene::ThreadedRendering, this, 4, std::ref(image), cam);
		std::thread threadObj6(&Scene::ThreadedRendering, this, 5, std::ref(image), cam);
		std::thread threadObj7(&Scene::ThreadedRendering, this, 6, std::ref(image), cam);
		std::thread threadObj8(&Scene::ThreadedRendering, this, 7, std::ref(image), cam);

		threadObj1.join();
		threadObj2.join();
		threadObj3.join();
		threadObj4.join();
		threadObj5.join();
		threadObj6.join();
		threadObj7.join();
		threadObj8.join();

		// DON'T FORGET TO CHANGE THREAD COUNT //

		// Save image.
		image.saveImage(cam->imageName);
	}
}

Vector3f Scene::SingleSample(int row, int col, Camera* cam){
    Ray ray(0);
    Vector3f color;
    ReturnVal nearestRet;

    ray = cam->getPrimaryRay(row, col);
    nearestRet = BVHMethods::FindIntersection(ray, objects, instances);

    if (nearestRet.full)
    {
        color = Shading(ray, nearestRet, materials[nearestRet.matIndex - 1]);
    }

    else
    {
        color = GetBackgroundColor(row, col, cam, ray);
    }

    return color;
}

Vector3f Scene::MultiSample(int col, int row, Camera* cam){
    Vector3f lbCorner = cam->PixelLBCorner(row, col);
    Vector3f color = {0,0,0};
    ReturnVal nearestRet;
    Ray sampleRay(0);

    int sampleCount = cam->GetTotalSampleCount();
    for (int i = 0; i < sampleCount; i++){
        sampleRay = cam->getSampleRay(lbCorner, i);
        nearestRet = BVHMethods::FindIntersection(sampleRay, objects, instances);

        // If any intersection happened, compute shading.
        if (nearestRet.full)
        {
            color += Shading(sampleRay, nearestRet, materials[nearestRet.matIndex - 1]);
        }
            // Else paint with background color.
        else
        {
            color += GetBackgroundColor(row, col, cam, sampleRay);
        }
    }

    color = color / sampleCount;
    return color;
}

Vector3f Scene::GetBackgroundColor(int row, int col, Camera* cam, const Ray &ray){
    if (environmentLightIndex != -1){
        if (lights[environmentLightIndex]->GetType() != Environment){
            return backgroundColor;
        }
        float theta = acos(ray.direction[1]);
        float phi = atan2(ray.direction[2], ray.direction[0]);
        float textureU = (-phi + M_PI) / (2 * M_PI);
        float textureV = theta / M_PI;
        Texture* txt = ((EnvironmentLight*)lights[environmentLightIndex])->GetTexture();
        Vector3f radiance = txt->GetColorAtCoordinates(textureU, textureV);
        return radiance;
    }

    if (backgroundTexture == -1){
        return backgroundColor;
    }

    float u = ((float) col) / cam->imgPlane.nx;
    float v = ((float) row) / cam->imgPlane.ny;

    return textures[backgroundTexture]->GetColorAtCoordinates(u, v);
}

Color Scene::RawColorToColor(Eigen::Vector3f color){
    color = color.cwiseMin(255);
    Color clampedColor{ (unsigned char)(color(0)),
                        (unsigned char)(color(1)),
                        (unsigned char)(color(2)) };

    return clampedColor;
}

void Scene::PutMarkAt(int x, int y, Image& image)
{
	image.setPixelValue(x - 1, y, Vector3f{ 255, 0, 0 });
	image.setPixelValue(x + 1, y, Vector3f{ 255, 0, 0 });
	image.setPixelValue(x, y - 1, Vector3f{ 255, 0, 0 });
	image.setPixelValue(x, y + 1, Vector3f{ 255, 0, 0 });
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
    std::vector<ComponentBRDF> _brdf;
    Parser::ParseBRDF(pRoot, _brdf);
	Parser::ParseMaterials(pRoot, materials, _brdf);

	std::cout << "Parsing textures." << std::endl;
	Parser::ParseTextures(pRoot, xmlPath, textures, images);

    std::cout << "Parsing transformations." << std::endl;
    Parser::ParseTransformations(pRoot, translations, scalings, rotations, composites);

    std::cout << "Parsing vertices." << std::endl;
    Parser::ParseVertices(pRoot, vertices);

    std::cout << "Parsing texture coordinates." << std::endl;
    Parser::ParseTextureCoordinates(pRoot, textureCoordinates);

    std::cout << "Parsing objects." << std::endl;
	Parser::ParseObjects(pRoot, xmlPath, objects, instances, vertices, textureCoordinates);

    std::cout << "Parsing lights." << std::endl;
	Parser::ParseLights(pRoot, ambientLight, lights, images, environmentLightIndex);

	std::cout << "Parsing complete." << std::endl;

	backgroundTexture = -1;
	int textureSize = textures.size();
	for (int i = 0; i < textureSize; i++){
	    if (textures[i]->decalMode == ReplaceBackground){
	        backgroundTexture = i;
	    }
	}

    mt = std::mt19937 (rd());
    dist = std::uniform_real_distribution<float>(0.0, 1.0);
}
