#ifndef _PARSER_H_
#define _PARSER_H_

#include "Instance.h"

using namespace tinyxml2;
using namespace Eigen;

namespace Parser{
    void ParseSceneAttributes(XMLNode* pRoot, int &maxRecursionDepth, Vector3f &backgroundColor, float &shadowRayEps,
            float &intTestEps){
        const char* str;
        XMLError eResult;
        XMLElement* pElement;

        maxRecursionDepth = 1;
        shadowRayEps = 0.002;
        intTestEps = 0.001;

        pElement = pRoot->FirstChildElement("MaxRecursionDepth");
        if (pElement != nullptr)
        {
            pElement->QueryIntText(&maxRecursionDepth);
        }

        pElement = pRoot->FirstChildElement("BackgroundColor");
        str = pElement->GetText();
        sscanf(str, "%f %f %f", &backgroundColor(0), &backgroundColor(1), &backgroundColor(2));

        pElement = pRoot->FirstChildElement("ShadowRayEpsilon");
        if (pElement != nullptr)
        {
            pElement->QueryFloatText(&shadowRayEps);
        }

        pElement = pRoot->FirstChildElement("IntersectionTestEpsilon");
        if (pElement != nullptr)
        {
            eResult = pElement->QueryFloatText(&intTestEps);
        }
    }

    void ParseCameras(XMLNode* pRoot, std::vector<Camera*> &cameras){
        const char* str;
        XMLError eResult;

        XMLElement* pElement = pRoot->FirstChildElement("Cameras");
        XMLElement* pCamera = pElement->FirstChildElement("Camera");
        XMLElement* camElement;
        while (pCamera != nullptr)
        {
            int id, numSamples;
            char imageName[64];
            Vector3f pos, gaze, up;
            ImagePlane imgPlane;

            eResult = pCamera->QueryIntAttribute("id", &id);

            // Parse NumSamples
            numSamples = 1;
            camElement = pCamera->FirstChildElement("NumSamples");
            if(camElement){
                eResult = camElement->QueryIntText(&numSamples);
            }

            camElement = pCamera->FirstChildElement("Position");
            str = camElement->GetText();
            sscanf(str, "%f %f %f", &pos(0), &pos(1), &pos(2));

            // Parse Gaze
            camElement = pCamera->FirstChildElement("Gaze");
            if (camElement)
            {
                str = camElement->GetText();
                sscanf(str, "%f %f %f", &gaze(0), &gaze(1), &gaze(2));
            }
            camElement = pCamera->FirstChildElement("GazePoint");
            if (camElement)
            {
                str = camElement->GetText();
                Vector3f gazePoint;
                sscanf(str, "%f %f %f", &gazePoint[0], &gazePoint[1], &gazePoint[2]);
                gaze = gazePoint - pos;
            }

            camElement = pCamera->FirstChildElement("Up");
            str = camElement->GetText();
            sscanf(str, "%f %f %f", &up(0), &up(1), &up(2));
            camElement = pCamera->FirstChildElement("NearDistance");
            eResult = camElement->QueryFloatText(&imgPlane.distance);
            camElement = pCamera->FirstChildElement("ImageResolution");
            str = camElement->GetText();
            sscanf(str, "%d %d", &imgPlane.nx, &imgPlane.ny);
            camElement = pCamera->FirstChildElement("ImageName");
            str = camElement->GetText();
            strcpy(imageName, str);

            // Parse near plane.
            camElement = pCamera->FirstChildElement("NearPlane");
            if (camElement)
            {
                str = camElement->GetText();
                sscanf(str, "%f %f %f %f", &imgPlane.left, &imgPlane.right, &imgPlane.bottom, &imgPlane.top);
            }
            camElement = pCamera->FirstChildElement("FovY");
            if (camElement)
            {
                float fov;
                eResult = camElement->QueryFloatText(&fov);
                fov = (fov * 0.5f) * (M_PI / 180.0f);
                float aspectRatio = (float)imgPlane.nx / (float)imgPlane.ny;
                float y = tan(fov) * imgPlane.distance;
                float x = aspectRatio * y;

                imgPlane.top = y;
                imgPlane.bottom = -y;
                imgPlane.left = -x;
                imgPlane.right = x;
            }

            cameras.push_back(new Camera(id, numSamples, imageName, pos, gaze, up, imgPlane));

            pCamera = pCamera->NextSiblingElement("Camera");
        }
    }

