//-----------------------------------------------------------------------------
// Utils.h
// Created at 2017.09.01 16:51
// License: see LICENSE file
//
// helper functions
//-----------------------------------------------------------------------------
#pragma once
#include "stdafx.h"

template <typename IndexType, typename PositionType>
class OrderedIndexMap
{
public:
    OrderedIndexMap() : _maxPosition(0) {}

    inline void add(IndexType index, PositionType position)
    {
        auto it = _orderedIndices.find(index);
        if (it == _orderedIndices.end())
        {
            std::vector<PositionType> positions(1, position);
            _orderedIndices.insert(std::make_pair(index, positions));
        }
        else
        {
            _orderedIndices[index].push_back(position);
        }
        if (position > _maxPosition)
        {
            _maxPosition = position;
        }
    }

    inline void merge(const std::vector<IndexType> &indicesToMerge)
    {
        if (indicesToMerge.size() < 2)
        {
            return;
        }
        for (auto index : indicesToMerge)
        {
            assert(_orderedIndices.find(index) != _orderedIndices.end());
            if (_orderedIndices.find(index) == _orderedIndices.end())
            {
                std::cout << "OrderedIndexMap::merge: Couldn't find index " + index << std::endl;
                return;
            }
        }

        auto destIt = _orderedIndices.find(indicesToMerge[0]);
        auto &destVector = destIt->second;
        for (size_t index = 1; index < indicesToMerge.size(); ++index)
        {
            auto srcIt = _orderedIndices.find(indicesToMerge[index]);
            for (auto srcIndex : srcIt->second)
            {
                destVector.push_back(srcIndex);
            }
            _orderedIndices.erase(srcIt);
        }
    }

    inline void clear()
    {
        _orderedIndices.clear();
    }

    inline std::vector<IndexType> createVector()
    {
        std::vector<IndexType> v;
        v.resize(_maxPosition + 1);
        IndexType counter = 0;
        for (const auto &indexPair : _orderedIndices)
        {
            for (auto index : indexPair.second)
            {
                v[index] = counter;
            }
            counter++;
        }
        return v;
    }

    // returning iterators would be better
    inline const std::map<IndexType, std::vector<PositionType>> & orderedIndices() const { return _orderedIndices; }

private:
    std::map<IndexType, std::vector<PositionType>> _orderedIndices; // vertex index -> positions in index buffer
    PositionType _maxPosition;
};

// That's from https://stackoverflow.com/questions/5100718/integer-to-hex-string-in-c
template <typename T>
std::string intToHexString(T i)
{
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(sizeof(T) * 2)
           << std::hex << i;
    return stream.str();
}

template <typename T, int N>
Json::Value toJsonArray(T arrayData[N])
{
    Json::Value jResult;
    for (int index = 0; index < N; ++index)
    {
        jResult.append(arrayData[index]);
    }
    return jResult;
}