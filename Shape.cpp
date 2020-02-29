#include "Shape.h"
#include "Scene.h"
#include <cstdio>
#include "math.h"
#include <limits>

using namespace Eigen;

namespace ShapeHelpers
{
	float FindMinOfThree(float a, float b, float c)
	{
		if (a <= b && a <= c){
			return a;
		}
		else if (b <= a && b <= c){
			return b;
		}

		return c;
	}

	float FindMaxOfThree(float a, float b, float c)
	{
		if (a >= b && a >= c){
			return a;
		}
		else if (b >= a && b >= c){
			return b;
		}

		return c;
	}
}

Shape::Shape(void)
{
}

Shape::Shape(int id, int matIndex)
        : id(id), matIndex(matIndex)
{
}

Sphere::Sphere(void)
{
}

Sphere::Sphere(int id, int matIndex, int cIndex, float R)
        : Shape(id, matIndex)
{
    this->id = id;
    this->matIndex = matIndex;
    this->cIndex = cIndex;
    this->R = R;
}

ReturnVal Sphere::intersect(const Ray& ray) const
{
    Vector3f d, o, c;
    d = ray.direction;
    o = ray.origin;
    c = pScene->vertices[cIndex - 1];
    ReturnVal ret;

    float discriminant = ((d.dot(o - c)) * (d.dot(o - c)) - (d.dot(d)) * ((o - c).dot(o - c) - R * R));
    if (discriminant < pScene->intTestEps)
    {
        ret.full = false;
        return ret;
    }
    float t1 = (-d.dot(o - c) + sqrt(discriminant)) / (d.dot(d));
    float t2 = (-d.dot(o - c) - sqrt(discriminant)) / (d.dot(d));
    Vector3f intersectionPoint;
    if (t2 > t1 && t2 > 0)
    {
        intersectionPoint = ray.getPoint(t1);
    }
    else if (t1 > 0)
    {
        intersectionPoint = ray.getPoint(t2);
    }
    else
    {
        ret.full = false;
        return ret;
    }

    ret.point = intersectionPoint;
    ret.normal = (intersectionPoint - c) / (intersectionPoint - c).norm();
    ret.full = true;

    return ret;
}

void Sphere::FillPrimitives(std::vector<Shape*> &primitives) const
{
	primitives.push_back(new Sphere(id, matIndex, cIndex, R));
}

BBox Sphere::GetBoundingBox() const
{
	Vector3f center = pScene->vertices[cIndex - 1];
	Vector3f minPoint = {center[0] - R, center[1] - R, center[2] - R};
	Vector3f maxPoint = {center[0] + R, center[1] + R, center[2] + R};

	return BBox{minPoint, maxPoint};
}

Eigen::Vector3f Sphere::GetCenter() const
{
	return pScene->vertices[cIndex - 1];
}

Triangle::Triangle(void)
{
}

Triangle::Triangle(int id, int matIndex, int p1Index, int p2Index, int p3Index)
        : Shape(id, matIndex)
{
    this->id = id;
    this->matIndex = matIndex;
    this->p1Index = p1Index;
    this->p2Index = p2Index;
    this->p3Index = p3Index;
}

ReturnVal Triangle::intersect(const Ray& ray) const
{
    Vector3f a, b, c;
    a = pScene->vertices[p1Index - 1];
    b = pScene->vertices[p2Index - 1];
    c = pScene->vertices[p3Index - 1];

    Vector3f normal = (c - b).cross(a - b);

    Matrix3f matrix, matrix_beta, matrix_gamma, matrix_t;
    matrix << a - b, a - c, ray.direction;
    matrix_beta << a - ray.origin, a - c, ray.direction;
    matrix_gamma << a - b, a - ray.origin, ray.direction;
    matrix_t << a - b, a - c, a - ray.origin;
    ReturnVal ret;

    float det = matrix.determinant();

    float beta, gamma, t;
    beta = (matrix_beta).determinant() / (det);
    gamma = (matrix_gamma).determinant() / (det);
    t = (matrix_t).determinant() / (det);

    if (t >= -pScene->intTestEps && (beta + gamma <= 1) && beta >= -pScene->intTestEps &&
            gamma >= -pScene->intTestEps)
    {
        ret.normal = normal / normal.norm();
        ret.point = ray.getPoint(t);
        ret.full = true;
    }

    return ret;
}

void Triangle::FillPrimitives(std::vector<Shape*> &primitives) const
{
	primitives.push_back(new Triangle(id, matIndex, p1Index, p2Index, p3Index));
}

BBox Triangle::GetBoundingBox() const
{
	Vector3f a, b, c;
	a = pScene->vertices[p1Index - 1];
	b = pScene->vertices[p2Index - 1];
	c = pScene->vertices[p3Index - 1];

	Vector3f minPoint = {ShapeHelpers::FindMinOfThree(a[0], b[0], c[0]),
			ShapeHelpers::FindMinOfThree(a[1], b[1], c[1]),
			ShapeHelpers::FindMinOfThree(a[2], b[2], c[2])};

	Vector3f maxPoint = {ShapeHelpers::FindMaxOfThree(a[0], b[0], c[0]),
			ShapeHelpers::FindMaxOfThree(a[1], b[1], c[1]),
			ShapeHelpers::FindMaxOfThree(a[2], b[2], c[2])};

	return BBox{minPoint, maxPoint};
}

Eigen::Vector3f Triangle::GetCenter() const
{
	Vector3f a, b, c;
	a = pScene->vertices[p1Index - 1];
	b = pScene->vertices[p2Index - 1];
	c = pScene->vertices[p3Index - 1];

	return Vector3f {(a[0] + b[0] + c[0]) / 3.0f,
			(a[1] + b[1] + c[1]) / 3.0f,
			(a[2] + b[2] + c[2]) / 3.0f};
}

Mesh::Mesh()
{
}

Mesh::Mesh(int id, int matIndex, const std::vector<Triangle>& faces)
        : Shape(id, matIndex)
{
    this->id = id;
    this->matIndex = matIndex;
    this->faces = faces;
}

ReturnVal Mesh::intersect(const Ray& ray) const
{
    ReturnVal ret;

    ReturnVal nearestRet;
    float nearestPoint = std::numeric_limits<float>::max();
    float returnDistance = 0;

    for (int i = 0; i < faces.size(); i++)
    {
        ret = faces[i].intersect(ray);

        if (ret.full)
        {
            returnDistance = (ret.point - ray.origin).norm();

            if (returnDistance < nearestPoint)
            {
                nearestPoint = returnDistance;
                nearestRet = ret;
            }
        }
    }

    return nearestRet;
}

void Mesh::FillPrimitives(std::vector<Shape*> &primitives) const
{
	for (int i = 0; i < faces.size(); i++)
	{
		faces[i].FillPrimitives(primitives);
	}
}

BBox Mesh::GetBoundingBox() const
{
	return BBox{};
}

Eigen::Vector3f Mesh::GetCenter() const
{
	return Vector3f {};
}
