//-----------------------------------------------------------------------------
// Utils.cpp
// Created at 2017.09.01 16:51
// License: see LICENSE file
//
// helper functions
//-----------------------------------------------------------------------------
#include "Utils.h"

std::string setRelativePath(const std::string &sourcePath, const std::string &relativePath)
{
    std::string result(relativePath);
    auto lastPos = sourcePath.find_last_of("/\\");
    if (lastPos != std::string::npos)
    {
        result += sourcePath.substr(lastPos + 1);
    }
    else
    {
        result += "/" + sourcePath;
    }
    return result;
}