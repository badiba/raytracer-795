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

// Class to hold everything related to a scene.
class Scene
{
public:
    int maxRecursionDepth;            // Maximum recursion depth
    float intTestEps;                // IntersectionTestEpsilon. You will need this one while implementing intersect routines in Shape class
    float shadowRayEps;                // ShadowRayEpsilon. You will need this one while generating shadow rays.
    Eigen::Vector3f backgroundColor;        // Background color
    Eigen::Vector3f ambientLight;            // Ambient light radiance

    std::vector<Camera*> cameras;        // Vector holding all cameras
    std::vector<PointLight*> lights;    // Vector holding all point lights
    std::vector<Material*> materials;    // Vector holding all materials
    std::vector<Eigen::Vector3f> vertices;        // Vector holding all vertices (vertex data)
    std::vector<Shape*> objects;        // Vector holding all shapes

    Scene(const char* xmlPath);        // Constructor. Parses XML file and initializes vectors above. Implemented for you.

    void renderScene(
            void);            // Method to render scene, an image is created for each camera in the scene. You will implement this.

private:
    // Write any other stuff here
    Eigen::Vector3f diffuse(ReturnVal ret, Material* mat, Ray ray, PointLight* light);

    bool isDark(Eigen::Vector3f point, PointLight* light);

    Eigen::Vector3f specular(Ray ray, ReturnVal ret, Material* mat, PointLight* light);

    Eigen::Vector3f ambient(Material* mat);

    Color shading(Ray ray, ReturnVal ret, Material* mat);
};

#endif