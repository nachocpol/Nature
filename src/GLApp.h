/*
    GLApp.h nachocpol@gmail.com
*/

#pragma once

#include "App.h"
#include "Window.h"
#include "Terrain.h"
#include "Camera.h"
#include "Sky.h"

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
    glw::Material mBaseMatRt;
    glw::Mesh mBaseQuadRt;
    glm::vec2 mWind;//asfsafsafsafs
    glm::vec3 mSunDirection;

    // Clouds
    float mCloudsHeight = 6000.0f;
    float mCloudScaleFactor = 0.0005f;
    int mScatSamples = 4;
    int mScatSamplesLow = 1;
    float mScatSampleDist = 1.2f;
    glw::Material mCloudsMat;
    glw::Mesh mCloudsPlane;

    // Water
    float mWaterHeight = 20.0f;
    float mWaterScale = 20480.0f;
    float mWaveSize = 60.0f;
    float mWaveStrenght = 0.02f;
    float mNormStrenght = 1.0f;
    glm::vec2 mWaveSpeed = glm::vec2(0.001f, 0.0005f);
    glm::vec2 mWaveSpeed2 = glm::vec2(-0.001f, 0.0005f);
    float mWaterShinyFactor = 30.0f;
    glm::vec3 mSpecColor = glm::vec3(0.1f,0.1f,0.1f);
    glm::vec3 mWaterTint = glm::vec3(0.23f, 0.35f, 0.57f);
    float mWaterTintFactor = 0.1f;

    glw::RenderTarget mWaterReflecRt;
    glw::RenderTarget mWaterRefracRt;
    glw::Mesh mWaterMesh;
    glw::Material mWaterMaterial;
    glw::Texture mWaterDuDvTexture;
    glw::Texture mWaterNormTexture;

    // Noise texture
    std::vector<Color> mLutColors;
    glw::Texture mLutTexture;

    // Sky
    Sky mSky;
};
