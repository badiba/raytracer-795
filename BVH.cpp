#include "BVH.h"
#include "defs.h"
#include "Scene.h"
#include <math.h>

using namespace Eigen;

namespace BVHHelpers
{
	float FindMinOfThree(float a, float b, float c)
	{
		if (a <= b && a <= c)
		{
			return a;
		}
		else if (b <= a && b <= c)
		{
			return b;
		}

		return c;
	}

	float FindMaxOfThree(float a, float b, float c)
	{
		if (a >= b && a >= c)
		{
			return a;
		}
		else if (b >= a && b >= c)
		{
			return b;
		}

		return c;
	}
}

BVH::BVH()
{
	root = nullptr;
	bvhMaxRecursionDepth = 30;

	// Traverse all objects and fill primitives into a vector.
	for (int i = 0; i < pScene->objects.size(); i++)
	{
		pScene->objects[i]->FillPrimitives(primitives);
	}

	ConstructionHelper(0, primitives.size(), 0, root, 0);
}

BVH::BVH(Shape* object){
    root = nullptr;
    bvhMaxRecursionDepth = 30;

    object->FillPrimitives(primitives);
    textures = object->textures;
    textureOffset = object->textureOffset;

    ConstructionHelper(0, primitives.size(), 0, root, 0);
}

void BVH::ConstructionHelper(int startIndex, int endIndex, int splitType, BTNode<BBox>*& node, int recursionDepth)
{
	// Stopping condition.
	if (startIndex == endIndex - 1 || recursionDepth >= bvhMaxRecursionDepth)
	{
		BBox box;
		box.startIndex = startIndex;
		box.endIndex = endIndex;
		node = new BTNode<BBox>(box, nullptr, nullptr);
		return;
	}

	if (startIndex == endIndex)
	{
		return;
	}

	// splitType = 0 means split by X. splitType = 2 means split by Z.
	// splitType is increased at every call. Therefore if it is bigger
	// than 2, then set it back to 0 which is split by X.
	if (splitType > 2)
	{
		splitType = 0;
	}

	BBox box = ComputeBoundingBox(startIndex, endIndex);
	node = new BTNode<BBox>(box, nullptr, nullptr);

	int swapIndex = startIndex;
	float split = FindMedian(startIndex, endIndex, splitType);

	for (int i = startIndex; i < endIndex; i++)
	{
		float center = primitives[i]->GetCenter()[splitType];
		if (center < split)
		{
			Shape* tmp = primitives[swapIndex];
			primitives[swapIndex] = primitives[i];
			primitives[i] = tmp;

			swapIndex++;
		}
	}

	ConstructionHelper(startIndex, swapIndex, splitType + 1, node->left, recursionDepth + 1);
	ConstructionHelper(swapIndex, endIndex, splitType + 1, node->right, recursionDepth + 1);
}

ReturnVal BVH::FindIntersection(const Ray& ray)
{
	return FindIntersectionWithBVH(ray, root);
}

float BVH::FindMedian(int startIndex, int endIndex, int coordinate)
{
	std::vector<float> centers;
	for (int i = startIndex; i < endIndex; i++)
	{
		centers.push_back(primitives[i]->GetCenter()[coordinate]);
	}

	std::sort(centers.begin(), centers.end());

	int length = endIndex - startIndex;
	int medianIndex = length / 2;
	if (length % 2 == 0){
		return (centers[medianIndex-1] + centers[medianIndex]) * 0.5f;
	}
	else{
		return centers[medianIndex];
	}
}

ReturnVal BVH::FindIntersectionWithBVH(const Ray& ray, BTNode<BBox>* node)
{
	// Empty
	if (!node)
	{
		ReturnVal ret;
		ret.full = false;
		return ret;
	}

	// Leaf
	if (!node->left && !node->right)
	{
		int startIndex = node->data.startIndex;
		int endIndex = node->data.endIndex;

		ReturnVal ret;
		ReturnVal nearestRet;
		float returnDistance = 0;
		float nearestPoint = std::numeric_limits<float>::max();

		// Check intersection of the ray with all objects in the bounding box.
		for (int i = startIndex; i < endIndex; i++)
		{
			ret = primitives[i]->bvhIntersect(ray, textures, textureOffset);
			if (ret.full)
			{
				// Save the nearest intersected object.
				returnDistance = (ret.point - ray.origin).norm();
				if (returnDistance < nearestPoint)
				{
					nearestPoint = returnDistance;
					nearestRet = ret;
					nearestRet.matIndex = primitives[i]->matIndex;
				}
			}
		}

		return nearestRet;
	}

	if (RayBBoxIntersection(ray, node->data))
	{
		ReturnVal retLeft = FindIntersectionWithBVH(ray, node->left);
		ReturnVal retRight = FindIntersectionWithBVH(ray, node->right);

		if (retLeft.full && !retRight.full)
		{
			return retLeft;
		}
		else if (!retLeft.full && retRight.full)
		{
			return retRight;
		}
		else if (retLeft.full && retRight.full)
		{
			float leftDistance = (retLeft.point - ray.origin).norm();
			float rightDistance = (retRight.point - ray.origin).norm();

			if (leftDistance < rightDistance)
			{
				return retLeft;
			}
			else
			{
				return retRight;
			}
		}
	}

	ReturnVal ret;
	ret.full = false;
	return ret;
}

