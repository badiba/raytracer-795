#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "Transformation.h"
#include "glm/gtx/string_cast.hpp"
#include "Shape.h"
#include <vector>

class Instance{
public:
    int id;
    int matIndex;
    bool resetTransform;

    Shape* baseMesh;
    glm::mat4* transformationMatrix;
    std::vector<Transformation*> objTransformations;

    Instance();
    Instance(int id, Shape* baseMesh, int matIndex, bool resetTransform, const std::vector<Transformation*>& transformations);
};

#endif
