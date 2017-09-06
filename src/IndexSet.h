//-----------------------------------------------------------------------------
// IndexSet.h
// Created at 2017.09.06 14:07
// License: see LICENSE file
//
// IndexSet structure which contains separated stream indices for the vertex
//-----------------------------------------------------------------------------
#pragma once
#include "stdafx.h"

struct IndexSet
{
    uint32_t controlPoint = -1;
    uint32_t normal = -1;
    uint32_t uv = -1;
    uint32_t tangent = -1;
    uint32_t binormal = -1;
    uint32_t vertexColor = -1;

    bool operator==(const IndexSet& rhs) const
    {
        return controlPoint == rhs.controlPoint &&
            normal == rhs.normal &&
            uv == rhs.uv &&
            tangent == rhs.tangent &&
            binormal == rhs.binormal &&
            vertexColor == rhs.vertexColor;
    }

    bool operator<(const IndexSet& rhs) const
    {
        return controlPoint < rhs.controlPoint ||
            (controlPoint == rhs.controlPoint && normal < rhs.normal) ||
            (controlPoint == rhs.controlPoint && normal == rhs.normal && uv < rhs.uv) ||
            (controlPoint == rhs.controlPoint && normal == rhs.normal && uv == rhs.uv &&
                tangent < rhs.tangent) ||
                (controlPoint == rhs.controlPoint && normal == rhs.normal && uv == rhs.uv &&
                    tangent == rhs.tangent && binormal < rhs.binormal) ||
                    (controlPoint == rhs.controlPoint && normal == rhs.normal && uv == rhs.uv &&
                        tangent == rhs.tangent && binormal == rhs.binormal && vertexColor < rhs.vertexColor);
    }
};