    void ParseMaterials(XMLNode* pRoot, std::vector<Material*> &materials){
        const char* str;
        XMLError eResult;

        XMLElement* pElement = pRoot->FirstChildElement("Materials");
        XMLElement* pMaterial = pElement->FirstChildElement("Material");
        XMLElement* materialElement;
        while (pMaterial != nullptr)
        {
            materials.push_back(new Material());

            int curr = materials.size() - 1;

            eResult = pMaterial->QueryIntAttribute("id", &materials[curr]->id);
            materialElement = pMaterial->FirstChildElement("AmbientReflectance");
            str = materialElement->GetText();
            sscanf(str, "%f %f %f", &materials[curr]->ambientRef(0), &materials[curr]->ambientRef(1),
                   &materials[curr]->ambientRef(2));
            materialElement = pMaterial->FirstChildElement("DiffuseReflectance");
            str = materialElement->GetText();
            sscanf(str, "%f %f %f", &materials[curr]->diffuseRef(0), &materials[curr]->diffuseRef(1),
                   &materials[curr]->diffuseRef(2));
            materialElement = pMaterial->FirstChildElement("SpecularReflectance");
            str = materialElement->GetText();
            sscanf(str, "%f %f %f", &materials[curr]->specularRef(0), &materials[curr]->specularRef(1),
                   &materials[curr]->specularRef(2));

            // Parse mirrors.
            materialElement = pMaterial->FirstChildElement("MirrorReflectance");
            if (materialElement != nullptr)
            {
                str = materialElement->GetText();
                sscanf(str, "%f %f %f", &materials[curr]->mirrorRef(0), &materials[curr]->mirrorRef(1),
                       &materials[curr]->mirrorRef(2));
            }
            else
            {
                materials[curr]->mirrorRef(0) = 0.0;
                materials[curr]->mirrorRef(1) = 0.0;
                materials[curr]->mirrorRef(2) = 0.0;
            }

            // Parse PhongExponent.
            materialElement = pMaterial->FirstChildElement("PhongExponent");
            if (materialElement != nullptr)
            {
                materialElement->QueryIntText(&materials[curr]->phongExp);
            }

            // Parse type, RefractionIndex, AbsorptionIndex, AbsorptionCoefficient.
            const XMLAttribute* attr = pMaterial->FirstAttribute();

            materials[curr]->type = Normal;
            while (attr != nullptr)
            {
                if (std::strncmp(attr->Name(), "type", 4) != 0)
                {
                    attr = attr->Next();
                    continue;
                }

                if (std::strncmp(attr->Value(), "dielectric", 10) == 0)
                {
                    materials[curr]->type = Dielectric;
                }
                else if (std::strncmp(attr->Value(), "conductor", 9) == 0)
                {
                    materials[curr]->type = Conductor;
                }
                else if (std::strncmp(attr->Value(), "mirror", 6) == 0)
                {
                    materials[curr]->type = Mirror;
                }
                else
                {
                    materials[curr]->type = Normal;
                }
                break;
            }

            materialElement = pMaterial->FirstChildElement("RefractionIndex");
            if (materialElement != nullptr)
            {
                materialElement->QueryFloatText(&materials[curr]->refractionIndex);
            }
            else
            {
                materials[curr]->refractionIndex = 0;
            }

            materialElement = pMaterial->FirstChildElement("AbsorptionIndex");
            if (materialElement != nullptr)
            {
                materialElement->QueryFloatText(&materials[curr]->absorptionIndex);
            }
            else
            {
                materials[curr]->absorptionIndex = 0;
            }

            materialElement = pMaterial->FirstChildElement("AbsorptionCoefficient");
            if (materialElement != nullptr)
            {
                str = materialElement->GetText();
                sscanf(str, "%f %f %f", &materials[curr]->absorptionCoefficient(0),
                       &materials[curr]->absorptionCoefficient(1),
                       &materials[curr]->absorptionCoefficient(2));
            }
            else
            {
                materials[curr]->absorptionCoefficient(0) = 0.0;
                materials[curr]->absorptionCoefficient(1) = 0.0;
                materials[curr]->absorptionCoefficient(2) = 0.0;
            }

            // Move onto the next element.
            pMaterial = pMaterial->NextSiblingElement("Material");
        }
    }

