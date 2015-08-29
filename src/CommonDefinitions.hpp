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
};

void inline safeRelease(IUnknown* comObject)
{
    if(comObject)
    {
        comObject->Release();
    }
}