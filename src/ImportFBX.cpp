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

struct IndexSet
{
	uint32_t controlPoint = -1;
	uint32_t normal = -1;
	uint32_t uv = -1;
	uint32_t tangent = -1;
	uint32_t binormal = -1;
	uint32_t vertexColor = -1;

	bool operator==(const IndexSet& rhs) const
	{
		return controlPoint == rhs.controlPoint &&
			normal == rhs.normal &&
			uv == rhs.uv &&
			tangent == rhs.tangent &&
			binormal == rhs.binormal &&
			vertexColor == rhs.vertexColor;
	}
	
	bool operator<(const IndexSet& rhs) const
	{
		return controlPoint < rhs.controlPoint ||
			(controlPoint == rhs.controlPoint && normal < rhs.normal) ||
			(controlPoint == rhs.controlPoint && normal == rhs.normal && uv < rhs.uv) ||
			(controlPoint == rhs.controlPoint && normal == rhs.normal && uv == rhs.uv &&
				tangent < rhs.tangent) ||
			(controlPoint == rhs.controlPoint && normal == rhs.normal && uv == rhs.uv &&
				tangent == rhs.tangent && binormal < rhs.binormal) ||
			(controlPoint == rhs.controlPoint && normal == rhs.normal && uv == rhs.uv && 
				tangent == rhs.tangent && binormal == rhs.binormal && vertexColor < rhs.vertexColor);
	}
};

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
		objectNode.parentUid = InvalidID;
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

