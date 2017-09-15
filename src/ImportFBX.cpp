//-----------------------------------------------------------------------------
// ImportFBX.cpp
// Created at 2017.08.26 14:09
// License: see LICENSE file
//
// import FBX file: mesh, scene nodes, materials
//-----------------------------------------------------------------------------
#include "ImportFBX.h"
#include "stdafx.h"
#include <fbxsdk.h>

#include "Common.h"
#include "Utils.h"
#include "UtilsFBX.h"
#include "IndexSet.h"

ImportFBXResult importFBXFile(const std::string &path, const ImportSettings &settings)
{
    ImportFBXResult result;

    auto sdkManager = FbxManager::Create();
    auto ioSettings = FbxIOSettings::Create(sdkManager, IOSROOT);
    sdkManager->SetIOSettings(ioSettings);
    // TODO: Configure the FbxIOSettings object if needed
    auto fbxImporter = FbxImporter::Create(sdkManager, "");
    bool importStatus;
    importStatus = fbxImporter->Initialize(path.c_str(), -1, sdkManager->GetIOSettings());
    if (!importStatus)
    {
        std::cout << "Error loading file " << path << " :" << std::endl << fbxImporter->GetStatus().GetErrorString() << std::endl;
        return result;
    }

    // get file version
    int versionMajor, versionMinor, versionRevision;
    fbxImporter->GetFileVersion(versionMajor, versionMinor, versionRevision);
    std::cout << "Imported file version: " << versionMajor << "." << versionMinor << "." << versionRevision << std::endl;

    // import scene
    auto scene = FbxScene::Create(sdkManager, "importedScene");
    importStatus = fbxImporter->Import(scene);
    if (!importStatus)
    {
        std::cout << "Error importing scene for file " << path << " :" << std::endl << fbxImporter->GetStatus().GetErrorString() << std::endl;
        return result;
    }
    fbxImporter->Destroy();

    std::map<FbxMesh*, uint32_t> fbxMeshMap;
    //in FBX: right handed, Y-Up axis system. 1 unit = 1cm
    // import all scene geometries
    for (int32_t geometryIndex = 0; geometryIndex < scene->GetGeometryCount(); ++geometryIndex)
    {
        FbxGeometry* geometry = scene->GetGeometry(geometryIndex);
        if (geometry->GetAttributeType() == FbxNodeAttribute::EType::eMesh)
        {
            StreamMesh mesh;

            FbxMesh* fbxMesh = static_cast<FbxMesh*>(geometry);
            fbxMeshMap.insert(std::make_pair(fbxMesh, static_cast<uint32_t>(result.sceneMeshes.size())));
            int vertexCount = fbxMesh->GetControlPointsCount();
            int polygonCount = fbxMesh->GetPolygonCount();

            auto controlPoints = fbxMesh->GetControlPoints();

            std::vector<IndexSet> indexSets;
            std::vector<FbxVector4> normals;
            std::vector<FbxVector2> UVs;
            std::vector<FbxVector4> tangents;
            std::vector<FbxVector4> binormals;
            int vertexId = 0;
            // process polygons - split vertices
            for (int polygonIndex = 0; polygonIndex < polygonCount; polygonIndex++)
            {
                //DisplayInt("        Polygon ", polygonIndex);
                /*int elementTangentIndex;
                for (elementTangentIndex = 0; elementTangentIndex < fbxMesh->GetElementPolygonGroupCount(); elementTangentIndex++)
                {
                    FbxGeometryElementPolygonGroup* elementPolyGroup = fbxMesh->GetElementPolygonGroup(elementTangentIndex);
                    switch (elementPolyGroup->GetMappingMode())
                    {
                    case FbxGeometryElement::eByPolygon:
                        if (elementPolyGroup->GetReferenceMode() == FbxGeometryElement::eIndex)
                        {
                            FBXSDK_sprintf(header, 100, "        Assigned to group: ");
                            int polyGroupId = elementPolyGroup->GetIndexArray().GetAt(polygonIndex);
                            DisplayInt(header, polyGroupId);
                            break;
                        }
                    default:
                        // any other mapping modes don't make sense
                        //DisplayString("        \"unsupported group assignment\"");
                        break;
                    }
                }*/
                int polygonSize = fbxMesh->GetPolygonSize(polygonIndex);
                std::vector<IndexSet> polygonIndexSets;
                for (int polyVertexIndex = 0; polyVertexIndex < polygonSize; polyVertexIndex++)
                {
                    IndexSet indexSet;
                    int controlPointIndex = fbxMesh->GetPolygonVertex(polygonIndex, polyVertexIndex);
                    indexSet.controlPoint = controlPointIndex;
                    if (settings.importVertexColors)
                    {
                        for (int vertexColorIndex = 0; vertexColorIndex < fbxMesh->GetElementVertexColorCount(); vertexColorIndex++)
                        {
                            FbxGeometryElementVertexColor* elementVertexColor = fbxMesh->GetElementVertexColor(vertexColorIndex);
                            int directColorIndex = getDirectIndex<FbxColor>(elementVertexColor, controlPointIndex, vertexId); //-1;
                            if (directColorIndex > 0)
                            {
                                indexSet.vertexColor = directColorIndex;
                            }
                        }
                    }
                    if (settings.importUVs)
                    {
                        for (int uvIndex = 0; uvIndex < fbxMesh->GetElementUVCount(); ++uvIndex)
                        {
                            FbxGeometryElementUV* elementUV = fbxMesh->GetElementUV(uvIndex);
                            int directUVIndex = -1;
                            switch (elementUV->GetMappingMode())
                            {
                            default:
                                break;
                            case FbxGeometryElement::eByControlPoint:
                                directUVIndex = getDirectIndexByControlPoint<FbxVector2>(elementUV, controlPointIndex);
                                break;
                            case FbxGeometryElement::eByPolygonVertex:
                            {
                                int textureUVIndex = fbxMesh->GetTextureUVIndex(polygonIndex, polyVertexIndex);
                                switch (elementUV->GetReferenceMode())
                                {
                                case FbxGeometryElement::eDirect:
                                case FbxGeometryElement::eIndexToDirect:
                                {
                                    directUVIndex = textureUVIndex;
                                }
                                break;
                                default:
                                    break; // other reference modes not shown here!
                                }
                            }
                            break;
                            case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
                            case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
                            case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
                                break;
                            }
                            indexSet.uv = directUVIndex;
                            auto uvValue = elementUV->GetDirectArray().GetAt(directUVIndex);
                            UVs.push_back(uvValue);
                        }
                    }
                    if (settings.importNormals)
                    {
                        for (int elementNormalIndex = 0; elementNormalIndex < fbxMesh->GetElementNormalCount(); ++elementNormalIndex)
                        {
                            FbxGeometryElementNormal* elementNormal = fbxMesh->GetElementNormal(elementNormalIndex);
                            if (elementNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                            {
                                int directIndex = getDirectIndexByPolygonVertex(elementNormal, vertexId);
                                indexSet.normal = directIndex;
                                auto normalValue = elementNormal->GetDirectArray().GetAt(directIndex);
                                normals.push_back(normalValue);
                            }
                        }
                    }
                    if (settings.importTangents)
                    {
                        for (int elementTangentIndex = 0; elementTangentIndex < fbxMesh->GetElementTangentCount(); ++elementTangentIndex)
                        {
                            FbxGeometryElementTangent* elementTangent = fbxMesh->GetElementTangent(elementTangentIndex);
                            if (elementTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                            {
                                int directIndex = getDirectIndexByPolygonVertex(elementTangent, vertexId);
                                indexSet.tangent = directIndex;
                                auto tangentValue = elementTangent->GetDirectArray().GetAt(directIndex);
                                tangents.push_back(tangentValue);
                            }
                        }
                    }
                    if (settings.importBinormals)
                    {
                        for (int elementBinormalIndex = 0; elementBinormalIndex < fbxMesh->GetElementBinormalCount(); ++elementBinormalIndex)
                        {
                            FbxGeometryElementBinormal* elementBinormal = fbxMesh->GetElementBinormal(elementBinormalIndex);
                            if (elementBinormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                            {
                                int directIndex = getDirectIndexByPolygonVertex(elementBinormal, vertexId);
                                indexSet.binormal = directIndex;
                                auto binormalValue = elementBinormal->GetDirectArray().GetAt(directIndex);
                                binormals.push_back(binormalValue);
                            }
                        }
                    }
                    polygonIndexSets.push_back(indexSet);
                    vertexId++;
                } // for polygonSize
                for (size_t polygonVertexIndex = 2; polygonVertexIndex < polygonIndexSets.size(); ++polygonVertexIndex) // triangulate if needed
                {
                    indexSets.push_back(polygonIndexSets[0]);
                    indexSets.push_back(polygonIndexSets[polygonVertexIndex - 1]);
                    indexSets.push_back(polygonIndexSets[polygonVertexIndex]);
                }
            } // for polygonCount

            std::vector<uint32_t> indexVector;
            std::map<IndexSet, uint32_t> orderedVertices; // vertex stream index set -> index
            std::vector<IndexSet> uniqueVertices;
            std::vector<uint32_t> optimizedIndexVector;
            OrderedIndexMap<uint32_t, uint32_t> orderedIndexMap;

            // TODO: optimize by spatial position/uv
            uint32_t currentIndex = 0;
            uint32_t currentPosition = 0;
            for (const auto &indexSet : indexSets)
            {
                auto orderedIt = orderedVertices.find(indexSet);
                if (orderedIt == orderedVertices.end())
                {
                    orderedVertices.insert(std::make_pair(indexSet, currentIndex));
                    uniqueVertices.push_back(indexSet);
                    indexVector.push_back(currentIndex);
                    orderedIndexMap.add(currentIndex, currentPosition);
                    currentIndex++;
                }
                else
                {
                    indexVector.push_back(orderedIt->second);
                    orderedIndexMap.add(orderedIt->second, currentPosition);
                }
                ++currentPosition;
            }
            // import normals, merge vertices with similar normals
            if (settings.importNormals && settings.mergeNormalThresholdAngle > 0.0f)
            {
                std::vector<uint32_t> offsetVector(indexVector.size(), 0);
                double cosThreshold = std::cos(settings.mergeNormalThresholdAngle * Pi / 180.0f);
                int currentControlPoint = -1;
                int currentUV = -1;
                FbxVector4 currentNormalValue(0, 0, 0, 0);
                std::vector<uint32_t> verticesToMerge;
                std::map<uint32_t, std::vector<uint32_t>> normalVerticesToMerge;
                for (const auto &orderedVertexPair : orderedVertices)
                {
                    const auto &vertexNormal = normals[orderedVertexPair.first.normal];
                    if (currentControlPoint == -1)
                    {
                        currentControlPoint = orderedVertexPair.first.controlPoint;
                        currentUV = orderedVertexPair.first.uv;
                        currentNormalValue = vertexNormal;
                        verticesToMerge.clear();
                        verticesToMerge.push_back(orderedVertexPair.second);
                    }
                    else if (currentControlPoint == orderedVertexPair.first.controlPoint)
                    {
                        if (currentUV == orderedVertexPair.first.uv) // TODO: also check tangent, binormal and vertexColor
                        {
                            double cosAngle = currentNormalValue.DotProduct(vertexNormal);
                            if (cosAngle > cosThreshold) // merge
                            {
                                verticesToMerge.push_back(orderedVertexPair.second);
                            }
                        }
                        else
                        {
                            currentUV = orderedVertexPair.first.uv;
                            if (verticesToMerge.size() > 1)
                            {
                                normalVerticesToMerge.insert(std::make_pair(currentControlPoint, verticesToMerge));
                            }
                            verticesToMerge.clear();
                        }
                    }
                    else
                    {
                        if (verticesToMerge.size() > 1)
                        {
                            normalVerticesToMerge.insert(std::make_pair(currentControlPoint, verticesToMerge));
                        }
                        currentControlPoint = orderedVertexPair.first.controlPoint;
                        currentUV = orderedVertexPair.first.uv;
                        currentNormalValue = vertexNormal;
                        verticesToMerge.clear();
                        verticesToMerge.push_back(orderedVertexPair.second);// first.normal);
                    }
                }
                if (verticesToMerge.size() > 1)
                {
                    normalVerticesToMerge.insert(std::make_pair(currentControlPoint, verticesToMerge));
                }

                for (const auto &mergePair : normalVerticesToMerge)
                {
                    std::vector<std::map<IndexSet, uint32_t>::iterator> vertexIterators;
                    FbxVector4 normalSum(0, 0, 0, 0);
                    for (const auto vertexIndex : mergePair.second)
                    {
                        auto it = std::find_if(orderedVertices.begin(), orderedVertices.end(),
                            [vertexIndex](const std::pair<IndexSet, uint32_t> &vertexPair)
                        {
                            return vertexPair.second == vertexIndex;
                        });
                        assert(it != orderedVertices.end());
                        if (it != orderedVertices.end())
                        {
                            vertexIterators.push_back(it);
                            const auto& normal = normals[it->first.normal];
                            normalSum += normal;
                        }
                    }
                    if (vertexIterators.size() > 1)
                    {
                        normalSum /= (double)vertexIterators.size();
                        normalSum.Normalize();
                        const auto &firstIter = vertexIterators[0];
                        normals[firstIter->first.normal] = normalSum;

                        std::replace_if(indexVector.begin(), indexVector.end(),
                            [vertexIterators](const uint32_t &index) {
                            for (size_t iterIndex = 1; iterIndex < vertexIterators.size(); ++iterIndex)
                            {
                                auto currentIter = vertexIterators[iterIndex];
                                if (index == currentIter->second)
                                {
                                    return true;
                                }
                            }
                            return false;
                        }, vertexIterators[0]->second);

                        uniqueVertices.erase(std::remove_if(uniqueVertices.begin(), uniqueVertices.end(),
                            [vertexIterators](const IndexSet& vertex) {
                            for (size_t iterIndex = 1; iterIndex < vertexIterators.size(); ++iterIndex)
                            {
                                auto currentIter = vertexIterators[iterIndex];
                                if (vertex == (*currentIter).first)
                                {
                                    return true;
                                }
                            }
                            return false;
                        }), uniqueVertices.end());

                        orderedIndexMap.merge(mergePair.second);
                    }
                }
                optimizedIndexVector = orderedIndexMap.createVector();
                indexVector = optimizedIndexVector;
            }

            // save indices
            if (indexVector.size() > 0)
            {
                VectorStream indexStream;
                indexStream.elementType = static_cast<uint32_t>(StreamElementType::UInt);
                indexStream.elementVectorSize = 1;
                if (uniqueVertices.size() < 65535)
                {
                    indexStream.elementSize = 2;
                }
                else
                {
                    indexStream.elementSize = 4;
                }
                uint32_t dataSize = indexStream.elementSize * indexStream.elementVectorSize * static_cast<uint32_t>(indexVector.size());
                indexStream.streamSize = dataSize + sizeof(indexStream.magicSTRM) + sizeof(indexStream.streamSize);
                indexStream.data.resize(dataSize);
                uint32_t dataOffset = 0;
                if (indexStream.elementSize == 2)
                {
                    for (size_t index = 0; index < indexVector.size(); ++index)
                    {
                        uint16_t indexValue = static_cast<uint16_t>(indexVector[index]);
                        memcpy(indexStream.data.data() + dataOffset, &indexValue, sizeof(indexValue));
                        dataOffset += indexStream.elementSize * indexStream.elementVectorSize;
                    }
                }
                else //if (indexStream.elementSize == 4)
                {
                    memcpy(indexStream.data.data(), indexVector.data(), indexStream.streamSize);
                }
                indexStream.elementCount = static_cast<uint32_t>(indexVector.size());
                indexStream.attributeType = static_cast<uint32_t>(AttributeType::Index);
                mesh.streams.push_back(indexStream);
            }

            // normals
            if (normals.size() > 0)
            {
                uint32_t fieldOffset = offsetof(IndexSet, normal);
                VectorStream normalStream = createFloat3Stream(normals, uniqueVertices, fieldOffset);
                normalStream.attributeType = static_cast<uint32_t>(AttributeType::Normal);
                mesh.streams.push_back(normalStream);
            }
            // texture coordinates
            if (UVs.size() > 0)
            {
                VectorStream uvStream;
                uvStream.elementType = static_cast<uint32_t>(StreamElementType::Float);
                uvStream.elementVectorSize = 2;
                uvStream.elementSize = 4;
                uint32_t dataSize = uvStream.elementSize * uvStream.elementVectorSize * static_cast<uint32_t>(uniqueVertices.size());
                uvStream.streamSize = dataSize + sizeof(uvStream.magicSTRM) + sizeof(uvStream.streamSize);
                uvStream.data.resize(uvStream.streamSize);
                uint32_t dataOffset = 0;
                for (int vertexIndex = 0; vertexIndex < uniqueVertices.size(); ++vertexIndex)
                {
                    float uv[2];
                    const auto uvData = UVs[uniqueVertices[vertexIndex].uv].mData;
                    uv[0] = static_cast<float>(uvData[0]);
                    uv[1] = static_cast<float>(uvData[1]);
                    memcpy(uvStream.data.data() + dataOffset, uv, sizeof(uv));
                    dataOffset += uvStream.elementSize * uvStream.elementVectorSize;
                }
                uvStream.elementCount = static_cast<uint32_t>(uniqueVertices.size());
                uvStream.attributeType = static_cast<uint32_t>(AttributeType::UV);
                mesh.streams.push_back(uvStream);
            }
            // tangents
            if (tangents.size() > 0)
            {
                uint32_t fieldOffset = offsetof(IndexSet, tangent);
                VectorStream tangentStream = createFloat3Stream(tangents, uniqueVertices, fieldOffset);
                tangentStream.attributeType = static_cast<uint32_t>(AttributeType::Tangent);
                mesh.streams.push_back(tangentStream);
            }
            // binormals
            if (binormals.size() > 0)
            {
                uint32_t fieldOffset = offsetof(IndexSet, binormal);
                VectorStream binormalStream = createFloat3Stream(binormals, uniqueVertices, fieldOffset);
                binormalStream.attributeType = static_cast<uint32_t>(AttributeType::Binormal);
                mesh.streams.push_back(binormalStream);
            }
            // import vertices
            if (uniqueVertices.size() > 0 && controlPoints != nullptr) // remove 4th dimension
            {
                VectorStream vertexStream;
                vertexStream.elementType = static_cast<uint32_t>(StreamElementType::Float);
                vertexStream.elementVectorSize = 3;
                if (settings.convertPositionsToFloat32)
                {
                    vertexStream.elementSize = 4;
                }
                else
                {
                    vertexStream.elementSize = 8;
                }
                uint32_t dataSize = vertexStream.elementSize * vertexStream.elementVectorSize * static_cast<uint32_t>(uniqueVertices.size());
                vertexStream.streamSize = dataSize + sizeof(vertexStream.magicSTRM) + sizeof(vertexStream.streamSize);
                vertexStream.data.resize(dataSize);
                uint32_t dataOffset = 0;
                for (int vertexIndex = 0; vertexIndex < uniqueVertices.size(); ++vertexIndex)
                {
                    //const auto &controlPoint = controlPoints[vertexIndex];
                    int uniqueIndex = uniqueVertices[vertexIndex].controlPoint;
                    const auto &controlPoint = controlPoints[uniqueIndex];
                    if (settings.convertPositionsToFloat32)
                    {
                        float position[3];
                        position[0] = static_cast<float>(controlPoint.mData[0]);
                        position[1] = static_cast<float>(controlPoint.mData[1]);
                        position[2] = static_cast<float>(controlPoint.mData[2]);
                        memcpy(vertexStream.data.data() + dataOffset, position, sizeof(position));
                    }
                    else
                    {
                        double position[3];
                        position[0] = controlPoint.mData[0];
                        position[1] = controlPoint.mData[1];
                        position[2] = controlPoint.mData[2];
                        memcpy(vertexStream.data.data() + dataOffset, position, sizeof(position));
                    }
                    dataOffset += vertexStream.elementSize * vertexStream.elementVectorSize;
                }
                vertexStream.elementCount = static_cast<uint32_t>(uniqueVertices.size());
                vertexStream.attributeType = static_cast<uint32_t>(AttributeType::Position);
                mesh.streams.push_back(vertexStream);
            }

            mesh.header.streamCount = static_cast<uint32_t>(mesh.streams.size());
            result.sceneMeshes.push_back(mesh);
        }
    }

    uint32_t materialNumber = 0;
    for (int nodeIndex = 0; nodeIndex < scene->GetNodeCount(); ++nodeIndex)
    {
        auto fbxNode = scene->GetNode(nodeIndex);
        // object material
        int materialCount = fbxNode->GetSrcObjectCount<FbxSurfaceMaterial>();
        std::vector<uint32_t> materialIndices;
        for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
        {
            FbxSurfaceMaterial* material = fbxNode->GetSrcObject<FbxSurfaceMaterial>(materialIndex);
            auto name = material->GetName();
            Material resultMtrl;
            resultMtrl.materialName = name;
            // TODO: process hardware shaders. See 
            // http://help.autodesk.com/view/FBX/2018/ENU/?guid=__cpp_ref__import_scene_2_display_material_8cxx_example_html
            //const FbxImplementation* implementationHLSL = GetImplementation(material, FBXSDK_IMPLEMENTATION_HLSL);
            //const FbxImplementation* implementationCGFX = GetImplementation(material, FBXSDK_IMPLEMENTATION_CGFX);
            if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
            {
                auto phongMaterial = FbxCast<FbxSurfacePhong>(material);
                // TODO: add more parameters in future if needed (***Factors, Bump, Displacement,...)
                auto ambientParam = getMaterialParam("Ambient", phongMaterial->Ambient);
                resultMtrl.float3Params.push_back(ambientParam);
                auto diffuseParam = getMaterialParam("Diffuse", phongMaterial->Diffuse);
                resultMtrl.float3Params.push_back(diffuseParam);
                auto emissiveParam = getMaterialParam("Emissive", phongMaterial->Emissive);
                resultMtrl.float3Params.push_back(emissiveParam);
                auto emissiveFactorParam = getMaterialParam("EmissiveFactor", phongMaterial->EmissiveFactor);
                resultMtrl.floatParams.push_back(emissiveFactorParam);
                auto reflectionParam = getMaterialParam("Reflection", phongMaterial->Reflection);
                resultMtrl.float3Params.push_back(reflectionParam);
                auto reflectionFactorParam = getMaterialParam("ReflectionFactor", phongMaterial->ReflectionFactor);
                resultMtrl.floatParams.push_back(reflectionFactorParam);
                auto shininessParam = getMaterialParam("Shininess", phongMaterial->Shininess);
                resultMtrl.floatParams.push_back(shininessParam);
                auto specularParam = getMaterialParam("Specular", phongMaterial->Specular);
                resultMtrl.float3Params.push_back(specularParam);
                auto transparencyFactorParam = getMaterialParam("TransparencyFactor", phongMaterial->TransparencyFactor);
                resultMtrl.floatParams.push_back(transparencyFactorParam);
                auto transparentParam = getMaterialParam("TransparentColor", phongMaterial->TransparentColor);
                resultMtrl.float3Params.push_back(transparentParam);
            }
            else if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
            {
                auto lambertMaterial = FbxCast<FbxSurfaceLambert>(material);
                // TODO: add more parameters in future if needed (***Factors, Bump, Displacement,...)
                auto ambientParam = getMaterialParam("Ambient", lambertMaterial->Ambient);
                resultMtrl.float3Params.push_back(ambientParam);
                auto diffuseParam = getMaterialParam("Diffuse", lambertMaterial->Diffuse);
                resultMtrl.float3Params.push_back(diffuseParam);
                auto emissiveParam = getMaterialParam("Emissive", lambertMaterial->Emissive);
                resultMtrl.float3Params.push_back(emissiveParam);
                auto emissiveFactorParam = getMaterialParam("EmissiveFactor", lambertMaterial->EmissiveFactor);
                resultMtrl.floatParams.push_back(emissiveFactorParam);
                auto transparencyFactorParam = getMaterialParam("TransparencyFactor", lambertMaterial->TransparencyFactor);
                resultMtrl.floatParams.push_back(transparencyFactorParam);
                auto transparentParam = getMaterialParam("TransparentColor", lambertMaterial->TransparentColor);
                resultMtrl.float3Params.push_back(transparentParam);
            }
            //addTextureProperty(resultMtrl.mapNameParams, material, FbxSurfaceMaterial::sDiffuse);
            //...

            std::vector<std::string> propertyNames;
            auto property = material->GetFirstProperty();
            while (property.IsValid())
            {
                propertyNames.push_back(property.GetName().Buffer());
                auto textureNames = getTextureNames(property);
                if (!textureNames.paramName.empty() && !textureNames.value.empty())
                {
                    // set different relative path for texture maps
                    if (!settings.textureRelativePath.empty())
                    {
                        for (size_t nameIndex = 0; nameIndex < textureNames.value.size(); ++nameIndex)
                        {
                            auto &namePair = textureNames.value[nameIndex];
                            namePair.second = setRelativePath(namePair.second, settings.textureRelativePath);
                        }
                    }
                    resultMtrl.mapNameParams.push_back(textureNames);
                }
                property = material->GetNextProperty(property);
            }
            resultMtrl.materialId = materialNumber;
            materialIndices.push_back(materialNumber);
            result.sceneMaterials.push_back(resultMtrl);
            materialNumber++;
        }
        // object info
        if (settings.convertPositionsToFloat32)
        {
            auto node = getObjectNode<float>(fbxNode, fbxMeshMap);
            // TODO: extract material faces from FBX, sort by materials
            if (node.meshIndex != InvalidID)
            {
                uint32_t indexCount = 0;
                for (uint32_t meshStreamIndex = 0;
                    meshStreamIndex < result.sceneMeshes[node.meshIndex].streams.size();
                    ++meshStreamIndex)
                {
                    auto &stream = result.sceneMeshes[node.meshIndex].streams[meshStreamIndex];
                    if (stream.attributeType == static_cast<uint32_t>(AttributeType::Index))
                    {
                        indexCount = stream.elementCount;
                    }
                }
                MaterialIndex matIndex = { 0, 0, indexCount };
                node.materialIndices.push_back(matIndex);
            }
            result.objectsFloat.push_back(node);
        }
        else
        {
            auto node = getObjectNode<double>(fbxNode, fbxMeshMap);
            // TODO: extract material faces from FBX, sort by materials
            // TODO: implement in separate function
            if (node.meshIndex != InvalidID)
            {
                uint32_t indexCount = 0;
                for (uint32_t meshStreamIndex = 0;
                    meshStreamIndex < result.sceneMeshes[node.meshIndex].streams.size();
                    ++meshStreamIndex)
                {
                    auto &stream = result.sceneMeshes[node.meshIndex].streams[meshStreamIndex];
                    if (stream.attributeType == static_cast<uint32_t>(AttributeType::Index))
                    {
                        indexCount = stream.elementCount;
                    }
                }
                MaterialIndex matIndex = { 0, 0, indexCount };
                node.materialIndices.push_back(matIndex);
            }
            result.objectsDouble.push_back(node);
        }
    }
    result.success = true;
    return result;
}
