//-----------------------------------------------------------------------------
// ObjectNode.h
// Created at 2017.09.05 21:10
// License: see LICENSE file
//
// ObjectNode class
//-----------------------------------------------------------------------------
#pragma once

const uint64_t InvalidUID = (uint64_t)-1;
const uint32_t InvalidID = (uint32_t)-1;

template <typename FloatType>
struct ObjectNode
{
    uint64_t uid;
    uint64_t parentUid = InvalidUID;
    std::string name;
    FloatType translation[3];
    FloatType rotation[4];
    FloatType scale[3];
    uint32_t meshIndex = InvalidID;
    //uint32_t materialIndex = InvalidID;
    std::vector<uint32_t> materialIndices; // TODO: names???
};