    void ParseTransformations(XMLNode* pRoot, std::vector<Transformation*> &translations,
                              std::vector<Transformation*> &scalings, std::vector<Transformation*> &rotations){
        const char* str;
        XMLError eResult;

        XMLElement* pElement = pRoot->FirstChildElement("Transformations");
        if (pElement != nullptr){
            // Parse translations.
            XMLElement* pTranslation = pElement->FirstChildElement("Translation");
            while(pTranslation != nullptr){
                translations.push_back(new Transformation());

                int curr = translations.size() - 1;
                eResult = pTranslation->QueryIntAttribute("id", &translations[curr]->id);

                str = pTranslation->GetText();
                sscanf(str, "%f %f %f", &translations[curr]->common[0], &translations[curr]->common[1],
                       &translations[curr]->common[2]);

                pTranslation = pTranslation->NextSiblingElement("Translation");
            }

            // Parse scalings.
            XMLElement* pScaling = pElement->FirstChildElement("Scaling");
            while(pScaling != nullptr){
                scalings.push_back(new Transformation());

                int curr = scalings.size() - 1;
                eResult = pScaling->QueryIntAttribute("id", &scalings[curr]->id);

                str = pScaling->GetText();
                sscanf(str, "%f %f %f", &scalings[curr]->common[0], &scalings[curr]->common[1],
                       &scalings[curr]->common[2]);

                pScaling = pScaling->NextSiblingElement("Scaling");
            }

            // Parse rotations.
            XMLElement* pRotation = pElement->FirstChildElement("Rotation");
            while(pRotation != nullptr){
                rotations.push_back(new Transformation());

                int curr = rotations.size() - 1;
                eResult = pRotation->QueryIntAttribute("id", &rotations[curr]->id);

                str = pRotation->GetText();
                sscanf(str, "%f %f %f %f", &rotations[curr]->angle, &rotations[curr]->common[0], &rotations[curr]->common[1],
                       &rotations[curr]->common[2]);

                pRotation = pRotation->NextSiblingElement("Rotation");
            }
        }
    }

