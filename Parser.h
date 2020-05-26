#ifndef _PARSER_H_
#define _PARSER_H_

#include "Instance.h"
#include "Texture.h"

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
        if (pElement != nullptr){
            str = pElement->GetText();
            sscanf(str, "%f %f %f", &backgroundColor(0), &backgroundColor(1), &backgroundColor(2));
        }

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
            float focusDistance = 0, apertureSize = 0;
            bool isDof = false;
            char imageName[64];
            Vector3f pos, gaze, up;
            ImagePlane imgPlane;
            std::string handedness;

            eResult = pCamera->QueryIntAttribute("id", &id);
            bool isLeftHanded = false;
            const XMLAttribute* attr = pCamera->FirstAttribute();
            while (attr != nullptr)
            {
                if (std::strncmp(attr->Name(), "handedness", 10) != 0)
                {
                    attr = attr->Next();
                    continue;
                }

                if (std::strncmp(attr->Value(), "left", 4) == 0)
                {
                    isLeftHanded = true;
                }
                break;
            }

            // Parse depth of field.
            camElement = pCamera->FirstChildElement("FocusDistance");
            if (camElement){
                eResult = camElement->QueryFloatText(&focusDistance);
                isDof = true;
            }
            camElement = pCamera->FirstChildElement("ApertureSize");
            if (camElement){
                eResult = camElement->QueryFloatText(&apertureSize);
            }

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

            cameras.push_back(new Camera(id, numSamples, imageName, pos, gaze, up, imgPlane, focusDistance, apertureSize,
                    isDof, isLeftHanded));

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

            bool degamma = false;
            const XMLAttribute* attrDegamma = pMaterial->FirstAttribute();
            while (attrDegamma != nullptr)
            {
                if (std::strncmp(attrDegamma->Name(), "degamma", 7) != 0)
                {
                    attrDegamma = attrDegamma->Next();
                    continue;
                }

                if (std::strncmp(attrDegamma->Value(), "true", 4) == 0)
                {
                    degamma = true;
                }
                break;
            }

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

            if (degamma){
                float gamma = 2.2f;
                materials[curr]->ambientRef[0] = pow(materials[curr]->ambientRef[0], gamma);
                materials[curr]->ambientRef[1] = pow(materials[curr]->ambientRef[1], gamma);
                materials[curr]->ambientRef[2] = pow(materials[curr]->ambientRef[2], gamma);
                materials[curr]->diffuseRef[0] = pow(materials[curr]->diffuseRef[0], gamma);
                materials[curr]->diffuseRef[1] = pow(materials[curr]->diffuseRef[1], gamma);
                materials[curr]->diffuseRef[2] = pow(materials[curr]->diffuseRef[2], gamma);
                materials[curr]->specularRef[0] = pow(materials[curr]->specularRef[0], gamma);
                materials[curr]->specularRef[1] = pow(materials[curr]->specularRef[1], gamma);
                materials[curr]->specularRef[2] = pow(materials[curr]->specularRef[2], gamma);
            }

            materialElement = pMaterial->FirstChildElement("Roughness");
            if (materialElement != nullptr){
                eResult = materialElement->QueryFloatText(&materials[curr]->roughness);
                materials[curr]->isRough = true;
            }
            else{
                materials[curr]->isRough = false;
            }

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

    void ParseTextures(XMLNode* pRoot, const char* xmlPath, std::vector<Texture*> &textures, std::vector<std::string>& imageNames){
        std::vector<std::string> images;
        const XMLAttribute* attr;
        bool isImage = false;
        int imageId = 0;
        int normalizer = 255;
        float noiseScale = 1;
        float bumpFactor = 1;
        DecalMode dm = NoDecal;
        NoiseConversion nc = NCLinear;
        Interpolation interpolation = NN;

        std::string basePath;
        std::string directPath;
        int lastIndex = 0;
        for (int i = 0; xmlPath[i] != '\0'; i++){
            if (xmlPath[i] == '/'){
                lastIndex = i;
            }
        }
        for (int i = 0; i <= lastIndex; i++){
            basePath += xmlPath[i];
        }

        XMLElement* pElement = pRoot->FirstChildElement("Textures");
        if (pElement == nullptr){
            return;
        }

        XMLElement* pImage = pElement->FirstChildElement("Images");
        if (pImage != nullptr){
            pImage = pElement->FirstChildElement("Images")->FirstChildElement("Image");
            while (pImage != nullptr){
                directPath = basePath + pImage->GetText();
                images.push_back(directPath);
                imageNames.push_back(directPath);
                pImage = pImage->NextSiblingElement("Image");
            }
        }

        XMLElement* textureElement;
        XMLElement* pTextureMap = pElement->FirstChildElement("TextureMap");
        while(pTextureMap != nullptr){
            attr = pTextureMap->FirstAttribute();
            while (attr != nullptr){
                if (std::strncmp(attr->Name(), "type", 4) != 0)
                {
                    attr = attr->Next();
                    continue;
                }

                isImage = std::strncmp(attr->Value(), "image", 5) == 0;
                break;
            }

            textureElement = pTextureMap->FirstChildElement("ImageId");
            if (textureElement != nullptr)
            {
                textureElement->QueryIntText(&imageId);
            }
            textureElement = pTextureMap->FirstChildElement("DecalMode");
            if (textureElement != nullptr)
            {
                if (std::strncmp(textureElement->GetText(), "blend_kd", 8) == 0){
                    dm = BlendKd;
                }
                else if (std::strncmp(textureElement->GetText(), "replace_kd", 10) == 0)
                {
                    dm = ReplaceKd;
                }
                else if (std::strncmp(textureElement->GetText(), "replace_all", 11) == 0){
                    dm = ReplaceAll;
                }
                else if (std::strncmp(textureElement->GetText(), "bump_normal", 11) == 0){
                    dm = BumpNormal;
                }
                else if (std::strncmp(textureElement->GetText(), "replace_normal", 14) == 0){
                    dm = ReplaceNormal;
                }
                else if (std::strncmp(textureElement->GetText(), "replace_background", 18) == 0){
                    dm = ReplaceBackground;
                }
            }
            textureElement = pTextureMap->FirstChildElement("NoiseConversion");
            if (textureElement != nullptr)
            {
                if (std::strncmp(textureElement->GetText(), "absval", 6) == 0){
                    nc = Absval;
                }
                else
                {
                    nc = NCLinear;
                }
            }
            textureElement = pTextureMap->FirstChildElement("Interpolation");
            if (textureElement != nullptr)
            {
                if (std::strncmp(textureElement->GetText(), "nearest", 7) == 0){
                    interpolation = NN;
                }
                else if (std::strncmp(textureElement->GetText(), "bilinear", 8) == 0)
                {
                    interpolation = Bilinear;
                }
            }
            textureElement = pTextureMap->FirstChildElement("Normalizer");
            if (textureElement != nullptr)
            {
                textureElement->QueryIntText(&normalizer);
            }
            textureElement = pTextureMap->FirstChildElement("NoiseScale");
            if (textureElement != nullptr)
            {
                textureElement->QueryFloatText(&noiseScale);
            }
            textureElement = pTextureMap->FirstChildElement("BumpFactor");
            if (textureElement != nullptr)
            {
                textureElement->QueryFloatText(&bumpFactor);
            }

            if (isImage){
                textures.push_back(new Texture(images[imageId-1], dm, interpolation, ImageTexture, normalizer, bumpFactor));
            }
            else{
                textures.push_back(new Texture(dm, interpolation, PerlinTexture, nc, normalizer, noiseScale, bumpFactor));
            }
            pTextureMap = pTextureMap->NextSiblingElement("TextureMap");
        }
    }

    void ParseTransformations(XMLNode* pRoot, std::vector<Transformation*> &translations,
                              std::vector<Transformation*> &scalings, std::vector<Transformation*> &rotations,
                              std::vector<Transformation*> &composites){
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

            // Parse composites.
            XMLElement* pComposite = pElement->FirstChildElement("Composite");
            while(pComposite != nullptr){
                composites.push_back(new Transformation());

                int curr = composites.size() - 1;
                eResult = pComposite->QueryIntAttribute("id", &composites[curr]->id);

                str = pComposite->GetText();
                sscanf(str, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
                        &composites[curr]->composite[0][0], &composites[curr]->composite[1][0],
                       &composites[curr]->composite[2][0], &composites[curr]->composite[3][0],
                       &composites[curr]->composite[0][1], &composites[curr]->composite[1][1],
                       &composites[curr]->composite[2][1], &composites[curr]->composite[3][1],
                       &composites[curr]->composite[0][2], &composites[curr]->composite[1][2],
                       &composites[curr]->composite[2][2], &composites[curr]->composite[3][2],
                       &composites[curr]->composite[0][3], &composites[curr]->composite[1][3],
                       &composites[curr]->composite[2][3], &composites[curr]->composite[3][3]);

                pComposite = pComposite->NextSiblingElement("Composite");
            }
        }
    }

    void ParseVertices(XMLNode* pRoot, std::vector<Vector3f> &vertices){
        const char* str;
        XMLError eResult;

        XMLElement* pElement = pRoot->FirstChildElement("VertexData");
        if (pElement == nullptr){
            return;
        }

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

    void ParseTextureCoordinates(XMLNode* pRoot, std::vector<Vector2f> &textureCoordinates){
        const char* str;
        XMLError eResult;

        XMLElement* pElement = pRoot->FirstChildElement("TexCoordData");
        if (pElement == nullptr){
            return;
        }
        int cursor = 0;
        Vector2f tmpPoint;
        str = pElement->GetText();
        while (str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
        {
            cursor++;
        }
        while (str[cursor] != '\0')
        {
            for (int cnt = 0; cnt < 2; cnt++)
            {
                if (cnt == 0)
                {
                    tmpPoint(0) = atof(str + cursor);
                }
                else
                {
                    tmpPoint(1) = atof(str + cursor);
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
            textureCoordinates.push_back(tmpPoint);
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
            else if (str[cursor] == 'c'){
                transformationIndex = atoi(str + cursor + 1);
                transformations.push_back(new Transformation(transformationIndex, TransformationType::Composite));
            }

            cursor++;
            while(str[cursor] != 's' && str[cursor] != 't' && str[cursor] != 'r' && str[cursor] != '\0'){
                cursor++;
            }
        }
    }

    void ParseObjects(XMLNode* pRoot, const char* xmlPath, std::vector<Shape*> &objects, std::vector<Instance*> &instances,
            std::vector<Vector3f> &vertices, std::vector<Vector2f> &textureCoordinates){
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
            bool isBlur = false;
            glm::vec3 blurTransformation = {0,0,0};
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

            // Parse texture id
            std::vector<int> textures;
            int textureOne, textureTwo;
            objElement = pObject->FirstChildElement("Textures");
            if (objElement != nullptr){
                str = objElement->GetText();
                bool isTwo = false;
                for (int i = 0; str[i] != '\0'; i++){
                    if (str[i] == ' '){
                        isTwo = true;
                    }
                }

                if (isTwo){
                    sscanf(str, "%d %d", &textureOne, &textureTwo);
                    textures.push_back(textureOne);
                    textures.push_back(textureTwo);
                }
                else{
                    sscanf(str, "%d", &textureOne);
                    textures.push_back(textureOne);
                }
            }

            // Parse motion blur.
            objElement = pObject->FirstChildElement("MotionBlur");
            if (objElement != nullptr){
                isBlur = true;
                str = objElement->GetText();
                sscanf(str, "%f %f %f", &blurTransformation[0], &blurTransformation[1], &blurTransformation[2]);
            }

            objElement = pObject->FirstChildElement("Center");
            eResult = objElement->QueryIntText(&cIndex);
            objElement = pObject->FirstChildElement("Radius");
            eResult = objElement->QueryFloatText(&R);

            objects.push_back(new Sphere(id, matIndex, cIndex, R, transformations, blurTransformation, isBlur));
            objects[objects.size()-1]->textures = textures;

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
            bool isBlur = false;
            glm::vec3 blurTransformation = {0,0,0};
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

            // Parse texture id
            std::vector<int> textures;
            int textureOne, textureTwo;
            objElement = pObject->FirstChildElement("Textures");
            if (objElement != nullptr){
                str = objElement->GetText();
                bool isTwo = false;
                for (int i = 0; str[i] != '\0'; i++){
                    if (str[i] == ' '){
                        isTwo = true;
                    }
                }

                if (isTwo){
                    sscanf(str, "%d %d", &textureOne, &textureTwo);
                    textures.push_back(textureOne);
                    textures.push_back(textureTwo);
                }
                else{
                    sscanf(str, "%d", &textureOne);
                    textures.push_back(textureOne);
                }
            }

            // Parse motion blur.
            objElement = pObject->FirstChildElement("MotionBlur");
            if (objElement != nullptr){
                isBlur = true;
                str = objElement->GetText();
                sscanf(str, "%f %f %f", &blurTransformation[0], &blurTransformation[1], &blurTransformation[2]);
            }

            objElement = pObject->FirstChildElement("Indices");
            str = objElement->GetText();
            sscanf(str, "%d %d %d", &p1Index, &p2Index, &p3Index);

            objects.push_back(new Triangle(id, matIndex, p1Index, p2Index, p3Index, transformations, blurTransformation, isBlur));
            objects[objects.size()-1]->textures = textures;

            pObject = pObject->NextSiblingElement("Triangle");
        }

        // Parse meshes
        int meshStartIndex = objects.size();
        std::cout << "- Parsing meshes." << std::endl;
        pObject = pElement->FirstChildElement("Mesh");
        while (pObject != nullptr)
        {
            int id, matIndex, p1Index, p2Index, p3Index, cursor, vertexOffset, textureOffset;
            cursor = 0;
            vertexOffset = 0;
            textureOffset = 0;
            bool isBlur = false;

            glm::vec3 blurTransformation = {0,0,0};
            std::vector<Triangle*> faces;
            std::vector<Transformation*> transformations;

            eResult = pObject->QueryIntAttribute("id", &id);

            // Parse Is Smooth
            bool isSmooth = false;
            const XMLAttribute* attr = pObject->FirstAttribute();
            while (attr != nullptr)
            {
                if (std::strncmp(attr->Name(), "shadingMode", 11) != 0)
                {
                    attr = attr->Next();
                    continue;
                }

                if (std::strncmp(attr->Value(), "smooth", 6) == 0)
                {
                    isSmooth = true;
                }
                break;
            }

            objElement = pObject->FirstChildElement("Material");
            eResult = objElement->QueryIntText(&matIndex);

            // Parse object transformations.
            objElement = pObject->FirstChildElement("Transformations");
            if (objElement != nullptr){
                str = objElement->GetText();
                ParseObjectTransformations(str, transformations);
            }

            // Parse texture id
            std::vector<int> textures;
            int textureOne, textureTwo;
            objElement = pObject->FirstChildElement("Textures");
            if (objElement != nullptr){
                str = objElement->GetText();
                bool isTwo = false;
                for (int i = 0; str[i] != '\0'; i++){
                    if (str[i] == ' '){
                        isTwo = true;
                    }
                }

                if (isTwo){
                    sscanf(str, "%d %d", &textureOne, &textureTwo);
                    textures.push_back(textureOne);
                    textures.push_back(textureTwo);
                }
                else{
                    sscanf(str, "%d", &textureOne);
                    textures.push_back(textureOne);
                }
            }

            // Parse motion blur.
            objElement = pObject->FirstChildElement("MotionBlur");
            if (objElement != nullptr){
                isBlur = true;
                str = objElement->GetText();
                sscanf(str, "%f %f %f", &blurTransformation[0], &blurTransformation[1], &blurTransformation[2]);
            }

            objElement = pObject->FirstChildElement("Faces");

            // Parse PLY File ---------> BEGIN.
            bool isPly = false;
            attr = objElement->FirstAttribute();
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

                textureOffset = textureCoordinates.size() + 1;
                if (plyIn.getElement("vertex").hasProperty("u")){
                    std::vector<double> u = plyIn.getElement("vertex").getProperty<double>("u");
                    std::vector<double> v = plyIn.getElement("vertex").getProperty<double>("v");

                    Vector2f txtCoordinate;
                    for (int i = 0; i < u.size(); i++){
                        txtCoordinate[0] = u[i];
                        txtCoordinate[1] = v[i];
                        textureCoordinates.push_back(txtCoordinate);
                    }
                }

                std::vector<std::vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();
                int fIndSize = fInd.size();
                int vertexCount = vertices.size() + 1;
                for (int i = 0; i < fIndSize; i++)
                {
                    if (fInd[i].size() == 4){
                        p1Index = fInd[i][0] + vertexCount;
                        p2Index = fInd[i][1] + vertexCount;
                        p3Index = fInd[i][2] + vertexCount;
                        faces.push_back(new Triangle(-1, matIndex, p1Index, p2Index, p3Index, isSmooth));

                        p1Index = fInd[i][2] + vertexCount;
                        p2Index = fInd[i][3] + vertexCount;
                        p3Index = fInd[i][0] + vertexCount;
                        faces.push_back(new Triangle(-1, matIndex, p1Index, p2Index, p3Index, isSmooth));
                    }
                    else{
                        p1Index = fInd[i][0] + vertexCount;
                        p2Index = fInd[i][1] + vertexCount;
                        p3Index = fInd[i][2] + vertexCount;
                        faces.push_back(new Triangle(-1, matIndex, p1Index, p2Index, p3Index, isSmooth));
                    }
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

                vertexOffset = vertexCount;
                objects.push_back(new Mesh(id, matIndex, faces, transformations, blurTransformation, isBlur, isSmooth));
                objects[objects.size()-1]->textures = textures;
                objects[objects.size()-1]->textureOffset = textureOffset - vertexOffset;

                pObject = pObject->NextSiblingElement("Mesh");
                continue;
            }
            // Parse PLY File ---------> COMPLETED.

            cursor = 0;
            objElement->QueryIntAttribute("vertexOffset", &vertexOffset);
            objElement->QueryIntAttribute("textureOffset", &textureOffset);
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
                faces.push_back(new Triangle(-1, matIndex, p1Index, p2Index, p3Index, isSmooth));
            }

            objects.push_back(new Mesh(id, matIndex, faces, transformations, blurTransformation, isBlur, isSmooth));
            objects[objects.size()-1]->textures = textures;
            objects[objects.size()-1]->textureOffset = textureOffset - vertexOffset;

            pObject = pObject->NextSiblingElement("Mesh");
        }

        // Parse mesh instances.
        std::cout << "- Parsing meshes instances." << std::endl;
        pObject = pElement->FirstChildElement("MeshInstance");
        while(pObject != nullptr){
            int id, baseMeshId, matIndex;
            bool resetTransform = false;
            bool isBlur = false;
            glm::vec3 blurTransformation = {0,0,0};
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

            // Parse motion blur.
            objElement = pObject->FirstChildElement("MotionBlur");
            if (objElement != nullptr){
                isBlur = true;
                str = objElement->GetText();
                sscanf(str, "%f %f %f", &blurTransformation[0], &blurTransformation[1], &blurTransformation[2]);
            }

            int objectSize = objects.size();
            for(int i = meshStartIndex; i < objectSize; i++){
                if (objects[i]->id == baseMeshId){
                    baseMesh = objects[i];
                }
            }

            instances.push_back(new Instance(id, baseMesh, matIndex, resetTransform, transformations, blurTransformation, isBlur));
            pObject = pObject->NextSiblingElement("MeshInstance");
        }
    }

    void ParseLights(XMLNode* pRoot, Vector3f &ambientLight, std::vector<Light*> &lights, std::vector<std::string>& images,
            int& eli){
        const char* str;
        XMLError eResult;

        int id;
        int imageId;
        Vector3f position;
        Vector3f intensity;
        Vector3f direction;
        Vector3f radiance;
        float coverage;
        float fall;
        float size;
        XMLElement* pElement = pRoot->FirstChildElement("Lights");

        XMLElement* lightElement;
        XMLElement* pLight = pElement->FirstChildElement("AmbientLight");
        if (pLight != nullptr){
            str = pLight->GetText();
            sscanf(str, "%f %f %f", &ambientLight(0), &ambientLight(1), &ambientLight(2));
        }
        else{
            ambientLight = {0,0,0};
        }

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

        pLight = pElement->FirstChildElement("DirectionalLight");
        while (pLight != nullptr)
        {
            eResult = pLight->QueryIntAttribute("id", &id);
            lightElement = pLight->FirstChildElement("Direction");
            str = lightElement->GetText();
            sscanf(str, "%f %f %f", &direction(0), &direction(1), &direction(2));
            lightElement = pLight->FirstChildElement("Radiance");
            str = lightElement->GetText();
            sscanf(str, "%f %f %f", &radiance(0), &radiance(1), &radiance(2));

            lights.push_back(new DirectionalLight(direction, radiance));

            pLight = pLight->NextSiblingElement("DirectionalLight");
        }

        pLight = pElement->FirstChildElement("SpotLight");
        while (pLight != nullptr)
        {
            eResult = pLight->QueryIntAttribute("id", &id);
            lightElement = pLight->FirstChildElement("Position");
            str = lightElement->GetText();
            sscanf(str, "%f %f %f", &position(0), &position(1), &position(2));
            lightElement = pLight->FirstChildElement("Direction");
            str = lightElement->GetText();
            sscanf(str, "%f %f %f", &direction(0), &direction(1), &direction(2));
            lightElement = pLight->FirstChildElement("Intensity");
            str = lightElement->GetText();
            sscanf(str, "%f %f %f", &intensity(0), &intensity(1), &intensity(2));
            lightElement = pLight->FirstChildElement("CoverageAngle");
            eResult = lightElement->QueryFloatText(&coverage);
            lightElement = pLight->FirstChildElement("FalloffAngle");
            eResult = lightElement->QueryFloatText(&fall);

            lights.push_back(new SpotLight(position, direction, intensity, coverage, fall));

            pLight = pLight->NextSiblingElement("SpotLight");
        }

        pLight = pElement->FirstChildElement("AreaLight");
        while (pLight != nullptr)
        {
            eResult = pLight->QueryIntAttribute("id", &id);
            lightElement = pLight->FirstChildElement("Position");
            str = lightElement->GetText();
            sscanf(str, "%f %f %f", &position(0), &position(1), &position(2));
            lightElement = pLight->FirstChildElement("Normal");
            str = lightElement->GetText();
            sscanf(str, "%f %f %f", &direction(0), &direction(1), &direction(2));
            lightElement = pLight->FirstChildElement("Radiance");
            str = lightElement->GetText();
            sscanf(str, "%f %f %f", &radiance(0), &radiance(1), &radiance(2));
            lightElement = pLight->FirstChildElement("Size");
            eResult = lightElement->QueryFloatText(&size);

            lights.push_back(new AreaLight(position, direction, radiance, size));

            pLight = pLight->NextSiblingElement("AreaLight");
        }

        eli = -1;
        pLight = pElement->FirstChildElement("SphericalDirectionalLight");
        while (pLight != nullptr)
        {
            eli = lights.size();
            eResult = pLight->QueryIntAttribute("id", &id);
            lightElement = pLight->FirstChildElement("ImageId");
            eResult = lightElement->QueryIntText(&imageId);

            lights.push_back(new EnvironmentLight(images[imageId-1]));

            pLight = pLight->NextSiblingElement("SphericalDirectionalLight");
        }
    }
}

#endif
