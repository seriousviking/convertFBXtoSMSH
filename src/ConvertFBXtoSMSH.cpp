//-----------------------------------------------------------------------------
// ConvertFBXtoSMSH.cpp : Defines the entry point for the console application.
// Created at 2017.08.26 14:00
// License: see LICENSE file
//
// command line: ConvertFBXtoSMSH importFile exportFile
//-----------------------------------------------------------------------------
#include "stdafx.h"

#include "ImportFBX.h"
#include "ExportMesh.h"
#include "ExportScene.h"
#include "Utils.h"

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "<app_name> importFile exportFile" << std::endl;
		return -1;
	}
	ImportSettings settings;
	settings.mergeNormalThresholdAngle = 45.0f;
	std::string importPath(argv[1]);
	auto importData = importFBXFile(importPath, settings);

	if (importData.success)
	{
		std::string exportPath(argv[2]);
		uint32_t sceneMeshIndex = 0;
		for (auto sceneMesh : importData.sceneMeshes)
		{
			std::string resultPath = exportPath + "-" + std::to_string(sceneMeshIndex) + ".msh";
			bool success = exportMeshToFile(resultPath, sceneMesh);
			if (!success)
			{
				std::cout << "Failed to export mesh at path " + resultPath << std::endl;
			}
		}

		std::string scenePath = exportPath + ".scene.json";
		if (!exportSceneToFile(scenePath, importData, settings.compactSceneJson))
		{
			std::cout << "Failed to save scene json at path " + scenePath << std::endl;
		}
	}
    return 0;
}

