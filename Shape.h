#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <vector>
#include "Ray.h"
#include "Eigen/Dense"

#include "defs.h"

class Shape
{
public:
    int id;
    int matIndex;

    virtual ReturnVal intersect(
            const Ray& ray) const = 0;

    Shape(void);

    Shape(int id, int matIndex);

private:

};

class Sphere : public Shape
{
public:
    Sphere(void);

    Sphere(int id, int matIndex, int cIndex, float R);

    ReturnVal intersect(
            const Ray& ray) const;

private:
    int cIndex;
    float R;
};

class Triangle : public Shape
{
public:
    Triangle(void);

    Triangle(int id, int matIndex, int p1Index, int p2Index, int p3Index);

    ReturnVal intersect(
            const Ray& ray) const;

private:
    int p1Index;
    int p2Index;
    int p3Index;
};

class Mesh : public Shape
{
public:
    Mesh(void);

    Mesh(int id, int matIndex, const std::vector<Triangle>& faces);

    ReturnVal intersect(
            const Ray& ray) const;

private:
    std::vector<Triangle> faces;
};

#endif
