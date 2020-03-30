#include "Helper.h"

namespace Transforming{
    void ApplyTransformations(std::vector<Eigen::Vector3f> &vertices, std::vector<Shape*> &objects,
                              std::vector<Transformation*> &translations,
                              std::vector<Transformation*> &scalings,
                              std::vector<Transformation*> &rotations){
        int objectSize = objects.size();

        int transformIndex = 0;
        TransformationType type;
        glm::vec3 glmCommon;
        glm::vec3 glmTransformed;
        Eigen::Vector3f common;
        float angle = 0;

        for (int i = 0; i < objectSize; i++){
            int transformSize = 0;
            glm::mat4 glmModel(1.0f);

            transformSize = objects[i]->objTransformations.size();
            std::cout << "object index: " << i << std::endl;

            for (int j = transformSize-1; j >= 0; j--){
                type = objects[i]->objTransformations[j]->type;
                transformIndex = objects[i]->objTransformations[j]->id;
                std::cout << "transform index: " << j << " type: " << type << std::endl;

                if (type == TransformationType::Translation){
                    common = translations[transformIndex-1]->common;
                    glmCommon = {common[0], common[1], common[2]};
                    glmModel = glm::translate(glmModel, glmCommon);
                    std::cout << "common: " << common << std::endl;
                }
                else if (type == TransformationType::Scaling){
                    common = scalings[transformIndex-1]->common;
                    glmCommon = {common[0], common[1], common[2]};
                    glmModel = glm::scale(glmModel, glmCommon);
                    std::cout << "common: " << common << std::endl;
                }
                else if (type == TransformationType::Rotation){
                    common = rotations[transformIndex-1]->common;
                    glmCommon = {common[0], common[1], common[2]};
                    angle = rotations[transformIndex-1]->angle;
                    glmModel = glm::rotate(glmModel, glm::radians(angle), glmCommon); // IS THIS ANGLE IN RADIANS?
                    std::cout << "common: " << common << std::endl;
                    std::cout << "angle: " << angle << std::endl;
                }
            }

            std::cout << glm::to_string(glmModel) << std::endl;

            if (transformSize <= 0){
                continue;
            }
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