#include "defs.h"
#include "Scene.h"
#include "BVH.h"

Scene* pScene; // definition of the global scene variable (declared in defs.h)

int main(int argc, char* argv[])
{
	const char* xmlPath = argv[1];
	pScene = new Scene(xmlPath);

	//BVH *bvh = new BVH();

	//return 0; // TESTING

	pScene->renderScene();
	return 0;
}

