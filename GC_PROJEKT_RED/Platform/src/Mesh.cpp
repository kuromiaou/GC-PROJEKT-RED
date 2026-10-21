#include "Platform/Mesh.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace
{
    bool IsSep(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\0'; }

    void ParseFaceToken(const char** p, int* vi, int* ti)
    {
        while (**p == ' ' || **p == '\t') ++(*p);
        *vi = std::atoi(*p);
        *ti = 0;
        while (**p && **p != '/' && !IsSep(**p)) ++(*p);
        if (**p == '/')
        {
            ++(*p);
            if (**p != '/')
            {
                *ti = std::atoi(*p);
                while (**p && **p != '/' && !IsSep(**p)) ++(*p);
            }
            if (**p == '/')
            {
                ++(*p);
                while (**p && !IsSep(**p)) ++(*p);
            }
        }
    }
}

bool Mesh::Load(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
    {
        std::cerr << "Mesh::Load: could not open " << path << "\n";
        return false;
    }

    verts.clear();
    uvs.clear();
    faces.clear();
    faceUvs.clear();

    std::string line;
    while (std::getline(file, line))
    {
        const char* s = line.c_str();
        if (s[0] == 'v' && s[1] == ' ')
        {
            float x, y, z;
            if (std::sscanf(s + 2, "%f %f %f", &x, &y, &z) == 3)
                verts.emplace_back(x, y, z);
        }
        else if (s[0] == 'v' && s[1] == 't' && s[2] == ' ')
        {
            float u, v;
            if (std::sscanf(s + 3, "%f %f", &u, &v) >= 2)
                uvs.push_back({ u, v });
        }
        else if (s[0] == 'f' && s[1] == ' ')
        {
            const char* p = s + 2;
            int v[3], t[3];
            for (int k = 0; k < 3; ++k)
                ParseFaceToken(&p, &v[k], &t[k]);
            for (int k = 0; k < 3; ++k)
            {
                faces.push_back(v[k] - 1);
                faceUvs.push_back(t[k] ? t[k] - 1 : -1);
            }
        }
    }
    return true;
}
