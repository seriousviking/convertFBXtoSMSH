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

namespace name_fs = std::experimental::filesystem;

int main(int argc, char* argv[])
{
    // TODO: investigate how to use UTF-8/widechar paths in cross-platform way
    //std::setlocale(LC_ALL, "en_US.utf8");
    //std::locale utf8(std::locale(), ::new std::codecvt_utf8<char>);
    //std::locale::global(utf8);//std::locale("en_US.utf8"));
    //std::cout.imbue(std::locale());

    if (argc < 3)
    {
        std::cout << "<app_name> importFile exportPath" << std::endl;
        return -1;
    }
    ImportSettings settings;
    settings.mergeNormalThresholdAngle = 45.0f;
    std::string importPath(argv[1]);
    auto importData = importFBXFile(importPath, settings);

    if (importData.success)
    {
        std::error_code errorCode;
        std::string exportPath(argv[2]);
        std::string meshPathPrefix("meshes/");
        name_fs::path basePath(exportPath);
        if (!name_fs::exists(basePath))
        {
            name_fs::create_directories(basePath, errorCode);
            if (errorCode)
            {
                std::cout << "Error creating directories at path " << basePath << ": " << errorCode.message() << std::endl;
                return -2;
            }
        }

        name_fs::path meshPath = basePath;
        if (!importData.sceneMeshes.empty())
        {
            meshPath.append(meshPathPrefix);
            if (!name_fs::exists(meshPath))
            {
                name_fs::create_directories(meshPath, errorCode);
                if (errorCode)
                {
                    std::cout << "Error creating directories at path " << meshPath << ": " << errorCode.message() << std::endl;
                    return -3;
                }
            }
        }

        uint32_t sceneMeshIndex = 0;
        for (auto sceneMesh : importData.sceneMeshes)
        {
            name_fs::path resultPath = meshPath;
            resultPath.append(std::to_string(sceneMeshIndex) + ".msh");
            bool success = exportMeshToFile(resultPath.u8string(), sceneMesh);
            if (!success)
            {
                std::cout << "Failed to export mesh at path " + resultPath.u8string() << std::endl;
                return -4;
            }
        }
        name_fs::path scenePath = basePath;
        scenePath.append("scene.json");
        if (!exportSceneToFile(scenePath.u8string(), importData, meshPathPrefix, settings.compactSceneJson))
        {
            std::cout << "Failed to save scene json at path " + scenePath.u8string() << std::endl;
            return -5;
        }
    }
    return 0;
}

