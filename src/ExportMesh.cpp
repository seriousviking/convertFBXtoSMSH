//-----------------------------------------------------------------------------
// ExportMesh.cpp
// Created at 2017.09.02 14:08
// License: see LICENSE file
//
// saves stream mesh to file
//-----------------------------------------------------------------------------
#include "ExportMesh.h"
#include "StreamMeshData.h"

template <typename T>
void writeToStream(std::ostream &ofs, T value)
{
    ofs.write((char*)&value, sizeof(value));
}

bool exportMeshToFile(const std::string &fileName, const StreamMesh &meshData)
{
    using namespace std;
    ofstream ofs;
    ofs.open(fileName, ios_base::beg | ios_base::binary);
    if (!ofs.is_open())
    {
        return false;
    }
    writeToStream(ofs, meshData.header.magicMESH);
    writeToStream(ofs, meshData.header.headerSize);
    writeToStream(ofs, meshData.header.version);
    writeToStream(ofs, meshData.header.streamCount);
    for (const auto &streamData : meshData.streams)
    {
        writeToStream(ofs, streamData.magicSTRM);
        writeToStream(ofs, streamData.streamSize);
        writeToStream(ofs, streamData.elementCount);
        writeToStream(ofs, streamData.elementType);
        writeToStream(ofs, streamData.elementSize);
        writeToStream(ofs, streamData.elementVectorSize);
        writeToStream(ofs, streamData.attributeType);
        ofs.write((char*)streamData.data, streamData.elementCount);
    }
    ofs.close();
    return true;
}
