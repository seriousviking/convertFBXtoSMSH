//-----------------------------------------------------------------------------
// StreamMeshData.h
// Created at 2017.08.26 14:14
// License: see LICENSE file
//
// stream mesh data structures
//-----------------------------------------------------------------------------

#pragma once
#include <cstdint>
#include <vector>

enum class StreamElementType
{
    Float,
    Int,
    UInt,
};

enum class AttributeType
{
    Index,
    Position,
    Normal,
    UV,
    Tangent,
    Binormal
};

namespace StreamConstants
{
    const uint32_t MagicMESH = 0x4853454D;//0x4D455348;
    const uint32_t MagicSTRM = 0x4D525453;//0x5354524D;
}

struct StreamMeshHeader
{
    uint32_t magicMESH = StreamConstants::MagicMESH;
    uint32_t headerSize = sizeof(StreamMeshHeader);
    uint32_t version = 1;
    uint32_t streamCount = 0;
};

struct VectorStream
{
    uint32_t magicSTRM = StreamConstants::MagicSTRM;
    uint32_t streamSize = 0;
    uint32_t attributeType = 0; //see enum AttributeType
    uint32_t elementCount = 0;
    uint32_t elementType = static_cast<uint32_t>(StreamElementType::Float);//StreamElementType
    uint32_t elementSize = 0;// size of one of <elementType>
    uint32_t elementVectorSize = 0;// number of <elementType> in one element of stream
    std::vector<uint8_t> data;

    inline uint32_t headerSize() const 
    { 
        uint32_t size = sizeof(magicSTRM) + sizeof(streamSize) + sizeof(attributeType);
        size += sizeof(elementCount) + sizeof(elementType);
        size += sizeof(elementSize) + sizeof(elementVectorSize);
        return size;
            
    }
};

struct StreamMesh
{
    StreamMeshHeader header;
    std::vector<VectorStream> streams;
};
