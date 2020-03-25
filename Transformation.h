#ifndef _TRANSFORMATION_H_
#define _TRANSFORMATION_H_
#include "Eigen/Dense"

enum TransformationType{None, Translation, Scaling, Rotation};

class Transformation{
public:
    int id;
    TransformationType type;

    float angle;
    Eigen::Vector3f common;

    Transformation(void);
};


#endif
