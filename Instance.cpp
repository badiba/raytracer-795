#include "Instance.h"

Instance::Instance(int id, Shape* baseMesh, int matIndex, bool resetTransform, const std::vector<Transformation*>& transformations){
    this->id = id;
    this->baseMesh = baseMesh;
    this->matIndex = matIndex;
    this->resetTransform = resetTransform;
    this->objTransformations = transformations;
}

Instance::Instance(){

}