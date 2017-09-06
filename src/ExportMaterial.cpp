//-----------------------------------------------------------------------------
// ExportMaterial.cpp
// Created at 2017.09.06 15:02
// License: see LICENSE file
//
// saves material to json object or (in future?) to file
//-----------------------------------------------------------------------------
#include "ExportMaterial.h"

template<typename T>
Json::Value serializeMaterialParam(const MaterialParam<T> &param) 
{ 
    assert(0 && "Unsupported material param type");
    return Json::Value();
}

template<>
Json::Value serializeMaterialParam(const MaterialParam<float> &param)
{
    Json::Value result(Json::objectValue);
    result["paramName"] = param.paramName;
    result["value"] = param.value;
    return result;
}

template<>
Json::Value serializeMaterialParam(const MaterialParam<int> &param)
{
    Json::Value result(Json::objectValue);
    result["paramName"] = param.paramName;
    result["value"] = param.value;
    return result;
}

// TODO: improve template specializations
template<typename T, size_t N>
Json::Value serializeMaterialArrayParam(const MaterialParam<T[N]> &param)
{
    Json::Value result(Json::objectValue);
    result["paramName"] = param.paramName;
    Json::Value jArray(Json::arrayValue);
    for (size_t index = 0; index < N; ++index)
    {
        jArray.append(param.value[index]);
    }
    result["value"] = jArray;
    return result;
}

template<>
Json::Value serializeMaterialParam(const MaterialParam<std::vector<StringPair>> &param)
{
    Json::Value result(Json::objectValue);
    result["paramName"] = param.paramName;
    Json::Value jArray(Json::arrayValue);
    for (const auto &pair : param.value)
    {
        Json::Value jPair(Json::objectValue);
        jPair[pair.first] = pair.second; // maybe it's better to make separate entry for "first" item
        jArray.append(jPair);
    }
    result["value"] = jArray;
    return result;
}

Json::Value serializeMaterial(const Material &material)
{
    Json::Value result(Json::objectValue);
    result["materialId"] = material.materialId;
    result["materialName"] = material.materialName;
    Json::Value jFloatParams(Json::arrayValue);
    for (const auto &floatParam : material.floatParams)
    {
        jFloatParams.append(serializeMaterialParam(floatParam));
    }
    result["floatParams"] = jFloatParams;
    Json::Value jIntParams(Json::arrayValue);
    for (const auto &intParam : material.intParams)
    {
        jIntParams.append(serializeMaterialParam(intParam));
    }
    result["intParams"] = jIntParams;
    Json::Value jFloat3Params(Json::arrayValue);
    for (const auto &float3Param : material.float3Params)
    {
        jFloat3Params.append(serializeMaterialArrayParam(float3Param));
    }
    result["float3Params"] = jFloat3Params;
    Json::Value jFloat4Params(Json::arrayValue);
    for (const auto &float4Param : material.float4Params)
    {
        jFloat4Params.append(serializeMaterialArrayParam(float4Param));
    }
    result["float4Params"] = jFloat4Params;
    Json::Value jMapNameParams(Json::arrayValue);
    for (const auto &mapNameParam : material.mapNameParams)
    {
        jMapNameParams.append(serializeMaterialParam(mapNameParam));
    }
    result["mapNameParams"] = jMapNameParams;
    return result;
}
