//-----------------------------------------------------------------------------
// ExportMesh.h
// Created at 2017.09.02 14:08
// License: see LICENSE file
//
// saves stream mesh to file
//-----------------------------------------------------------------------------
#pragma once
#include "stdafx.h"

struct StreamMesh;

bool exportMeshToFile(const std::string &fileName, const StreamMesh &meshData);

