#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <vector>
#include "Ray.h"
#include "Eigen/Dense"

#include "defs.h"
#include "Transformation.h"

class Shape
{
public:
    int id;
    int matIndex;

    std::vector<Transformation*> objTransformations;

    virtual ReturnVal intersect(
            const Ray& ray) const = 0;
    virtual void FillPrimitives(std::vector<Shape*> &primitives) const = 0;
    virtual BBox GetBoundingBox() const = 0;
    virtual Eigen::Vector3f GetCenter() const = 0;

    Shape(void);

    Shape(int id, int matIndex);

private:

};

class Sphere : public Shape
{
public:
    Sphere(void);

    Sphere(int id, int matIndex, int cIndex, float R, const std::vector<Transformation*>& transformations);
    Sphere(int id, int matIndex, int cIndex, float R);

    ReturnVal intersect(
            const Ray& ray) const;
    void FillPrimitives(std::vector<Shape*> &primitives) const;
	BBox GetBoundingBox() const;
	Eigen::Vector3f GetCenter() const;

private:
    int cIndex;
    float R;
};

class Triangle : public Shape
{
public:
    Triangle(void);

    Triangle(int id, int matIndex, int p1Index, int p2Index, int p3Index, const std::vector<Transformation*>& transformations);
    Triangle(int id, int matIndex, int p1Index, int p2Index, int p3Index);
    int GetIndexOne();
    int GetIndexTwo();
    int GetIndexThree();

    ReturnVal intersect(
            const Ray& ray) const;
	void FillPrimitives(std::vector<Shape*> &primitives) const;
	BBox GetBoundingBox() const;
	Eigen::Vector3f GetCenter() const;

private:
    int p1Index;
    int p2Index;
    int p3Index;
};

class Mesh : public Shape
{
public:
    Mesh(void);

    Mesh(int id, int matIndex, const std::vector<Triangle>& faces, const std::vector<Transformation*>& transformations);

    ReturnVal intersect(
            const Ray& ray) const;
	void FillPrimitives(std::vector<Shape*> &primitives) const;
	BBox GetBoundingBox() const;
	Eigen::Vector3f GetCenter() const;

private:
    std::vector<Triangle> faces;
};

#endif
