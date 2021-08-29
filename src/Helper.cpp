#include "Helper.h"
#include "BVH.h"
#include "Instance.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

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
        glm::mat4 inverseTransformations;
        glm::mat4 inverseTranspose;
        glm::vec3 blur;

        if (isNaN(ray.origin) || isNaN(ray.direction)){
            return nearestRet;
        }

        int objectSize = objects.size();
        for (int i = 0; i < objectSize; i++){
            inverseTransformations = *objects[i]->inverse_tMatrix;
            blur = objects[i]->blurTransformation;

            transformedRay = Transforming::TransformRay(ray, inverseTransformations, blur);
            ret = objects[i]->bvh->FindIntersection(transformedRay);
            if (ret.full)
            {
                distance = transformedRay.gett(ret.point);

                if (distance < nearestDistance && distance > 0)
                {
                    nearestDistance = distance;
                    inverseTranspose = *objects[i]->inverseTranspose_tMatrix;
                    ret.point = ray.getPoint(distance);
                    nearestRet = ret;
                }
            }
        }

        int instanceSize = instances.size();
        for (int i = 0; i < instanceSize; i++){
            inverseTransformations = *instances[i]->inverse_tMatrix;
            blur = instances[i]->blurTransformation;

            transformedRay = Transforming::TransformRay(ray, inverseTransformations, blur);
            ret = instances[i]->baseMesh->bvh->FindIntersection(transformedRay);
            if (ret.full)
            {
                distance = transformedRay.gett(ret.point);

                if (distance < nearestDistance && distance > 0)
                {
                    nearestDistance = distance;
                    inverseTranspose = *instances[i]->inverseTranspose_tMatrix;
                    ret.matIndex = instances[i]->matIndex;
                    ret.point = ray.getPoint(distance);
                    nearestRet = ret;
                }
            }
        }

        if (nearestRet.full){
            nearestRet.normal = Transforming::TransformNormal(nearestRet.normal, inverseTranspose);
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
        glmTransformedNormal = tMatrix * glmNormal;

        return Eigen::Vector3f{glmTransformedNormal[0], glmTransformedNormal[1], glmTransformedNormal[2]}.normalized();
    }

    Ray TransformRay(const Ray& ray, glm::mat4 &tMatrix, glm::vec3 &blur){
        Ray transformedRay(ray.time);
        blur = blur * ray.time;
        transformedRay.origin = ray.origin;
        transformedRay.direction = ray.direction;
        transformedRay.origin[0] -= blur[0];
        transformedRay.origin[1] -= blur[1];
        transformedRay.origin[2] -= blur[2];

        glm::vec4 glmOrigin;
        glm::vec4 glmDirection;
        glmOrigin = {transformedRay.origin[0], transformedRay.origin[1], transformedRay.origin[2], 1};
        glmDirection = {transformedRay.direction[0], transformedRay.direction[1], transformedRay.direction[2], 0};

        glm::vec3 glmTransformedOrigin;
        glm::vec3 glmTransformedDirection;
        glmTransformedOrigin = tMatrix * glmOrigin;
        glmTransformedDirection = tMatrix * glmDirection;

        transformedRay.origin = {glmTransformedOrigin[0], glmTransformedOrigin[1], glmTransformedOrigin[2]};
        transformedRay.direction = {glmTransformedDirection[0], glmTransformedDirection[1], glmTransformedDirection[2]};

        return transformedRay;
    }

    void ComputeObjectTransformations(std::vector<Shape*> &objects, std::vector<Instance*> instances,
            std::vector<Transformation*> &translations, std::vector<Transformation*> &scalings,
            std::vector<Transformation*> &rotations, std::vector<Transformation*> &composites){

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
                else if (type == TransformationType::Composite){
                    *glmModel = composites[transformIndex-1]->composite;
                }
            }

            objects[i]->inverse_tMatrix = new glm::mat4(1.0);
            *objects[i]->inverse_tMatrix = glm::inverse(*glmModel);

            objects[i]->inverseTranspose_tMatrix = new glm::mat4(1.0);
            *objects[i]->inverseTranspose_tMatrix = glm::inverseTranspose(*glmModel);
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
                else if (type == TransformationType::Composite){
                    *glmModel = composites[transformIndex-1]->composite;
                }
            }

            if (!instances[i]->resetTransform){
                *glmModel = (*glmModel) * (*instances[i]->baseMesh->transformationMatrix);
            }

            instances[i]->inverse_tMatrix = new glm::mat4(1.0);
            *instances[i]->inverse_tMatrix = glm::inverse(*glmModel);

            instances[i]->inverseTranspose_tMatrix = new glm::mat4(1.0);
            *instances[i]->inverseTranspose_tMatrix = glm::inverseTranspose(*glmModel);
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

namespace GeometryHelpers
{
    int GetAbsSmallestIndex(Eigen::Vector3f &vector){
        float compZero = abs(vector[0]);
        float compOne = abs(vector[1]);
        float compTwo = abs(vector[2]);

        if (compZero <= compOne && compZero <= compTwo){
            return 0;
        }
        else if (compOne <= compZero && compOne <= compTwo){
            return 1;
        }

        return 2;
    }

    Eigen::Vector3f GetOrthonormalUVector(const Eigen::Vector3f &vector){
        Eigen::Vector3f nonLinear = vector;
        nonLinear[GetAbsSmallestIndex(nonLinear)] = 1;

        return (vector.cross(nonLinear)).normalized();
    }
}

namespace ExrLibrary{
    Eigen::Vector2f ReadExr(const char *filename, float*& data){
        const char* err = nullptr;
        int width, height;
        int ret = LoadEXR(&data, &width, &height, filename, &err);

        if (ret != TINYEXR_SUCCESS) {
            if (err) {
                fprintf(stderr, "ERR : %s\n", err);
                FreeEXRErrorMessage(err); // release memory of error message.
            }
        }

        return {width, height};
    }

    void SaveExr(const char *filename, float* data, int width, int height){
        const char* err = nullptr;

        EXRHeader header;
        InitEXRHeader(&header);

        EXRImage image;
        InitEXRImage(&image);
        image.num_channels = 3;

        std::vector<float> images[3];
        images[0].resize(width * height);
        images[1].resize(width * height);
        images[2].resize(width * height);

        // Split RGBRGBRGB... into R, G and B layer
        for (int i = 0; i < width * height; i++) {
            images[0][i] = data[3*i+0];
            images[1][i] = data[3*i+1];
            images[2][i] = data[3*i+2];
        }

        float* image_ptr[3];
        image_ptr[0] = &(images[2].at(0)); // B
        image_ptr[1] = &(images[1].at(0)); // G
        image_ptr[2] = &(images[0].at(0)); // R

        image.images = (unsigned char**)image_ptr;
        image.width = width;
        image.height = height;

        header.num_channels = 3;
        header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
        // Must be (A)BGR order, since most of EXR viewers expect this channel order.
        strncpy(header.channels[0].name, "B", 255); header.channels[0].name[strlen("B")] = '\0';
        strncpy(header.channels[1].name, "G", 255); header.channels[1].name[strlen("G")] = '\0';
        strncpy(header.channels[2].name, "R", 255); header.channels[2].name[strlen("R")] = '\0';

        header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
        header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
        for (int i = 0; i < header.num_channels; i++) {
            header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
            header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
        }

        int ret = SaveEXRImageToFile(&image, &header, filename, &err);
        if (ret != TINYEXR_SUCCESS) {
            fprintf(stderr, "Save EXR err: %s\n", err);
            FreeEXRErrorMessage(err); // free's buffer for an error message
            return;
        }
    }
}