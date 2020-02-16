#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "Shape.h"
#include "tinyxml2.h"
#include <stdio.h>
#include "Image.h"
#include <algorithm>
#include <cmath>
#include <thread>
#include <time.h>

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

Color Scene::shading(Ray ray, ReturnVal ret, Material* mat)
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

    // Clamp and return.
    rawColor = rawColor.cwiseMin(255);
    Color clampedColor{ (unsigned char)(rawColor(0)), (unsigned char)(rawColor(1)), (unsigned char)(rawColor(2)) };
    return clampedColor;
}

void Scene::renderScene(void)
{
    // Save an image for all cameras.
    for (int i = 0; i < cameras.size(); i++)
    {
        Camera* cam = cameras[i];
        Ray ray;
        int width = cam->imgPlane.nx;
        int height = cam->imgPlane.ny;
        Image image(width, height);

        // For every pixel create a ray.
        for (int i = 0; i < cam->imgPlane.nx; i++)
        {
            for (int j = 0; j < cam->imgPlane.ny; j++)
            {
                ray = cam->getPrimaryRay(i, j);

                ReturnVal ret;
                ReturnVal nearestRet;
                float returnDistance = 0;
                float nearestPoint = std::numeric_limits<float>::max();
                int nearestObjectIndex = 0;

                // Check intersection of the ray with all objects.
                for (int k = 0; k < objects.size(); k++)
                {
                    ret = objects[k]->intersect(ray);
                    if (ret.full)
                    {
                        // Save the nearest intersected object.
                        returnDistance = (ret.point - ray.origin).norm();
                        if (returnDistance < nearestPoint)
                        {
                            nearestObjectIndex = k;
                            nearestPoint = returnDistance;
                            nearestRet = ret;
                        }
                    }
                }

                // If any intersection happened, compute shading.
                if (nearestRet.full)
                {
                    image.setPixelValue(i, j, shading(ray, nearestRet,
                            materials[objects[nearestObjectIndex]->matIndex - 1]));
                }
                // Else paint with background color.
                else
                {
                    image.setPixelValue(i, j, Color{ (unsigned char)backgroundColor(0),
                            (unsigned char)backgroundColor(1),
                            (unsigned char)backgroundColor(2) });
                }
            }
        }

        // Save image.
        image.saveImage(cam->imageName);
    }
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

    // Parse materals
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
        materialElement = pMaterial->FirstChildElement("PhongExponent");
        if (materialElement != nullptr)
        {
            materialElement->QueryIntText(&materials[curr]->phongExp);
        }

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
}