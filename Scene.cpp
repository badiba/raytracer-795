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

using namespace Eigen;
using namespace tinyxml2;

bool Scene::isDark(Vector3f point, PointLight* light)
{
	// Find direction vector from intersection to light.
	Vector3f direction = light->position - point;

	// Create a new ray. Origin is moved with epsilon towards light to avoid self intersection.
	Ray ray(point + direction * shadowRayEps, direction / direction.norm());
	ReturnVal ret;

	// Check intersection of ray with all objects to see if there is a shadow.
	for (int k = 0; k < objects.size(); k++)
	{
		// Get intersection information.
		ret = objects[k]->intersect(ray);

		// Check if intersected object is between point and light.
		bool objectBlocksLight = (point - light->position).norm() > (point - ret.point).norm();

		// If intersection happened and object is blocking the light then there is a shadow.
		if (ret.full && objectBlocksLight)
		{
			return true;
		}
	}

	// No shadow, return false.
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
	ReturnVal nearestRet = bvh->FindIntersection(reflectedRay);
	int materialIndex = nearestRet.matIndex - 1;

	return ShadingComponent{ reflectedRay, nearestRet, materials[materialIndex] };
}

ShadingComponent Scene::DielectricRefraction(const Ray& ray, const ReturnVal& ret, Material* mat)
{
	// Compute refracted ray.
	float cosTheta = -ray.direction.dot(ret.normal);
	float nt = mat->refractionIndex;
	float snell = 1.0f / nt;
	Vector3f leftPart = (ray.direction + ret.normal * cosTheta) * snell;
	float squareRootPart = 1 - pow(snell, 2) * (1 - pow(cosTheta, 2));
	squareRootPart = sqrt(squareRootPart);
	Vector3f tDirection = leftPart - (ret.normal * squareRootPart);
	Ray tRay(ret.point - ret.normal * shadowRayEps, tDirection);

	ReturnVal insideIntersection = bvh->FindIntersection(tRay);
	Ray escapedRay(insideIntersection.point + insideIntersection.normal * shadowRayEps, ray.direction);

	ReturnVal nearestRet = bvh->FindIntersection(escapedRay);
	int materialIndex = nearestRet.matIndex - 1;

	return ShadingComponent{escapedRay, nearestRet, materials[materialIndex]};
}

Vector3f Scene::RecursiveShading(const Ray& ray, const ReturnVal& ret, Material* mat, int depth)
{
	if (!ret.full)
	{
		return Vector3f{ 0, 0, 0 };
	}

	if (mat->type == Normal || depth == 1)
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
		ShadingComponent sc = DielectricRefraction(ray, ret, mat);
		Vector3f refractedColor = RecursiveShading(sc.ray, sc.ret, sc.mat, depth - 1);
		return BasicShading(ray, ret, mat) + refractedColor;
	}
	else
	{
		return Vector3f{ 255, 255, 255 };
	}
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
		if (isDark(ret.point, lights[i]))
		{
			continue;
		}

		// No shadow for this light: Add diffuse and specular shading color.
		rawColor += diffuse(ret, mat, ray, lights[i]);
		rawColor += specular(ray, ret, mat, lights[i]);
	}

	return rawColor;
}

void
Scene::ThreadedRendering(int widthStart, int heightStart, int widthOffset, int heightOffset, Image& image, Camera* cam)
{
	Ray ray;
	int widthEnd = widthStart + widthOffset;
	int heightEnd = heightStart + heightOffset;
	ReturnVal nearestRet;
	Color bgColor = Color{ (unsigned char)backgroundColor(0),
			(unsigned char)backgroundColor(1),
			(unsigned char)backgroundColor(2) };

	// For every pixel create a ray.
	for (int i = widthStart; i < widthEnd; i++)
	{
		for (int j = heightStart; j < heightEnd; j++)
		{
			// Find intersection of given ray using BVH.
			ray = cam->getPrimaryRay(i, j);
			nearestRet = bvh->FindIntersection(ray);

			// If any intersection happened, compute shading.
			if (nearestRet.full)
			{
				image.setPixelValue(i, j, Shading(ray, nearestRet,
						materials[nearestRet.matIndex - 1]));
			}
				// Else paint with background color.
			else
			{
				image.setPixelValue(i, j, bgColor);
			}
		}
	}
}

