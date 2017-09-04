//-----------------------------------------------------------------------------
// StreamMeshData.h
// Created at 2017.08.26 14:14
// License: see LICENSE file
//
// stream mesh data structures
//-----------------------------------------------------------------------------

#pragma once
#include "stdafx.h"

enum class StreamElementType
{
	Float,
	Int,
	UInt,
};

struct StreamMeshHeader
{
	uint32_t magicMESH = 0x4853454D;//0x4D455348;
	uint32_t headerSize = sizeof(StreamMeshHeader);
	uint32_t version = 1;
	uint32_t streamCount = 0;
	uint32_t materialId = 0;
};

struct MeshStream
{
	uint32_t magicSTRM = 0x4D525453;//0x5354524D;
	uint32_t streamSize = 0;
	uint32_t elementCount = 0;
	uint32_t elementType = static_cast<uint32_t>(StreamElementType::Float);//StreamElementType
	uint32_t elementSize = 0;// size of one of <elementType>
	uint32_t elementVectorSize = 0;// number of <elementType> in one element of stream
	uint8_t* data = nullptr;
};

struct StreamMesh
{
	StreamMeshHeader header;
	std::vector<MeshStream> streams;
};