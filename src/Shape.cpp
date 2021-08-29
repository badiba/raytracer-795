#include "Shape.h"
#include "Scene.h"
#include <cstdio>
#include "math.h"
#include <limits>
#include "Helper.h"
#include "Perlin.h"

using namespace Eigen;

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

Sphere::Sphere(int id, int matIndex, int cIndex, float R, const std::vector<Transformation *> &transformations,
               glm::vec3 &blurTransformation, bool isBlur)
    : Shape(id, matIndex)
{
    this->id = id;
    this->matIndex = matIndex;
    this->cIndex = cIndex;
    this->R = R;
    this->objTransformations = transformations;
    this->blurTransformation = blurTransformation;
    this->isBlur = isBlur;
}

Sphere::Sphere(int id, int matIndex, int cIndex, float R)
    : Shape(id, matIndex)
{
    this->id = id;
    this->matIndex = matIndex;
    this->cIndex = cIndex;
    this->R = R;
}

ReturnVal Sphere::intersect(const Ray &ray) const
{
}

void Sphere::FillPrimitives(std::vector<Shape *> &primitives) const
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

Triangle::Triangle(int id, int matIndex, int p1Index, int p2Index, int p3Index, const std::vector<Transformation *> &transformations,
                   glm::vec3 &blurTransformation, bool isBlur)
    : Shape(id, matIndex)
{
    this->id = id;
    this->matIndex = matIndex;
    this->p1Index = p1Index;
    this->p2Index = p2Index;
    this->p3Index = p3Index;
    this->objTransformations = transformations;
    this->blurTransformation = blurTransformation;
    this->isBlur = isBlur;
}

Triangle::Triangle(int id, int matIndex, int p1Index, int p2Index, int p3Index, bool isSmooth)
    : Shape(id, matIndex)
{
    this->id = id;
    this->matIndex = matIndex;
    this->p1Index = p1Index;
    this->p2Index = p2Index;
    this->p3Index = p3Index;
    this->isSmooth = isSmooth;
}

int Triangle::GetIndexOne()
{
    return p1Index;
}

int Triangle::GetIndexTwo()
{
    return p2Index;
}

int Triangle::GetIndexThree()
{
    return p3Index;
}

ReturnVal Triangle::intersect(const Ray &ray) const
{
    Vector3f a, b, c;
    a = pScene->vertices[p1Index - 1];
    b = pScene->vertices[p2Index - 1];
    c = pScene->vertices[p3Index - 1];

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

    Vector3f normal;
    if (isSmooth)
    {
        float alpha = 1 - beta - gamma;
        normal = pScene->vertexNormals[p1Index - 1] * alpha + pScene->vertexNormals[p2Index - 1] * beta +
                 pScene->vertexNormals[p3Index - 1] * gamma;
    }
    else
    {
        normal = (c - b).cross(a - b);
    }

    if (t >= -pScene->intTestEps && (beta + gamma <= 1) && beta >= -pScene->intTestEps &&
        gamma >= -pScene->intTestEps)
    {
        ret.normal = normal / normal.norm();
        ret.point = ray.getPoint(t);
        ret.full = true;
    }

    return ret;
}

void Triangle::FillPrimitives(std::vector<Shape *> &primitives) const
{
    primitives.push_back(new Triangle(id, matIndex, p1Index, p2Index, p3Index, isSmooth));
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

    return Vector3f{(a[0] + b[0] + c[0]) / 3.0f,
                    (a[1] + b[1] + c[1]) / 3.0f,
                    (a[2] + b[2] + c[2]) / 3.0f};
}

Mesh::Mesh()
{
}

Mesh::Mesh(int id, int matIndex, const std::vector<Triangle *> &faces, const std::vector<Transformation *> &transformations,
           glm::vec3 &blurTransformation, bool isBlur, bool isSmooth)
    : Shape(id, matIndex)
{
    this->id = id;
    this->matIndex = matIndex;
    this->faces = faces;
    this->objTransformations = transformations;
    this->blurTransformation = blurTransformation;
    this->isBlur = isBlur;
    this->isSmooth = isSmooth;
}

