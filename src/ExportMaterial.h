//-----------------------------------------------------------------------------
// ExportMaterial.h
// Created at 2017.09.06 15:02
// License: see LICENSE file
//
// saves material to json object or (in future?) to file
//-----------------------------------------------------------------------------
#pragma once
#include "stdafx.h"

#include "StreamMaterialData.h"

Json::Value serializeMaterial(const Material &material);
