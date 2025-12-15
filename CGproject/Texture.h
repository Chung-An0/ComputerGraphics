#pragma once
#include "Common.h"

class Texture {
public:
    static GLuint Load(const char* filepath);
    static GLuint LoadCubemap(vector<string> faces);
    static void Bind(GLuint textureID, int unit = 0);
    static void BindCubemap(GLuint cubemapID, int unit = 0);
    static void Unbind();
    static void UnbindCubemap();
};
