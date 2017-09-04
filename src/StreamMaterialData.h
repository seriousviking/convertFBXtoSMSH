//-----------------------------------------------------------------------------
// StreamMaterialData.h
// Created at 2017.08.26 17:10
// License: see LICENSE file
//
// material data structures
//-----------------------------------------------------------------------------
#pragma once
#include "stdafx.h"

template <typename T>
struct MaterialParam
{
	std::string paramName;
	T value;
};

struct Material
{
	uint32_t materialId;
	std::string materialName;
	std::vector<MaterialParam<float>> floatParams;
	std::vector<MaterialParam<float[3]>> float3Params;
	std::vector<MaterialParam<float[4]>> float4Params;
	std::vector<MaterialParam<int>> intParams;
};
