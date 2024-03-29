#ifndef _BVH_H_
#define _BVH_H_

#include "BTNode.h"
#include "Shape.h"
#include <iostream>

class BVH
{
public:

	ReturnVal FindIntersection(const Ray& ray);
	BTNode<BBox>* GetRoot();
	void DebugBVH();
	BVH();
	BVH(Shape* object);

private:
    std::vector<int> textures;
    int textureOffset;
	std::vector<Shape *> primitives;
	BTNode<BBox> *root;
	int bvhMaxRecursionDepth;

	ReturnVal FindIntersectionWithBVH(const Ray& ray, BTNode<BBox>* node);
	bool RayBBoxIntersection(const Ray& ray, BBox box);
	void ConstructionHelper(int startIndex, int endIndex, int splitType, BTNode<BBox>*& node, int recursionDepth);
	BBox ComputeBoundingBox(int startIndex, int endIndex);
	BBox MergeBBoxes(BBox boxOne, BBox boxTwo);
	Eigen::Vector3f FindMinPointOfTwo(Eigen::Vector3f v1, Eigen::Vector3f v2);
	Eigen::Vector3f FindMaxPointOfTwo(Eigen::Vector3f v1, Eigen::Vector3f v2);
	float FindMinOfTwo(float a, float b);
	float FindMaxOfTwo(float a, float b);
	float FindMedian(int startIndex, int endIndex, int coordinate);
};

#endif

