#ifndef _HELPER_H_
#define _HELPER_H_

#include <vector>
#include "Eigen/Dense"
#include <iostream>
#include "glm/ext.hpp"
#include "Shape.h"
#include "glm/gtx/string_cast.hpp"
#include "defs.h"
#include "Instance.h"

namespace Transforming{
    Eigen::Vector3f TransformPoint(Eigen::Vector3f point, glm::mat4 &tMatrix);
    Ray TransformRay(const Ray& ray, glm::mat4 &tMatrix, glm::vec3 &blur);
    Eigen::Vector3f TransformNormal(Eigen::Vector3f normal, glm::mat4 &tMatrix);

    void ComputeObjectTransformations(std::vector<Shape*> &objects, std::vector<Instance*> instances, std::vector<Transformation*> &translations,
                              std::vector<Transformation*> &scalings, std::vector<Transformation*> &rotations
            , std::vector<Transformation*> &composites);
}

namespace BVHMethods{
    bool isNaN(Eigen::Vector3f checkVector);
    ReturnVal FindIntersection(const Ray& ray, std::vector<Shape*> &objects, std::vector<Instance*> &instances);
}

namespace ShapeHelpers
{
    float FindMinOfThree(float a, float b, float c);

    float FindMaxOfThree(float a, float b, float c);

    int FindMinOfThree(int a, int b, int c);

    int FindMaxOfThree(int a, int b, int c);

    int VectorFindMin(std::vector<Triangle> &indices);

    int VectorFindMax(std::vector<Triangle> &indices);
}

namespace GeometryHelpers
{
    int GetAbsSmallestIndex(Eigen::Vector3f &vector);
    Eigen::Vector3f GetOrthonormalUVector(const Eigen::Vector3f &vector);
}

namespace ExrLibrary{
    Eigen::Vector2f ReadExr(const char *filename, float*& data);
    void SaveExr(const char *filename, float* data, int width, int height);
}

#endif
