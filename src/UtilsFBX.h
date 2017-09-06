//-----------------------------------------------------------------------------
// UtilsFBX.h
// Created at 2017.09.05 16:18
// License: see LICENSE file
//
// Utility functions to extract data from FBX structures
//-----------------------------------------------------------------------------
#pragma once

#include "stdafx.h"
#include <fbxsdk.h>

#include "StreamMeshData.h"
#include "StreamMaterialData.h"
#include "Utils.h"
#include "ObjectNode.h"
#include "IndexSet.h"

template <typename ValueType>
int getDirectIndexByControlPoint(FbxLayerElementTemplate<ValueType>* element, int controlPointIndex)
{
    switch (element->GetReferenceMode())
    {
    case FbxGeometryElement::eDirect:
        return controlPointIndex;
    case FbxGeometryElement::eIndexToDirect:
        return element->GetIndexArray().GetAt(controlPointIndex);
    default:
        break; // other reference modes not shown here!
    }
    return -1;
}

template <typename ValueType>
int getDirectIndexByPolygonVertex(FbxLayerElementTemplate<ValueType>* element, int vertexNumber)
{
    switch (element->GetReferenceMode())
    {
    case FbxGeometryElement::eDirect:
        return vertexNumber;
    case FbxGeometryElement::eIndexToDirect:
        return element->GetIndexArray().GetAt(vertexNumber);
    default:
        break; // other reference modes not shown here!
    }
    return -1;
}

template <typename ValueType>
int getDirectIndex(FbxLayerElementTemplate<ValueType>* element, int controlPointIndex, int vertexNumber)
{
    int directIndex = -1;
    switch (element->GetMappingMode())
    {
    default:
        break;
    case FbxGeometryElement::eByControlPoint:
        directIndex = getDirectIndexByControlPoint(element, controlPointIndex);
        break;
    case FbxGeometryElement::eByPolygonVertex:
    {
        directIndex = getDirectIndexByPolygonVertex(element, vertexNumber);
    }
    break;
    case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
    case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
    case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
        break;
    }
    return directIndex;
}

template <typename FloatType>
ObjectNode<FloatType> getObjectNode(FbxNode *fbxNode, const std::map<FbxMesh*, uint32_t> &fbxMeshMap)
{
    ObjectNode<FloatType> objectNode;
    objectNode.uid = fbxNode->GetUniqueID();
    objectNode.name = fbxNode->GetName();
    auto fbxParent = fbxNode->GetParent();
    if (fbxParent != nullptr)
    {
        objectNode.parentUid = fbxParent->GetUniqueID();
    }
    else
    {
        objectNode.parentUid = InvalidUID;
    }
    auto fbxMesh = fbxNode->GetMesh();
    auto meshIt = fbxMeshMap.find(fbxMesh);
    if (meshIt != fbxMeshMap.end())
    {
        objectNode.meshIndex = meshIt->second;
    }
    else
    {
        objectNode.meshIndex = -1;
    }

    auto localTransform = fbxNode->EvaluateLocalTransform();
    auto translation = localTransform.GetT();
    objectNode.translation[0] = static_cast<FloatType>(translation.mData[0]);
    objectNode.translation[1] = static_cast<FloatType>(translation.mData[1]);
    objectNode.translation[2] = static_cast<FloatType>(translation.mData[2]);
    auto rotation = localTransform.GetQ();
    objectNode.rotation[0] = static_cast<FloatType>(rotation.mData[0]);
    objectNode.rotation[1] = static_cast<FloatType>(rotation.mData[1]);
    objectNode.rotation[2] = static_cast<FloatType>(rotation.mData[2]);
    objectNode.rotation[3] = static_cast<FloatType>(rotation.mData[3]);
    auto scale = localTransform.GetS();
    objectNode.scale[0] = static_cast<FloatType>(scale.mData[0]);
    objectNode.scale[1] = static_cast<FloatType>(scale.mData[1]);
    objectNode.scale[2] = static_cast<FloatType>(scale.mData[2]);
    return objectNode;
}

MaterialParam<float> getMaterialParam(const std::string &name,
    const FbxPropertyT<FbxDouble> &propertyValue);
MaterialParam<float[3]> getMaterialParam(const std::string &name, 
    const FbxPropertyT<FbxDouble3> &propertyValue);
MaterialParam<float[4]> getMaterialParam(const std::string &name,
    const FbxPropertyT<FbxDouble4> &propertyValue);
MaterialParam<std::vector<StringPair>> getTextureNames(FbxProperty &property);
MaterialParam<std::vector<StringPair>> 
extractTextureProperty(FbxSurfaceMaterial* material, const char* propertyName);

void addTextureProperty(
    std::vector<MaterialParam<std::vector<StringPair>>> &params,
    FbxSurfaceMaterial* material,
    const char* propertyName
);

VectorStream createFloat3Stream(
    const std::vector<FbxVector4> &srcData,
    const std::vector<IndexSet> &vertexIndices,
    uint32_t indexFieldOffset
);
