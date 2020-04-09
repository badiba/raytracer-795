#include "Helper.h"
#include "BVH.h"
#include "Instance.h"

namespace BVHMethods{
    bool isNaN(Eigen::Vector3f checkVector){
        if (checkVector[0] != checkVector[0] || checkVector[1] != checkVector[1] || checkVector[2] != checkVector[2])
        {
            return true;
        }

        return false;
    }

    ReturnVal FindIntersection(const Ray &ray, std::vector<Shape*> &objects, std::vector<Instance*> &instances){
        Ray transformedRay(0);
        ReturnVal ret;
        ReturnVal nearestRet = {};
        float nearestDistance = std::numeric_limits<float>::max();
        float distance = 0;
        glm::mat4 transformations(1.0);
        glm::mat4 inverseTransformations;
        glm::mat4 intersectionTransformation;

        if (isNaN(ray.origin) || isNaN(ray.direction)){
            return nearestRet;
        }

        int objectSize = objects.size();
        for (int i = 0; i < objectSize; i++){
            if (objects[i]->isBlur){
                transformations = glm::translate(transformations, objects[i]->blurTransformation * ray.time);
                transformations = transformations * *objects[i]->transformationMatrix;
                inverseTransformations = glm::inverse(transformations);
            }
            else{
                transformations = *objects[i]->transformationMatrix;
                inverseTransformations = *objects[i]->inverse_tMatrix;
            }

            transformedRay = Transforming::TransformRay(ray, inverseTransformations);
            ret = objects[i]->bvh->FindIntersection(transformedRay);
            if (ret.full)
            {
                ret.point = Transforming::TransformPoint(ret.point, transformations);
                distance = (ret.point - ray.origin).norm();

                if (distance < nearestDistance && distance > 0)
                {
                    intersectionTransformation = inverseTransformations;
                    nearestDistance = distance;
                    nearestRet = ret;
                }
            }
        }

        int instanceSize = instances.size();
        for (int i = 0; i < instanceSize; i++){
            if (instances[i]->isBlur){
                transformations = glm::translate(transformations, instances[i]->blurTransformation * ray.time);
                transformations = transformations * *instances[i]->transformationMatrix;
                inverseTransformations = glm::inverse(transformations);
            }
            else{
                transformations = *instances[i]->transformationMatrix;
                inverseTransformations = *instances[i]->inverse_tMatrix;
            }

            transformedRay = Transforming::TransformRay(ray, inverseTransformations);
            ret = instances[i]->baseMesh->bvh->FindIntersection(transformedRay);
            if (ret.full){
                ret.matIndex = instances[i]->matIndex;
                ret.point = Transforming::TransformPoint(ret.point, transformations);
                distance = (ret.point - ray.origin).norm();

                if (distance < nearestDistance && distance > 0)
                {
                    intersectionTransformation = inverseTransformations;
                    nearestDistance = distance;
                    nearestRet = ret;
                }
            }
        }

        if (nearestRet.full){
            nearestRet.normal = Transforming::TransformNormal(nearestRet.normal, intersectionTransformation);
        }

        return nearestRet;
    }
}

namespace Transforming{
    bool FloatEquality(float a, float b){
        float precision = 0.0001f;
        return !(a > b + precision || a < b - precision);

    }

    Eigen::Vector3f TransformPoint(Eigen::Vector3f point, glm::mat4 &tMatrix){
        glm::vec4 glmPoint;
        glmPoint = {point[0], point[1], point[2], 1};

        glm::vec3 glmTransformedPoint;
        glmTransformedPoint = tMatrix * glmPoint;

        return Eigen::Vector3f{glmTransformedPoint[0], glmTransformedPoint[1], glmTransformedPoint[2]};
    }

    Eigen::Vector3f TransformNormal(Eigen::Vector3f normal, glm::mat4 &tMatrix){
        glm::vec4 glmNormal;
        glmNormal = {normal[0], normal[1], normal[2], 1};

        glm::vec3 glmTransformedNormal;
        glmTransformedNormal = glm::transpose(tMatrix) * glmNormal;

        return Eigen::Vector3f{glmTransformedNormal[0], glmTransformedNormal[1], glmTransformedNormal[2]}.normalized();
    }

    Ray TransformRay(const Ray& ray, glm::mat4 &tMatrix){
        glm::vec4 glmOrigin;
        glm::vec4 glmDirection;
        glmOrigin = {ray.origin[0], ray.origin[1], ray.origin[2], 1};
        glmDirection = {ray.direction[0], ray.direction[1], ray.direction[2], 0};

        glm::vec3 glmTransformedOrigin;
        glm::vec3 glmTransformedDirection;
        glmTransformedOrigin = tMatrix * glmOrigin;
        glmTransformedDirection = tMatrix * glmDirection;

        Ray transformedRay(0);
        transformedRay.origin = {glmTransformedOrigin[0], glmTransformedOrigin[1], glmTransformedOrigin[2]};
        transformedRay.direction = {glmTransformedDirection[0], glmTransformedDirection[1], glmTransformedDirection[2]};

        transformedRay.direction = transformedRay.direction.normalized();
        transformedRay.SetTime(ray.time);
        return transformedRay;
    }

