//-----------------------------------------------------------------------------
// UtilsFBX.cpp
// Created at 2017.09.05 16:18
// License: see LICENSE file
//
// Utility functions to extract data from FBX structures
//-----------------------------------------------------------------------------
#include "UtilsFBX.h"

MaterialParam<float> getMaterialParam(const std::string &name,
    const FbxPropertyT<FbxDouble> &propertyValue)
{
    auto prop = propertyValue.Get();
    MaterialParam<float> floatParam;
    floatParam.paramName = name;
    floatParam.value = static_cast<float>(prop);
    return floatParam;
}

MaterialParam<float[3]> getMaterialParam(const std::string &name,
    const FbxPropertyT<FbxDouble3> &propertyValue)
{
    auto prop = propertyValue.Get();
    MaterialParam<float[3]> floatVParam;
    floatVParam.paramName = name;
    floatVParam.value[0] = static_cast<float>(prop[0]);
    floatVParam.value[1] = static_cast<float>(prop[1]);
    floatVParam.value[2] = static_cast<float>(prop[2]);
    return floatVParam;
}

MaterialParam<float[4]> getMaterialParam(const std::string &name,
    const FbxPropertyT<FbxDouble4> &propertyValue)
{
    auto prop = propertyValue.Get();
    MaterialParam<float[4]> floatVParam;
    floatVParam.paramName = name;
    floatVParam.value[0] = static_cast<float>(prop[0]);
    floatVParam.value[1] = static_cast<float>(prop[1]);
    floatVParam.value[2] = static_cast<float>(prop[2]);
    floatVParam.value[3] = static_cast<float>(prop[3]);
    return floatVParam;
}

// from http://help.autodesk.com/view/FBX/2018/ENU/?guid=__cpp_ref__import_scene_2_display_mesh_8cxx_example_html
MaterialParam<std::vector<StringPair>> getTextureNames(FbxProperty &property)
{
    MaterialParam<std::vector<StringPair>> result;
    result.paramName = property.GetName();
    int layeredTextureCount = property.GetSrcObjectCount<FbxLayeredTexture>();
    if (layeredTextureCount > 0)
    {
        for (int layeredTexIndex = 0; layeredTexIndex<layeredTextureCount; ++layeredTexIndex)
        {
            FbxLayeredTexture *layeredTexture = property.GetSrcObject<FbxLayeredTexture>(layeredTexIndex);
            int textureCount = layeredTexture->GetSrcObjectCount<FbxTexture>();
            std::string layerTextureNames;
            for (int textureIndex = 0; textureIndex < textureCount; ++textureIndex)
            {
                std::string mapName, mapFileName;
                auto texture = layeredTexture->GetSrcObject<FbxTexture>(textureIndex);
                if (texture)
                {
                    mapName = texture->GetName();
                }
                FbxFileTexture* fileTexture = layeredTexture->GetSrcObject<FbxFileTexture>(textureIndex);
                if (fileTexture)
                {
                    mapFileName = fileTexture->GetFileName();
                }
                result.value.push_back(std::make_pair(mapName, mapFileName));
                break;// TODO: for now, import only first map of the property
            }
        }
    }
    else
    {
        //no layered texture simply get on the property
        int textureCount = property.GetSrcObjectCount<FbxTexture>();
        for (int textureIndex = 0; textureIndex < textureCount; ++textureIndex)
        {
            std::string mapName, mapFileName;
            FbxTexture* texture = property.GetSrcObject<FbxTexture>(textureIndex);
            if (texture)
            {
                mapName = texture->GetName();
                //result.value.push_back(texture->GetName());
            }
            FbxFileTexture* fileTexture = property.GetSrcObject<FbxFileTexture>(textureIndex);
            if (fileTexture)
            {
                mapFileName = fileTexture->GetFileName();
            }
            result.value.push_back(std::make_pair(mapName, mapFileName));
            break;// TODO: for now, import only first map of the property
        }
    }
    return result;
}

MaterialParam<std::vector<StringPair>> 
extractTextureProperty(FbxSurfaceMaterial* material, const char* propertyName)
{
    auto property = material->FindProperty(propertyName);
    if (property.IsValid())
    {
        return getTextureNames(property);
    }
    else
    {
        return MaterialParam<std::vector<StringPair>>();
    }
}

void addTextureProperty(
    std::vector<MaterialParam<std::vector<StringPair>>> &params,
    FbxSurfaceMaterial* material,
    const char* propertyName)
{
    auto property = material->FindProperty(propertyName);
    if (property.IsValid())
    {
        auto textureNames = getTextureNames(property);
        if (!textureNames.value.empty())
        {
            params.push_back(textureNames);
        }
    }
}

VectorStream createFloat3Stream(
    const std::vector<FbxVector4> &srcData,
    const std::vector<IndexSet> &vertexIndices,
    uint32_t indexFieldOffset
)
{
    VectorStream meshStream;
    meshStream.elementType = static_cast<uint32_t>(StreamElementType::Float);
    meshStream.elementVectorSize = 3;
    meshStream.elementSize = 4;
    meshStream.streamSize = meshStream.elementSize * meshStream.elementVectorSize * static_cast<uint32_t>(vertexIndices.size());
    meshStream.data = new uint8_t[meshStream.streamSize];
    uint32_t dataOffset = 0;
    for (int index = 0; index < vertexIndices.size(); ++index)
    {
        float normal[3];
        const auto &indexSet = vertexIndices[index];
        uint32_t indexValue = *(uint32_t*)(((const uint8_t*)&indexSet) + indexFieldOffset);
        const auto normalData = srcData[indexValue].mData;
        normal[0] = static_cast<float>(normalData[0]);
        normal[1] = static_cast<float>(normalData[1]);
        normal[2] = static_cast<float>(normalData[2]);
        memcpy(meshStream.data + dataOffset, normal, sizeof(normal));
        dataOffset += meshStream.elementSize * meshStream.elementVectorSize;
    }
    meshStream.elementCount = static_cast<uint32_t>(vertexIndices.size());
    return meshStream;
}
