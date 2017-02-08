/*
    GLApp.h nachocpol@gmail.com
*/

#pragma once

#include "App.h"
#include "Window.h"
#include "Terrain.h"
#include "Camera.h"

class GLApp:public App
{
public:
    GLApp();
    ~GLApp();

private:
    virtual bool Init();
    virtual void Update();
    virtual void Render();
    virtual void RenderUi();
    virtual void Release();

    Window mWindow;
    glm::uvec4 mViewport;

    // Scene
    Terrain mTerrain;
    Camera mCamera;
    glw::PassConstants mPassConst;
    glw::RenderTarget mBaseRt;

    // Clouds
    glw::Material mCloudsFboMat;
    glw::Mesh mCloudsFboQuad;

    // Water
    float mWaterHeight = 5.0f;
    glw::RenderTarget mWaterReflecRt;
    glw::RenderTarget mWaterRefracRt;
};
