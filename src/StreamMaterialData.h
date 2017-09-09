//-----------------------------------------------------------------------------
// StreamMaterialData.h
// Created at 2017.08.26 17:10
// License: see LICENSE file
//
// material data structures
//-----------------------------------------------------------------------------
#pragma once
#include <vector>
#include <string>
#include <utility>
#include "ObjectNode.h"

template <typename T>
struct MaterialParam
{
    std::string paramName;
    T value;
};

using StringPair = std::pair<std::string, std::string>;

struct Material
{
    uint32_t materialId = -1; // TODO: InvalidID
    std::string materialName;
    std::vector<MaterialParam<float>> floatParams;
    std::vector<MaterialParam<float[3]>> float3Params;
    std::vector<MaterialParam<float[4]>> float4Params;
    std::vector<MaterialParam<int>> intParams;
    std::vector<MaterialParam<std::vector<StringPair>>> mapNameParams; // MapName : MapFile. Multiple values for layered textures
};