void Scene::renderScene(void)
{
	// Create BVH.
	bvh = new BVH();
	std::cout << "BVH construction complete." << std::endl;

	// Save an image for all cameras.
	for (int i = 0; i < cameras.size(); i++)
	{
		Camera* cam = cameras[i];
		int width, height;
		width = cam->imgPlane.nx;
		height = cam->imgPlane.ny;
		Image image(width, height);

		std::vector<std::thread> threads;

		// TODO: Change structure of this.
		int offset_width = width / 2;
		int offset_height = height / 2;
		std::thread threadObj1(&Scene::ThreadedRendering, this, 0, 0, offset_width, offset_height, std::ref(image),
				cam);
		std::thread threadObj2(&Scene::ThreadedRendering, this, offset_width, 0, offset_width, offset_height,
				std::ref(image), cam);
		std::thread threadObj3(&Scene::ThreadedRendering, this, 0, offset_height, offset_width, offset_height,
				std::ref(image), cam);
		std::thread threadObj4(&Scene::ThreadedRendering, this, offset_width, offset_height, offset_width,
				offset_height, std::ref(image), cam);

		threadObj1.join();
		threadObj2.join();
		threadObj3.join();
		threadObj4.join();

		// Save image.
		image.saveImage(cam->imageName);
	}
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
	const char* str;
	XMLDocument xmlDoc;
	XMLError eResult;
	XMLElement* pElement;

	maxRecursionDepth = 1;
	shadowRayEps = 0.001;
	intTestEps = 0.001;

	eResult = xmlDoc.LoadFile(xmlPath);

	XMLNode* pRoot = xmlDoc.FirstChild();

	pElement = pRoot->FirstChildElement("MaxRecursionDepth");
	if (pElement != nullptr)
	{
		pElement->QueryIntText(&maxRecursionDepth);
	}

	pElement = pRoot->FirstChildElement("BackgroundColor");
	str = pElement->GetText();
	sscanf(str, "%f %f %f", &backgroundColor(0), &backgroundColor(1), &backgroundColor(2));

	pElement = pRoot->FirstChildElement("ShadowRayEpsilon");
	if (pElement != nullptr)
	{
		pElement->QueryFloatText(&shadowRayEps);
	}

	pElement = pRoot->FirstChildElement("IntersectionTestEpsilon");
	if (pElement != nullptr)
	{
		eResult = pElement->QueryFloatText(&intTestEps);
	}

	// Parse cameras
	pElement = pRoot->FirstChildElement("Cameras");
	XMLElement* pCamera = pElement->FirstChildElement("Camera");
	XMLElement* camElement;
	while (pCamera != nullptr)
	{
		int id;
		char imageName[64];
		Vector3f pos, gaze, up;
		ImagePlane imgPlane;

		eResult = pCamera->QueryIntAttribute("id", &id);
		camElement = pCamera->FirstChildElement("Position");
		str = camElement->GetText();
		sscanf(str, "%f %f %f", &pos(0), &pos(1), &pos(2));
		camElement = pCamera->FirstChildElement("Gaze");
		str = camElement->GetText();
		sscanf(str, "%f %f %f", &gaze(0), &gaze(1), &gaze(2));
		camElement = pCamera->FirstChildElement("Up");
		str = camElement->GetText();
		sscanf(str, "%f %f %f", &up(0), &up(1), &up(2));
		camElement = pCamera->FirstChildElement("NearPlane");
		str = camElement->GetText();
		sscanf(str, "%f %f %f %f", &imgPlane.left, &imgPlane.right, &imgPlane.bottom, &imgPlane.top);
		camElement = pCamera->FirstChildElement("NearDistance");
		eResult = camElement->QueryFloatText(&imgPlane.distance);
		camElement = pCamera->FirstChildElement("ImageResolution");
		str = camElement->GetText();
		sscanf(str, "%d %d", &imgPlane.nx, &imgPlane.ny);
		camElement = pCamera->FirstChildElement("ImageName");
		str = camElement->GetText();
		strcpy(imageName, str);

		cameras.push_back(new Camera(id, imageName, pos, gaze, up, imgPlane));

		pCamera = pCamera->NextSiblingElement("Camera");
	}

	// Parse materials
	pElement = pRoot->FirstChildElement("Materials");
	XMLElement* pMaterial = pElement->FirstChildElement("Material");
	XMLElement* materialElement;
	while (pMaterial != nullptr)
	{
		materials.push_back(new Material());

		int curr = materials.size() - 1;

		eResult = pMaterial->QueryIntAttribute("id", &materials[curr]->id);
		materialElement = pMaterial->FirstChildElement("AmbientReflectance");
		str = materialElement->GetText();
		sscanf(str, "%f %f %f", &materials[curr]->ambientRef(0), &materials[curr]->ambientRef(1),
				&materials[curr]->ambientRef(2));
		materialElement = pMaterial->FirstChildElement("DiffuseReflectance");
		str = materialElement->GetText();
		sscanf(str, "%f %f %f", &materials[curr]->diffuseRef(0), &materials[curr]->diffuseRef(1),
				&materials[curr]->diffuseRef(2));
		materialElement = pMaterial->FirstChildElement("SpecularReflectance");
		str = materialElement->GetText();
		sscanf(str, "%f %f %f", &materials[curr]->specularRef(0), &materials[curr]->specularRef(1),
				&materials[curr]->specularRef(2));

		// Parse mirrors.
		materialElement = pMaterial->FirstChildElement("MirrorReflectance");
		if (materialElement != nullptr)
		{
			str = materialElement->GetText();
			sscanf(str, "%f %f %f", &materials[curr]->mirrorRef(0), &materials[curr]->mirrorRef(1),
					&materials[curr]->mirrorRef(2));
		}
		else
		{
			materials[curr]->mirrorRef(0) = 0.0;
			materials[curr]->mirrorRef(1) = 0.0;
			materials[curr]->mirrorRef(2) = 0.0;
		}

		// Parse PhongExponent.
		materialElement = pMaterial->FirstChildElement("PhongExponent");
		if (materialElement != nullptr)
		{
			materialElement->QueryIntText(&materials[curr]->phongExp);
		}

		// Parse type, RefractionIndex, AbsorptionIndex, AbsorptionCoefficient.
		const XMLAttribute* attr = pMaterial->FirstAttribute();

		materials[curr]->type = Normal;
		while (attr != nullptr)
		{
			if (std::strncmp(attr->Name(), "type", 4) != 0)
			{
				attr = attr->Next();
				continue;
			}

			if (std::strncmp(attr->Value(), "dielectric", 10) == 0)
			{
				materials[curr]->type = Dielectric;
			}
			else if (std::strncmp(attr->Value(), "conductor", 9) == 0)
			{
				materials[curr]->type = Conductor;
			}
			else if (std::strncmp(attr->Value(), "mirror", 6) == 0)
			{
				materials[curr]->type = Mirror;
			}
			else
			{
				materials[curr]->type = Normal;
			}
			break;
		}

		materialElement = pMaterial->FirstChildElement("RefractionIndex");
		if (materialElement != nullptr)
		{
			materialElement->QueryFloatText(&materials[curr]->refractionIndex);
		}
		else
		{
			materials[curr]->refractionIndex = 0;
		}

		materialElement = pMaterial->FirstChildElement("AbsorptionIndex");
		if (materialElement != nullptr)
		{
			materialElement->QueryFloatText(&materials[curr]->absorptionIndex);
		}
		else
		{
			materials[curr]->absorptionIndex = 0;
		}

		materialElement = pMaterial->FirstChildElement("AbsorptionCoefficient");
		if (materialElement != nullptr)
		{
			str = materialElement->GetText();
			sscanf(str, "%f %f %f", &materials[curr]->absorptionCoefficient(0),
					&materials[curr]->absorptionCoefficient(1),
					&materials[curr]->absorptionCoefficient(2));
		}
		else
		{
			materials[curr]->absorptionCoefficient(0) = 0.0;
			materials[curr]->absorptionCoefficient(1) = 0.0;
			materials[curr]->absorptionCoefficient(2) = 0.0;
		}

		// Move onto the next element.
		pMaterial = pMaterial->NextSiblingElement("Material");
	}

	// Parse vertex data
	pElement = pRoot->FirstChildElement("VertexData");
	int cursor = 0;
	Vector3f tmpPoint;
	str = pElement->GetText();
	while (str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
	{
		cursor++;
	}
	while (str[cursor] != '\0')
	{
		for (int cnt = 0; cnt < 3; cnt++)
		{
			if (cnt == 0)
			{
				tmpPoint(0) = atof(str + cursor);
			}
			else if (cnt == 1)
			{
				tmpPoint(1) = atof(str + cursor);
			}
			else
			{
				tmpPoint(2) = atof(str + cursor);
			}
			while (str[cursor] != ' ' && str[cursor] != '\t' && str[cursor] != '\n')
			{
				cursor++;
			}
			while (str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
			{
				cursor++;
			}
		}
		vertices.push_back(tmpPoint);
	}

	// Parse objects
	pElement = pRoot->FirstChildElement("Objects");

	// Parse spheres
	XMLElement* pObject = pElement->FirstChildElement("Sphere");
	XMLElement* objElement;
	while (pObject != nullptr)
	{
		int id;
		int matIndex;
		int cIndex;
		float R;

		eResult = pObject->QueryIntAttribute("id", &id);
		objElement = pObject->FirstChildElement("Material");
		eResult = objElement->QueryIntText(&matIndex);
		objElement = pObject->FirstChildElement("Center");
		eResult = objElement->QueryIntText(&cIndex);
		objElement = pObject->FirstChildElement("Radius");
		eResult = objElement->QueryFloatText(&R);

		objects.push_back(new Sphere(id, matIndex, cIndex, R));

		pObject = pObject->NextSiblingElement("Sphere");
	}

	// Parse triangles
	pObject = pElement->FirstChildElement("Triangle");
	while (pObject != nullptr)
	{
		int id;
		int matIndex;
		int p1Index;
		int p2Index;
		int p3Index;

		eResult = pObject->QueryIntAttribute("id", &id);
		objElement = pObject->FirstChildElement("Material");
		eResult = objElement->QueryIntText(&matIndex);
		objElement = pObject->FirstChildElement("Indices");
		str = objElement->GetText();
		sscanf(str, "%d %d %d", &p1Index, &p2Index, &p3Index);

		objects.push_back(new Triangle(id, matIndex, p1Index, p2Index, p3Index));

		pObject = pObject->NextSiblingElement("Triangle");
	}

	// Parse meshes
	pObject = pElement->FirstChildElement("Mesh");
	while (pObject != nullptr)
	{
		int id;
		int matIndex;
		int p1Index;
		int p2Index;
		int p3Index;
		int cursor = 0;
		int vertexOffset = 0;
		std::vector<Triangle> faces;

		eResult = pObject->QueryIntAttribute("id", &id);
		objElement = pObject->FirstChildElement("Material");
		eResult = objElement->QueryIntText(&matIndex);
		objElement = pObject->FirstChildElement("Faces");
		objElement->QueryIntAttribute("vertexOffset", &vertexOffset);
		str = objElement->GetText();
		while (str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
		{
			cursor++;
		}
		while (str[cursor] != '\0')
		{
			for (int cnt = 0; cnt < 3; cnt++)
			{
				if (cnt == 0)
				{
					p1Index = atoi(str + cursor) + vertexOffset;
				}
				else if (cnt == 1)
				{
					p2Index = atoi(str + cursor) + vertexOffset;
				}
				else
				{
					p3Index = atoi(str + cursor) + vertexOffset;
				}
				while (str[cursor] != ' ' && str[cursor] != '\t' && str[cursor] != '\n')
				{
					cursor++;
				}
				while (str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
				{
					cursor++;
				}
			}
			faces.push_back(*(new Triangle(-1, matIndex, p1Index, p2Index, p3Index)));
		}

		objects.push_back(new Mesh(id, matIndex, faces));

		pObject = pObject->NextSiblingElement("Mesh");
	}

	// Parse lights
	int id;
	Vector3f position;
	Vector3f intensity;
	pElement = pRoot->FirstChildElement("Lights");

	XMLElement* pLight = pElement->FirstChildElement("AmbientLight");
	XMLElement* lightElement;
	str = pLight->GetText();
	sscanf(str, "%f %f %f", &ambientLight(0), &ambientLight(1), &ambientLight(2));

	pLight = pElement->FirstChildElement("PointLight");
	while (pLight != nullptr)
	{
		eResult = pLight->QueryIntAttribute("id", &id);
		lightElement = pLight->FirstChildElement("Position");
		str = lightElement->GetText();
		sscanf(str, "%f %f %f", &position(0), &position(1), &position(2));
		lightElement = pLight->FirstChildElement("Intensity");
		str = lightElement->GetText();
		sscanf(str, "%f %f %f", &intensity(0), &intensity(1), &intensity(2));

		lights.push_back(new PointLight(position, intensity));

		pLight = pLight->NextSiblingElement("PointLight");
	}

	std::cout << "Parsing complete." << std::endl;
}