    void ParseVertices(XMLNode* pRoot, std::vector<Vector3f> &vertices){
        const char* str;
        XMLError eResult;

        XMLElement* pElement = pRoot->FirstChildElement("VertexData");
        int cursor = 0;
        Vector3f tmpPoint;
        str = pElement->GetText();
        while (str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
        {
            cursor++;
        }
        while (str[cursor] != '\0')
        {
            for (int cnt = 0; cnt < 3; cnt++)
            {
                if (cnt == 0)
                {
                    tmpPoint(0) = atof(str + cursor);
                }
                else if (cnt == 1)
                {
                    tmpPoint(1) = atof(str + cursor);
                }
                else
                {
                    tmpPoint(2) = atof(str + cursor);
                }
                while (str[cursor] != ' ' && str[cursor] != '\t' && str[cursor] != '\n')
                {
                    cursor++;
                }
                while (str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
                {
                    cursor++;
                }
            }
            vertices.push_back(tmpPoint);
        }
    }

    void ParseObjectTransformations(const char* str, std::vector<Transformation*> &transformations){
        int transformationIndex = 0;
        int cursor = 0;

        while(str[cursor] != '\0'){
            if (str[cursor] == 't'){
                transformationIndex = atoi(str + cursor + 1);
                transformations.push_back(new Transformation(transformationIndex, TransformationType::Translation));
            }
            else if (str[cursor] == 's'){
                transformationIndex = atoi(str + cursor + 1);
                transformations.push_back(new Transformation(transformationIndex, TransformationType::Scaling));
            }
            else if (str[cursor] == 'r'){
                transformationIndex = atoi(str + cursor + 1);
                transformations.push_back(new Transformation(transformationIndex, TransformationType::Rotation));
            }

            cursor++;
            while(str[cursor] != 's' && str[cursor] != 't' && str[cursor] != 'r' && str[cursor] != '\0'){
                cursor++;
            }
        }
    }

    void ParseObjects(XMLNode* pRoot, const char* xmlPath, std::vector<Shape*> &objects, std::vector<Instance*> &instances,
            std::vector<Vector3f> &vertices){
        const char* str;
        XMLError eResult;

        XMLElement* pElement = pRoot->FirstChildElement("Objects");

        // Parse spheres
        std::cout << "- Parsing spheres." << std::endl;
        XMLElement* pObject = pElement->FirstChildElement("Sphere");
        XMLElement* objElement;
        while (pObject != nullptr)
        {
            int id;
            int matIndex;
            int cIndex;
            float R;
            std::vector<Transformation*> transformations;

            eResult = pObject->QueryIntAttribute("id", &id);
            objElement = pObject->FirstChildElement("Material");
            eResult = objElement->QueryIntText(&matIndex);

            // Parse object transformations.
            objElement = pObject->FirstChildElement("Transformations");
            if (objElement != nullptr){
                str = objElement->GetText();
                ParseObjectTransformations(str, transformations);
            }

            objElement = pObject->FirstChildElement("Center");
            eResult = objElement->QueryIntText(&cIndex);
            objElement = pObject->FirstChildElement("Radius");
            eResult = objElement->QueryFloatText(&R);

            objects.push_back(new Sphere(id, matIndex, cIndex, R, transformations));

            pObject = pObject->NextSiblingElement("Sphere");
        }

        // Parse triangles
        std::cout << "- Parsing triangles." << std::endl;
        pObject = pElement->FirstChildElement("Triangle");
        while (pObject != nullptr)
        {
            int id;
            int matIndex;
            int p1Index;
            int p2Index;
            int p3Index;
            std::vector<Transformation*> transformations;

            eResult = pObject->QueryIntAttribute("id", &id);
            objElement = pObject->FirstChildElement("Material");
            eResult = objElement->QueryIntText(&matIndex);

            // Parse object transformations.
            objElement = pObject->FirstChildElement("Transformations");
            if (objElement != nullptr){
                str = objElement->GetText();
                ParseObjectTransformations(str, transformations);
            }

            objElement = pObject->FirstChildElement("Indices");
            str = objElement->GetText();
            sscanf(str, "%d %d %d", &p1Index, &p2Index, &p3Index);

            objects.push_back(new Triangle(id, matIndex, p1Index, p2Index, p3Index, transformations));

            pObject = pObject->NextSiblingElement("Triangle");
        }

        // Parse meshes
        int meshStartIndex = objects.size();
        std::cout << "- Parsing meshes." << std::endl;
        pObject = pElement->FirstChildElement("Mesh");
        while (pObject != nullptr)
        {
            int id, matIndex, p1Index, p2Index, p3Index, cursor, vertexOffset;
            cursor = 0;
            vertexOffset = 0;
            std::vector<Triangle> faces;
            std::vector<Transformation*> transformations;

            eResult = pObject->QueryIntAttribute("id", &id);
            objElement = pObject->FirstChildElement("Material");
            eResult = objElement->QueryIntText(&matIndex);

            // Parse object transformations.
            objElement = pObject->FirstChildElement("Transformations");
            if (objElement != nullptr){
                str = objElement->GetText();
                ParseObjectTransformations(str, transformations);
            }

            objElement = pObject->FirstChildElement("Faces");

            // Parse PLY File ---------> BEGIN.
            bool isPly = false;
            const XMLAttribute* attr = objElement->FirstAttribute();
            while (attr != nullptr)
            {
                if (std::strncmp(attr->Name(), "plyFile", 7) != 0)
                {
                    attr = attr->Next();
                    continue;
                }

                isPly = true;
                break;
            }
            if (isPly)
            {
                // Get path of ply file.
                std::string plyPath = "";
                int lastIndex = 0;
                for (int i = 0; xmlPath[i] != '\0'; i++){
                    if (xmlPath[i] == '/'){
                        lastIndex = i;
                    }
                }
                for (int i = 0; i <= lastIndex; i++){
                    plyPath += xmlPath[i];
                }
                plyPath += attr->Value();

                happly::PLYData plyIn(plyPath);

                std::vector<std::vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();
                int fIndSize = fInd.size();
                int vertexCount = vertices.size() + 1;
                for (int i = 0; i < fIndSize; i++)
                {
                    p1Index = fInd[i][0] + vertexCount;
                    p2Index = fInd[i][1] + vertexCount;
                    p3Index = fInd[i][2] + vertexCount;
                    faces.push_back(*(new Triangle(-1, matIndex, p1Index, p2Index, p3Index)));
                }

                std::vector<std::array<double, 3>> vPos = plyIn.getVertexPositions();
                int vPosSize = vPos.size();
                Vector3f vertex;
                for (int i = 0; i < vPosSize; i++)
                {
                    vertex[0] = vPos[i][0];
                    vertex[1] = vPos[i][1];
                    vertex[2] = vPos[i][2];
                    vertices.push_back(vertex);
                }

                objects.push_back(new Mesh(id, matIndex, faces, transformations));

                pObject = pObject->NextSiblingElement("Mesh");
                continue;
            }
            // Parse PLY File ---------> COMPLETED.

            cursor = 0;
            objElement->QueryIntAttribute("vertexOffset", &vertexOffset);
            str = objElement->GetText();
            while (str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
            {
                cursor++;
            }
            while (str[cursor] != '\0')
            {
                for (int cnt = 0; cnt < 3; cnt++)
                {
                    if (cnt == 0)
                    {
                        p1Index = atoi(str + cursor) + vertexOffset;
                    }
                    else if (cnt == 1)
                    {
                        p2Index = atoi(str + cursor) + vertexOffset;
                    }
                    else
                    {
                        p3Index = atoi(str + cursor) + vertexOffset;
                    }
                    while (str[cursor] != ' ' && str[cursor] != '\t' && str[cursor] != '\n')
                    {
                        cursor++;
                    }
                    while (str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
                    {
                        cursor++;
                    }
                }
                faces.push_back(*(new Triangle(-1, matIndex, p1Index, p2Index, p3Index)));
            }

            objects.push_back(new Mesh(id, matIndex, faces, transformations));

            pObject = pObject->NextSiblingElement("Mesh");
        }

        // Parse mesh instances.
        std::cout << "- Parsing meshes instances." << std::endl;
        pObject = pElement->FirstChildElement("MeshInstance");
        while(pObject != nullptr){
            int id, baseMeshId, matIndex;
            bool resetTransform = false;
            std::vector<Transformation*> transformations;
            Shape* baseMesh;

            eResult = pObject->QueryIntAttribute("id", &id);
            eResult = pObject->QueryIntAttribute("baseMeshId", &baseMeshId);
            eResult = pObject->QueryBoolAttribute("resetTransform", &resetTransform);

            objElement = pObject->FirstChildElement("Material");
            eResult = objElement->QueryIntText(&matIndex);

            // Parse object transformations.
            objElement = pObject->FirstChildElement("Transformations");
            if (objElement != nullptr){
                str = objElement->GetText();
                ParseObjectTransformations(str, transformations);
            }

            int objectSize = objects.size();
            for(int i = meshStartIndex; i < objectSize; i++){
                if (objects[i]->id == baseMeshId){
                    baseMesh = objects[i];
                }
            }

            instances.push_back(new Instance(id, baseMesh, matIndex, resetTransform, transformations));
            pObject = pObject->NextSiblingElement("MeshInstance");
        }
    }

    void ParseLights(XMLNode* pRoot, Vector3f &ambientLight, std::vector<PointLight*> &lights){
        const char* str;
        XMLError eResult;

        int id;
        Vector3f position;
        Vector3f intensity;
        XMLElement* pElement = pRoot->FirstChildElement("Lights");

        XMLElement* pLight = pElement->FirstChildElement("AmbientLight");
        XMLElement* lightElement;
        str = pLight->GetText();
        sscanf(str, "%f %f %f", &ambientLight(0), &ambientLight(1), &ambientLight(2));

        pLight = pElement->FirstChildElement("PointLight");
        while (pLight != nullptr)
        {
            eResult = pLight->QueryIntAttribute("id", &id);
            lightElement = pLight->FirstChildElement("Position");
            str = lightElement->GetText();
            sscanf(str, "%f %f %f", &position(0), &position(1), &position(2));
            lightElement = pLight->FirstChildElement("Intensity");
            str = lightElement->GetText();
            sscanf(str, "%f %f %f", &intensity(0), &intensity(1), &intensity(2));

            lights.push_back(new PointLight(position, intensity));

            pLight = pLight->NextSiblingElement("PointLight");
        }
    }
}

#endif
