#include "Instance.h"

Instance::Instance(int id, Shape* baseMesh, int matIndex, bool resetTransform, const std::vector<Transformation*>& transformations,
                   glm::vec3 &blurTransformation, bool isBlur){
    this->id = id;
    this->baseMesh = baseMesh;
    this->matIndex = matIndex;
    this->resetTransform = resetTransform;
    this->objTransformations = transformations;
    this->blurTransformation = blurTransformation;
    this->isBlur = isBlur;
}

Instance::Instance(){

}