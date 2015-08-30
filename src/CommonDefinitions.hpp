#pragma once

#include <Windows.h>
/*
    This file is basically a collection of utility methods, structs and constants that may 
    be used across the code.
*/

const int kScreenWidth = 800;
const int kScreenHeight = 600;

struct Vertex
{
    FLOAT x, y, z; // Position
    FLOAT color[4];
    Vertex(FLOAT nx, FLOAT ny, FLOAT nz, FLOAT r, FLOAT g, FLOAT b, FLOAT alpha)
        : x(nx), y(ny), z(nz), color()
    {
        color[0] = r; color[1] = g; color[2] = b; color[3] = alpha;
    }
};

void inline safeRelease(IUnknown* comObject)
{
    if(comObject)
    {
        comObject->Release();
    }
}