#include "Transformation.h"

Transformation::Transformation(void)
{
}

Transformation::Transformation(int id, TransformationType type){
    this->id = id;
    this->type = type;
    this->angle = 0;
}