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
#include "ObjectNode.h"

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
    bool importVertexColors = false;// not implemented yet
    bool importUVs = true;
    bool importNormals = true;
    float mergeNormalThresholdAngle = 0.0f; // angle is in degrees
    bool importTangents = false;
    bool importBinormals = false;
    bool compactSceneJson = true;
};

ImportFBXResult importFBXFile(const std::string &path, const ImportSettings &settings);