    void ComputeObjectTransformations(std::vector<Shape*> &objects, std::vector<Instance*> instances,
            std::vector<Transformation*> &translations, std::vector<Transformation*> &scalings,
            std::vector<Transformation*> &rotations){

        int transformIndex = 0;
        TransformationType type;
        glm::vec3 glmCommon;
        Eigen::Vector3f common;
        float angle = 0;

        int objectSize = objects.size();
        for (int i = 0; i < objectSize; i++){
            int transformSize = 0;
            transformSize = objects[i]->objTransformations.size();

            objects[i]->transformationMatrix = new glm::mat4(1.0);
            glm::mat4* glmModel = objects[i]->transformationMatrix;

            for (int j = transformSize-1; j >= 0; j--){
                type = objects[i]->objTransformations[j]->type;
                transformIndex = objects[i]->objTransformations[j]->id;

                if (type == TransformationType::Translation){
                    common = translations[transformIndex-1]->common;
                    glmCommon = {common[0], common[1], common[2]};
                    *glmModel = glm::translate(*glmModel, glmCommon);
                }
                else if (type == TransformationType::Scaling){
                    common = scalings[transformIndex-1]->common;
                    glmCommon = {common[0], common[1], common[2]};
                    *glmModel = glm::scale(*glmModel, glmCommon);
                }
                else if (type == TransformationType::Rotation){
                    common = rotations[transformIndex-1]->common;
                    glmCommon = {common[0], common[1], common[2]};
                    angle = rotations[transformIndex-1]->angle;
                    *glmModel = glm::rotate(*glmModel, glm::radians(angle), glmCommon);
                }
            }

            objects[i]->inverse_tMatrix = new glm::mat4(1.0);
            *objects[i]->inverse_tMatrix = glm::inverse(*glmModel);
        }

        int instanceSize = instances.size();
        for (int i = 0; i < instanceSize; i++){
            instances[i]->transformationMatrix = new glm::mat4(1.0);
            glm::mat4* glmModel = instances[i]->transformationMatrix;

            int transformSize = instances[i]->objTransformations.size();
            for (int j = transformSize-1; j >= 0; j--){
                type = instances[i]->objTransformations[j]->type;
                transformIndex = instances[i]->objTransformations[j]->id;

                if (type == TransformationType::Translation){
                    common = translations[transformIndex-1]->common;
                    glmCommon = {common[0], common[1], common[2]};
                    *glmModel = glm::translate(*glmModel, glmCommon);
                }
                else if (type == TransformationType::Scaling){
                    common = scalings[transformIndex-1]->common;
                    glmCommon = {common[0], common[1], common[2]};
                    *glmModel = glm::scale(*glmModel, glmCommon);
                }
                else if (type == TransformationType::Rotation){
                    common = rotations[transformIndex-1]->common;
                    glmCommon = {common[0], common[1], common[2]};
                    angle = rotations[transformIndex-1]->angle;
                    *glmModel = glm::rotate(*glmModel, glm::radians(angle), glmCommon);
                }
            }

            if (!instances[i]->resetTransform){
                *glmModel = (*glmModel) * (*instances[i]->baseMesh->transformationMatrix);
            }

            instances[i]->inverse_tMatrix = new glm::mat4(1.0);
            *instances[i]->inverse_tMatrix = glm::inverse(*glmModel);
        }
    }
}

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

    int FindMinOfThree(int a, int b, int c)
    {
        if (a <= b && a <= c){
            return a;
        }
        else if (b <= a && b <= c){
            return b;
        }

        return c;
    }

    int FindMaxOfThree(int a, int b, int c)
    {
        if (a >= b && a >= c){
            return a;
        }
        else if (b >= a && b >= c){
            return b;
        }

        return c;
    }

    int VectorFindMin(std::vector<Triangle> &indices){
        int minimum = indices[0].GetIndexOne();
        int size = indices.size();

        for (int i = 0; i < size; i++)
        {
            if (indices[i].GetIndexOne() < minimum){
                minimum = indices[i].GetIndexOne();
            }
            if (indices[i].GetIndexTwo() < minimum){
                minimum = indices[i].GetIndexTwo();
            }
            if (indices[i].GetIndexThree() < minimum){
                minimum = indices[i].GetIndexThree();
            }
        }

        return minimum;
    }

    int VectorFindMax(std::vector<Triangle> &indices){
        int maximum = indices[0].GetIndexOne();
        int size = indices.size();

        for (int i = 0; i < size; i++)
        {
            if (indices[i].GetIndexOne() > maximum){
                maximum = indices[i].GetIndexOne();
            }
            if (indices[i].GetIndexTwo() > maximum){
                maximum = indices[i].GetIndexTwo();
            }
            if (indices[i].GetIndexThree() > maximum){
                maximum = indices[i].GetIndexThree();
            }
        }

        return maximum;
    }
}