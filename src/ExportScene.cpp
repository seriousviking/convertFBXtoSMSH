//-----------------------------------------------------------------------------
// ExportScene.cpp
// Created at 2017.09.03 22:22
// License: see LICENSE file
//
// saves scene to json file
//-----------------------------------------------------------------------------
#include "ExportScene.h"

#include "ImportFBX.h"
#include "Utils.h"
#include "ExportMaterial.h"

template <typename FloatType>
void appendObjectNodes(Json::Value &jObjectArray, const std::vector<ObjectNode<FloatType>> &objects)
{
    for (auto objectNode : objects)
    {
        Json::Value jTranslation = toJsonArray<FloatType, 3>(objectNode.translation);
        Json::Value jRotation = toJsonArray<FloatType, 4>(objectNode.rotation);
        Json::Value jScale = toJsonArray<FloatType, 3>(objectNode.scale);

        Json::Value jTransform;
        jTransform["translation"] = jTranslation;
        jTransform["rotation"] = jRotation;
        jTransform["scale"] = jScale;

        Json::Value jNode;
        jNode["name"] = objectNode.name;
        jNode["uid"] = intToHexString(objectNode.uid); // or just save as uint64_t?
        jNode["parentUid"] = intToHexString(objectNode.parentUid); // or just save as uint64_t?
        if (objectNode.meshIndex != InvalidID)
        {
            jNode["meshIndex"] = objectNode.meshIndex;
        }
        Json::Value jMaterialArray(Json::arrayValue);
        for (const auto &materialIndex : objectNode.materialIndices)
        {
            Json::Value jMaterial;
            jMaterial["materialId"] = materialIndex.materialId;
            jMaterial["startIndex"] = materialIndex.startIndex;
            jMaterial["indexCount"] = materialIndex.indexCount;
            jMaterialArray.append(jMaterial);
        }
        jNode["materialIndex"] = jMaterialArray;
        jNode["transform"] = jTransform;
        jObjectArray.append(jNode);
    }
}

bool exportSceneToFile(const std::string &fileName, 
                       const ImportFBXResult &importData, 
                       bool compactJson)
{
    Json::Value jObjectArray;
    appendObjectNodes(jObjectArray, importData.objectsFloat);
    appendObjectNodes(jObjectArray, importData.objectsDouble);

    Json::Value jsonRoot;
    jsonRoot["objects"] = jObjectArray;

    Json::Value jMaterialArray;
    for (const auto &material : importData.sceneMaterials)
    {
        jMaterialArray.append(serializeMaterial(material));
    }
    jsonRoot["materials"] = jMaterialArray;

    Json::StreamWriterBuilder jsonBuilder;
    if (compactJson)
    {
        jsonBuilder.settings_["indentation"] = "";
    }

    std::stringstream jsonStreamContents;
    std::ofstream ofScene;
    ofScene.open(fileName, std::ios::beg);
    if (ofScene.is_open())
    {
        std::unique_ptr<Json::StreamWriter> writer(
            jsonBuilder.newStreamWriter());
        writer->write(jsonRoot, &ofScene);
    }
    else
    {
        return false;
    }
    ofScene.close();
    return true;
}
