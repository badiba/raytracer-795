#ifndef _TRANSFORMATION_H_
#define _TRANSFORMATION_H_

#include "Eigen/Dense"
#include "glm/gtx/string_cast.hpp"

enum TransformationType{None, Translation, Scaling, Rotation, Composite};

class Transformation{
public:
    int id;
    TransformationType type;

    float angle;
    Eigen::Vector3f common;
    glm::mat4 composite;

    Transformation(void);
    Transformation(int id, TransformationType type);
};

#endif