ImportFBXResult importFBXFile(const std::string &path, const ImportSettings &settings)
{
	ImportFBXResult result;

	auto sdkManager = FbxManager::Create();

	auto ioSettings = FbxIOSettings::Create(sdkManager, IOSROOT);
	sdkManager->SetIOSettings(ioSettings);

	// ... Configure the FbxIOSettings object ...

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
			//auto normals = fbxMesh->GetPolygonVertexNormals();

			std::vector<IndexSet> indexSets;
			std::vector<FbxVector4> normals;
			std::vector<FbxVector2> UVs;
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
				//auto normals = fbxMesh->GetNormals()
				//int currentNormal = -1;
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
						verticesToMerge.push_back(orderedVertexPair.second);//first.normal);
					}
					else if (currentControlPoint == orderedVertexPair.first.controlPoint)
					{
						if (currentUV == orderedVertexPair.first.uv) // TODO: also check tangent, binormal and vertexColor
						{
							double cosAngle = currentNormalValue.DotProduct(vertexNormal);
							if (cosAngle > cosThreshold) // merge
							{
								verticesToMerge.push_back(orderedVertexPair.second);// first.normal);
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

						/*for (size_t iterIndex = 1; iterIndex < vertexIterators.size(); ++iterIndex)
						{
							auto currentIter = vertexIterators[iterIndex];
							for (size_t index = 0; index < indexVector.size(); ++index)
							{
								//if (indexVector[index] > currentIter->second) // >= ???
								//{
								//	if (offsetVector[index] != -1)
								//	{
								//		offsetVector[index] += 1;
								//	}
								//}
							}
							orderedVertices.erase(currentIter->first);
						}*/
						orderedIndexMap.merge(mergePair.second);
					}
				}
				optimizedIndexVector = orderedIndexMap.createVector();
				indexVector = optimizedIndexVector;
			}

			// save indices
			if (indexVector.size() > 0)
			{
				MeshStream indexStream;
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
				indexStream.streamSize = indexStream.elementSize * indexStream.elementVectorSize * static_cast<uint32_t>(indexVector.size());
				indexStream.data = new uint8_t[indexStream.streamSize];
				uint32_t dataOffset = 0;
				if (indexStream.elementSize == 2)
				{
					for (size_t index = 0; index < indexVector.size(); ++index)
					{
						uint16_t indexValue = static_cast<uint16_t>(indexVector[index]);
						memcpy(indexStream.data + dataOffset, &indexValue, sizeof(indexValue));
						dataOffset += indexStream.elementSize * indexStream.elementVectorSize;
					}
				}
				else //if (indexStream.elementSize == 4)
				{
					memcpy(indexStream.data, indexVector.data(), indexStream.streamSize);
				}
				indexStream.elementCount = static_cast<uint32_t>(indexVector.size());
				mesh.streams.push_back(indexStream);
			}

			// normals
			if (normals.size() > 0)
			{
				MeshStream normalStream;
				normalStream.elementType = static_cast<uint32_t>(StreamElementType::Float);
				normalStream.elementVectorSize = 3;
				normalStream.elementSize = 4;
				normalStream.streamSize = normalStream.elementSize * normalStream.elementVectorSize * static_cast<uint32_t>(uniqueVertices.size());
				normalStream.data = new uint8_t[normalStream.streamSize];
				uint32_t dataOffset = 0;
				for (int vertexIndex = 0; vertexIndex < uniqueVertices.size(); ++vertexIndex)
				{
					float normal[3];
					const auto normalData = normals[uniqueVertices[vertexIndex].normal].mData;
					normal[0] = static_cast<float>(normalData[0]);
					normal[1] = static_cast<float>(normalData[1]);
					normal[2] = static_cast<float>(normalData[2]);
					memcpy(normalStream.data + dataOffset, normal, sizeof(normal));
					dataOffset += normalStream.elementSize * normalStream.elementVectorSize;
				}
				normalStream.elementCount = static_cast<uint32_t>(uniqueVertices.size());
				mesh.streams.push_back(normalStream);
			}
			// texture coordinates
			if (UVs.size() > 0)
			{
				MeshStream uvStream;
				uvStream.elementType = static_cast<uint32_t>(StreamElementType::Float);
				uvStream.elementVectorSize = 2;
				uvStream.elementSize = 4;
				uvStream.streamSize = uvStream.elementSize * uvStream.elementVectorSize * static_cast<uint32_t>(uniqueVertices.size());
				uvStream.data = new uint8_t[uvStream.streamSize];
				uint32_t dataOffset = 0;
				for (int vertexIndex = 0; vertexIndex < uniqueVertices.size(); ++vertexIndex)
				{
					float uv[2];
					const auto uvData = UVs[uniqueVertices[vertexIndex].uv].mData;
					uv[0] = static_cast<float>(uvData[0]);
					uv[1] = static_cast<float>(uvData[1]);
					memcpy(uvStream.data + dataOffset, uv, sizeof(uv));
					dataOffset += uvStream.elementSize * uvStream.elementVectorSize;
				}
				uvStream.elementCount = static_cast<uint32_t>(uniqueVertices.size());
				mesh.streams.push_back(uvStream);
			}

			// import vertices
			if (uniqueVertices.size() > 0 && controlPoints != nullptr) // remove 4th dimension
			{
				MeshStream vertexStream;
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
				vertexStream.streamSize = vertexStream.elementSize * vertexStream.elementVectorSize * static_cast<uint32_t>(uniqueVertices.size());
				vertexStream.data = new uint8_t[vertexStream.streamSize];
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
						memcpy(vertexStream.data + dataOffset, position, sizeof(position));
					}
					else
					{
						double position[3];
						position[0] = controlPoint.mData[0];
						position[1] = controlPoint.mData[1];
						position[2] = controlPoint.mData[2];
						memcpy(vertexStream.data + dataOffset, position, sizeof(position));
					}
					dataOffset += vertexStream.elementSize * vertexStream.elementVectorSize;
				}
				vertexStream.elementCount = static_cast<uint32_t>(uniqueVertices.size());
				mesh.streams.push_back(vertexStream);
			}

			mesh.header.streamCount = static_cast<uint32_t>(mesh.streams.size());
			result.sceneMeshes.push_back(mesh);
		}
	}

	for (int nodeIndex = 0; nodeIndex < scene->GetNodeCount(); ++nodeIndex)
	{
		auto fbxNode = scene->GetNode(nodeIndex);
		if (settings.convertPositionsToFloat32)
		{
			auto node = getObjectNode<float>(fbxNode, fbxMeshMap);
			result.objectsFloat.push_back(node);
		}
		else
		{
			auto node = getObjectNode<double>(fbxNode, fbxMeshMap);
			result.objectsDouble.push_back(node);
		}
	}
	result.success = true;
	return result;
}
