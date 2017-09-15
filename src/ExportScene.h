//-----------------------------------------------------------------------------
// ExportScene.h
// Created at 2017.09.03 22:22
// License: see LICENSE file
//
// saves scene to json file
//-----------------------------------------------------------------------------
#pragma once
#include "stdafx.h"

struct ImportFBXResult;

bool exportSceneToFile(const std::string &fileName, 
                       const ImportFBXResult &importData, 
                       const std::string &meshFilePathPrefix,
                       bool compactJson);
