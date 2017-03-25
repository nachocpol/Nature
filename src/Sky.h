/*
    Sky.h nachocpol@gmail.com
*/

#pragma once
#include "GLWrapper.h"

class Sky
{
public:
    Sky();
    ~Sky();
    void Init();
    void Render();
    void RenderUi();

    glm::vec3 SunPosition;

private:
    glw::Mesh mSkyMesh;
    glw::Material mSkyMaterial;
};