ReturnVal Mesh::intersect(const Ray &ray) const
{
    ReturnVal ret;

    ReturnVal nearestRet;
    float nearestPoint = std::numeric_limits<float>::max();
    float returnDistance = 0;

    for (int i = 0; i < faces.size(); i++)
    {
        ret = faces[i]->intersect(ray);

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

void Mesh::FillPrimitives(std::vector<Shape *> &primitives) const
{
    for (int i = 0; i < faces.size(); i++)
    {
        primitives.push_back(faces[i]);
    }
}

BBox Mesh::GetBoundingBox() const
{
    return BBox{};
}

Eigen::Vector3f Mesh::GetCenter() const
{
    return Vector3f{};
}

void Shape::ComputeSmoothNormals()
{
}

void Sphere::ComputeSmoothNormals()
{
}

void Triangle::ComputeSmoothNormals()
{
    Vector3f a, b, c;
    a = pScene->vertices[p1Index - 1];
    b = pScene->vertices[p2Index - 1];
    c = pScene->vertices[p3Index - 1];

    Vector3f normal = ((c - b).cross(a - b)).normalized();

    pScene->vertexNormals[p1Index - 1] += normal;
    pScene->vertexNormals[p2Index - 1] += normal;
    pScene->vertexNormals[p3Index - 1] += normal;

    isSmooth = true;
}

void Mesh::ComputeSmoothNormals()
{
    if (!isSmooth)
    {
        return;
    }

    int faceSize = faces.size();
    for (int i = 0; i < faceSize; i++)
    {
        faces[i]->ComputeSmoothNormals();
    }
}

ReturnVal Mesh::bvhIntersect(const Ray &ray, std::vector<int> &txt, int txtOffset) const
{
    return intersect(ray);
}

ReturnVal Triangle::bvhIntersect(const Ray &ray, std::vector<int> &txt, int txtOffset) const
{
    Vector3f a, b, c;
    a = pScene->vertices[p1Index - 1];
    b = pScene->vertices[p2Index - 1];
    c = pScene->vertices[p3Index - 1];

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

    Vector3f normal;
    if (isSmooth)
    {
        float alpha = 1 - beta - gamma;
        normal = pScene->vertexNormals[p1Index - 1] * alpha + pScene->vertexNormals[p2Index - 1] * beta +
                 pScene->vertexNormals[p3Index - 1] * gamma;
    }
    else
    {
        normal = (c - b).cross(a - b);
    }

    if (t >= -pScene->intTestEps && (beta + gamma <= 1) && beta >= -pScene->intTestEps &&
        gamma >= -pScene->intTestEps)
    {
        ret.normal = normal / normal.norm();
        ret.point = ray.getPoint(t);

        // ---------- Texture computations. ---------- //
        Vector3f e1 = b - a;
        Vector3f e2 = c - a;
        ret = TextureComputation(ret, txt, txtOffset, e1, e2, beta, gamma);

        ret.full = true;
    }

    return ret;
}

ReturnVal Sphere::bvhIntersect(const Ray &ray, std::vector<int> &txt, int txtOffset) const
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
    if (t1 >= 0 && t2 < 0)
    {
        intersectionPoint = ray.getPoint(t1);
    }
    else if (t2 >= 0 && t1 < 0)
    {
        intersectionPoint = ray.getPoint(t2);
    }
    else if (t1 < 0 && t2 < 0)
    {
        ret.full = false;
        return ret;
    }
    else
    {
        if (t1 < t2)
        {
            intersectionPoint = ray.getPoint(t1);
        }
        else
        {
            intersectionPoint = ray.getPoint(t2);
        }
    }

    ret.point = intersectionPoint;
    ret.normal = (intersectionPoint - c) / (intersectionPoint - c).norm();

    // Do texture computations.
    ret = TextureComputation(ret, txt);

    ret.full = true;
    return ret;
}

ReturnVal Sphere::TextureComputation(ReturnVal &ret, std::vector<int> &txt) const
{
    ret.dm = NoDecal;
    Texture *texture;
    for (int i = 0; i < txt.size(); i++)
    {
        texture = pScene->textures[txt[i] - 1];
        if (texture->type == ImageTexture)
        {
            if (texture->decalMode == ReplaceKd || texture->decalMode == BlendKd || texture->decalMode == ReplaceAll)
            {
                ret.dm = texture->decalMode;

                Vector3f localCoordinates = ret.point - pScene->vertices[cIndex - 1];
                float textureTheta = acos(localCoordinates[1] / R);
                float texturePhi = atan2(localCoordinates[2], localCoordinates[0]);
                float textureU = (-texturePhi + M_PI) / (2 * M_PI);
                float textureV = textureTheta / M_PI;

                ret.textureColor = texture->GetColorAtCoordinates(textureU, textureV);
                ret.textureNormalizer = texture->normalizer;
            }
            else if (texture->decalMode == ReplaceNormal)
            {
                Vector3f localCoordinates = ret.point - pScene->vertices[cIndex - 1];
                float textureTheta = acos(localCoordinates[1] / R);
                float texturePhi = atan2(localCoordinates[2], localCoordinates[0]);
                float textureU = (-texturePhi + M_PI) / (2 * M_PI);
                float textureV = textureTheta / M_PI;

                float pi = M_PI;
                Vector3f dpdu = Vector3f{localCoordinates[2] * 2 * pi, 0, localCoordinates[0] * (-2) * pi};
                Vector3f dpdv = Vector3f{localCoordinates[1] * cos(texturePhi) * pi, (-1) * R * sin(textureTheta) * pi,
                                         localCoordinates[1] * sin(texturePhi) * pi};

                Vector3f rNormal = texture->GetColorAtCoordinates(textureU, textureV) / 255;
                rNormal = (rNormal - Vector3f{0.5f, 0.5f, 0.5f}).normalized();

                Matrix3f TBN;
                TBN.col(0) = dpdu.normalized();
                TBN.col(1) = dpdv.normalized();
                TBN.col(2) = ret.normal;

                ret.normal = TBN * rNormal;
            }
            else if (texture->decalMode == BumpNormal)
            {
                Vector3f localCoordinates = ret.point - pScene->vertices[cIndex - 1];
                float textureTheta = acos(localCoordinates[1] / R);
                float texturePhi = atan2(localCoordinates[2], localCoordinates[0]);
                float textureU = (-texturePhi + M_PI) / (2 * M_PI);
                float textureV = textureTheta / M_PI;

                float pi = M_PI;
                Vector3f dpdu = Vector3f{localCoordinates[2] * 2 * pi, 0, localCoordinates[0] * (-2) * pi};
                Vector3f dpdv = Vector3f{localCoordinates[1] * cos(texturePhi) * pi, (-1) * R * sin(textureTheta) * pi,
                                         localCoordinates[1] * sin(texturePhi) * pi};

                Vector2f derivatives = texture->GetChangeAtCoordinates(textureU, textureV) * (texture->bumpFactor);

                Vector3f dpPrimedu = (dpdu + derivatives[0] * ret.normal);
                Vector3f dpPrimedv = (dpdv + derivatives[1] * ret.normal);

                Vector3f newNormal = dpPrimedv.cross(dpPrimedu).normalized();
                if (ret.normal.dot(newNormal) > 0)
                {
                    ret.normal = newNormal;
                }
                else
                {
                    ret.normal = -newNormal;
                }
            }
        }
        else
        {
            if (texture->decalMode == ReplaceKd)
            {
                ret.dm = texture->decalMode;
                float perlin = pScene->perlin->ComputePerlin(ret.point, texture->noiseScale, texture->nc);

                ret.textureColor = {perlin, perlin, perlin};
                ret.textureNormalizer = 1;
            }
            else if (texture->decalMode == BumpNormal)
            {
                Vector3f g = pScene->perlin->GetGradient(ret.point, texture->noiseScale, texture->nc);
                Vector3f gParallel = g.dot(ret.normal) * ret.normal;
                Vector3f newNormal = ret.normal - (g - gParallel) * texture->bumpFactor;
                if (ret.normal.dot(newNormal) > 0)
                {
                    ret.normal = newNormal;
                }
                else
                {
                    ret.normal = -newNormal;
                }
                ret.normal = ret.normal.normalized();
            }
        }
    }

    return ret;
}

ReturnVal Triangle::TextureComputation(ReturnVal &ret, std::vector<int> &txt, int txtOffset, Eigen::Vector3f &e1, Eigen::Vector3f &e2, float beta, float gamma) const
{
    ret.dm = NoDecal;

    if (txt.size() == 0)
    {
        return ret;
    }

    float alpha = 1 - beta - gamma;
    Vector2f uv_0 = pScene->textureCoordinates[p1Index - 1 + txtOffset];
    Vector2f uv_1 = pScene->textureCoordinates[p2Index - 1 + txtOffset];
    Vector2f uv_2 = pScene->textureCoordinates[p3Index - 1 + txtOffset];
    Vector2f uv = uv_0 * alpha + uv_1 * beta + uv_2 * gamma;

    Texture *texture;
    for (int i = 0; i < txt.size(); i++)
    {
        texture = pScene->textures[txt[i] - 1];
        if (texture->type == ImageTexture)
        {
            if (texture->decalMode == ReplaceKd || texture->decalMode == BlendKd || texture->decalMode == ReplaceAll)
            {
                ret.dm = texture->decalMode;

                ret.textureColor = texture->GetColorAtCoordinates(uv[0], uv[1]);
                ret.textureNormalizer = texture->normalizer;
            }
            else if (texture->decalMode == ReplaceNormal)
            {
                MatrixXf E(2, 3);
                E.row(0) = e1;
                E.row(1) = e2;

                Matrix2f A;
                A << uv_1[0] - uv_0[0], uv_1[1] - uv_0[1], uv_2[0] - uv_0[0], uv_2[1] - uv_0[1];

                MatrixXf TB(2, 3);
                TB = A.inverse() * E;

                Vector3f rNormal = texture->GetColorAtCoordinates(uv[0], uv[1]) / 255;
                rNormal = (rNormal - Vector3f{0.5f, 0.5f, 0.5f}).normalized();

                Matrix3f TBN;
                TBN.col(0) = TB.row(0);
                TBN.col(1) = TB.row(1);
                TBN.col(2) = ret.normal;

                ret.normal = TBN * rNormal;
            }
            else if (texture->decalMode == BumpNormal)
            {
                MatrixXf E(2, 3);
                E.row(0) = e1;
                E.row(1) = e2;

                Matrix2f A;
                A << uv_1[0] - uv_0[0], uv_1[1] - uv_0[1], uv_2[0] - uv_0[0], uv_2[1] - uv_0[1];

                MatrixXf TB(2, 3);
                TB = A.inverse() * E;

                Vector2f derivatives = texture->GetChangeAtCoordinates(uv[0], uv[1]) * texture->bumpFactor;
                Vector3f T = TB.row(0);
                Vector3f B = TB.row(1);

                Vector3f dpPrimedu = (T + derivatives[0] * ret.normal);
                Vector3f dpPrimedv = (B + derivatives[1] * ret.normal);

                Vector3f newNormal = dpPrimedv.cross(dpPrimedu).normalized();
                if (ret.normal.dot(newNormal) > 0)
                {
                    ret.normal = newNormal;
                }
                else
                {
                    ret.normal = -newNormal;
                }
            }
        }
        else
        {
            if (texture->decalMode == ReplaceKd)
            {
                ret.dm = texture->decalMode;

                float perlin = pScene->perlin->ComputePerlin(ret.point, texture->noiseScale, texture->nc);

                ret.textureColor = {perlin, perlin, perlin};
                ret.textureNormalizer = 1;
            }
            else if (texture->decalMode == BumpNormal)
            {
                Vector3f g = pScene->perlin->GetGradient(ret.point, texture->noiseScale, texture->nc);
                Vector3f gParallel = g.dot(ret.normal) * ret.normal;

                Vector3f newNormal = ret.normal - (g - gParallel) * texture->bumpFactor;
                if (ret.normal.dot(newNormal) > 0)
                {
                    ret.normal = newNormal;
                }
                else
                {
                    ret.normal = -newNormal;
                }
                ret.normal = ret.normal.normalized();
            }
        }
    }

    return ret;
}