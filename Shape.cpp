#include "Shape.h"
#include "Scene.h"
#include <cstdio>
#include "math.h"
#include <limits>

Shape::Shape(void) {
}

Shape::Shape(int id, int matIndex)
        : id(id), matIndex(matIndex) {
}

Sphere::Sphere(void) {}

/* Constructor for sphere. You will implement this. */
Sphere::Sphere(int id, int matIndex, int cIndex, float R)
        : Shape(id, matIndex) {
    this->id = id;
    this->matIndex = matIndex;
    this->cIndex = cIndex;
    this->R = R;
}

/* Sphere-ray intersection routine. You will implement this.
Note that ReturnVal structure should hold the information related to the intersection point, e.g., coordinate of that point, normal at that point etc.
You should to declare the variables in ReturnVal structure you think you will need. It is in defs.h file. */
ReturnVal Sphere::intersect(const Ray &ray) const {
    Vector3f d, o, c;
    d = ray.direction;
    o = ray.origin;
    c = pScene->vertices[cIndex - 1];
    ReturnVal ret;

    float discriminant = ((d.dot(o - c)) * (d.dot(o - c)) - (d.dot(d)) * ((o - c).dot(o - c) - R * R));
    if (discriminant < pScene->intTestEps) {
        ret.full = false;
        return ret;
    }
    float t1 = (-d.dot(o - c) + sqrt(discriminant)) / (d.dot(d));
    float t2 = (-d.dot(o - c) - sqrt(discriminant)) / (d.dot(d));
    Vector3f intersectionPoint;
    if (t2 > t1 && t2 > 0) {
        intersectionPoint = ray.getPoint(t1);
    } else if (t1 > 0) {
        intersectionPoint = ray.getPoint(t2);
    } else {
        ret.full = false;
        return ret;
    }

    ret.point = intersectionPoint;
    ret.normal = (intersectionPoint - c) / (intersectionPoint - c).norm();
    ret.full = true;

    return ret;
}

Triangle::Triangle(void) {}

/* Constructor for triangle. You will implement this. */
Triangle::Triangle(int id, int matIndex, int p1Index, int p2Index, int p3Index)
        : Shape(id, matIndex) {
    this->id = id;
    this->matIndex = matIndex;
    this->p1Index = p1Index;
    this->p2Index = p2Index;
    this->p3Index = p3Index;
}

/* Triangle-ray intersection routine. You will implement this.
Note that ReturnVal structure should hold the information related to the intersection point, e.g., coordinate of that point, normal at that point etc.
You should to declare the variables in ReturnVal structure you think you will need. It is in defs.h file. */
ReturnVal Triangle::intersect(const Ray &ray) const {
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
        gamma >= -pScene->intTestEps) {
        ret.normal = normal / normal.norm();
        ret.point = ray.getPoint(t);
        ret.full = true;
    }

    return ret;
}

Mesh::Mesh() {}

/* Constructor for mesh. You will implement this. */
Mesh::Mesh(int id, int matIndex, const vector<Triangle> &faces)
        : Shape(id, matIndex) {
    this->id = id;
    this->matIndex = matIndex;
    this->faces = faces;
}

/* Mesh-ray intersection routine. You will implement this.
Note that ReturnVal structure should hold the information related to the intersection point, e.g., coordinate of that point, normal at that point etc.
You should to declare the variables in ReturnVal structure you think you will need. It is in defs.h file. */
ReturnVal Mesh::intersect(const Ray &ray) const {
    ReturnVal ret;

    ReturnVal nearestRet;
    float nearestPoint = std::numeric_limits<float>::max();
    float returnDistance = 0;

    for (int i = 0; i < faces.size(); i++) {
        ret = faces[i].intersect(ray);

        if (ret.full) {
            returnDistance = (ret.point - ray.origin).norm();

            if (returnDistance < nearestPoint) {
                nearestPoint = returnDistance;
                nearestRet = ret;
            }
        }
    }

    return nearestRet;
}
