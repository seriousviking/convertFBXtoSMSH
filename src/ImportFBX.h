//-----------------------------------------------------------------------------
// ImportFBX.h
// Created at 2017.08.26 14:09
// License: see LICENSE file
//
// import FBX file: mesh, scene nodes, materials
//-----------------------------------------------------------------------------
#pragma once
#include "StreamMeshData.h"
#include "StreamMaterialData.h"

const uint64_t InvalidID = (uint64_t)-1;

template <typename FloatType>
struct ObjectNode
{
	uint64_t uid;
	uint64_t parentUid = InvalidID;
	std::string name;
	FloatType translation[3];
	FloatType rotation[4];
	FloatType scale[3];
	uint32_t meshIndex;
};

struct ImportFBXResult
{
	bool success = false;
	std::vector<StreamMesh> sceneMeshes;
	std::vector<Material> sceneMaterials;
	std::vector<ObjectNode<double>> objectsDouble;
	std::vector<ObjectNode<float>> objectsFloat;
};

struct ImportSettings
{
	bool convertPositionsToFloat32 = true;
	bool importVertexColors = false;
	bool importUVs = true;
	bool importNormals = true;
	float mergeNormalThresholdAngle = 0.0f; // angle is in degrees
	bool importTangents = false; // not implemented yet
	bool importBinormals = false; // not implemented yet
	bool compactSceneJson = true;
};

ImportFBXResult importFBXFile(const std::string &path, const ImportSettings &settings);