bool BVH::RayBBoxIntersection(const Ray& ray, BBox box)
{
	float tx_e;
	float tx_l;
	float ty_e;
	float ty_l;
	float tz_e;
	float tz_l;
	float dx = ray.direction[0];
	float dy = ray.direction[1];
	float dz = ray.direction[2];

	if (dx > 0)
	{
		tx_e = (box.minPoint[0] - ray.origin[0]) / ray.direction[0];
		tx_l = (box.maxPoint[0] - ray.origin[0]) / ray.direction[0];
	}
	else
	{
		tx_e = (box.maxPoint[0] - ray.origin[0]) / ray.direction[0];
		tx_l = (box.minPoint[0] - ray.origin[0]) / ray.direction[0];
	}

	if (dy > 0)
	{
		ty_e = (box.minPoint[1] - ray.origin[1]) / ray.direction[1];
		ty_l = (box.maxPoint[1] - ray.origin[1]) / ray.direction[1];
	}
	else
	{
		ty_e = (box.maxPoint[1] - ray.origin[1]) / ray.direction[1];
		ty_l = (box.minPoint[1] - ray.origin[1]) / ray.direction[1];
	}

	if (dz > 0)
	{
		tz_e = (box.minPoint[2] - ray.origin[2]) / ray.direction[2];
		tz_l = (box.maxPoint[2] - ray.origin[2]) / ray.direction[2];
	}
	else
	{
		tz_e = (box.maxPoint[2] - ray.origin[2]) / ray.direction[2];
		tz_l = (box.minPoint[2] - ray.origin[2]) / ray.direction[2];
	}

	float tSmallestL = BVHHelpers::FindMinOfThree(tx_l, ty_l, tz_l);
	float tLargestE = BVHHelpers::FindMaxOfThree(tx_e, ty_e, tz_e);

	if (tSmallestL < tLargestE)
	{
		return false;
	}

	return true;
}

BBox BVH::ComputeBoundingBox(int startIndex, int endIndex)
{
	float _min = -std::numeric_limits<float>::max();
	float _max = std::numeric_limits<float>::max();

	BBox box = { Vector3f{ _max, _max, _max },
			Vector3f{ _min, _min, _min }};

	for (int i = startIndex; i < endIndex; i++)
	{
		BBox checkBox = primitives[i]->GetBoundingBox();
		box = MergeBBoxes(box, checkBox);
	}

	return box;
}

Eigen::Vector3f BVH::FindMinPointOfTwo(Eigen::Vector3f v1, Eigen::Vector3f v2)
{
	return Eigen::Vector3f{ FindMinOfTwo(v1[0], v2[0]),
			FindMinOfTwo(v1[1], v2[1]),
			FindMinOfTwo(v1[2], v2[2]) };
}

Eigen::Vector3f BVH::FindMaxPointOfTwo(Eigen::Vector3f v1, Eigen::Vector3f v2)
{
	return Eigen::Vector3f{ FindMaxOfTwo(v1[0], v2[0]),
			FindMaxOfTwo(v1[1], v2[1]),
			FindMaxOfTwo(v1[2], v2[2]) };
}

BBox BVH::MergeBBoxes(BBox boxOne, BBox boxTwo)
{
	return BBox{ FindMinPointOfTwo(boxOne.minPoint, boxTwo.minPoint),
			FindMaxPointOfTwo(boxOne.maxPoint, boxTwo.maxPoint) };
}

float BVH::FindMinOfTwo(float a, float b)
{
	if (a <= b)
	{
		return a;
	}

	return b;
}

float BVH::FindMaxOfTwo(float a, float b)
{
	if (a >= b)
	{
		return a;
	}

	return b;
}

BTNode<BBox>* BVH::GetRoot()
{
	return root;
}

void BVH::DebugBVH()
{

}