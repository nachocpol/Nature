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
	bool mShowGui = false;

	// Debug timing
	glw::GpuTimer mFrameTimer;
	glw::GpuTimer mTerrainTimerReflect;
	glw::GpuTimer mTerrainTimerRefract;
	glw::GpuTimer mTerrainTimerFinal;

	glw::GpuTimer mCloudsFinal;
	glw::GpuTimer mWaterFinal;
	glw::GpuTimer mPostProcessing;

	glw::GpuTimer mBloomFinalTimer;
	glw::GpuTimer mLensFlaresFinal;
	glw::GpuTimer mToneMapFinal;
	glw::GpuTimer mFxaaFinal;

	glw::GpuTimer mSkyReflect;
	glw::GpuTimer mSkyFinal;

    // Scene
    Terrain mTerrain;
    Camera mCamera;
    glw::PassConstants mPassConst;
    glw::RenderTarget mBaseRt;
    glw::Material mBaseMatRt;
    glw::Mesh mBaseQuadRt;
    glm::vec2 mWind;//asfsafsafsafs
    glm::vec3 mSunDirection;
    bool mSimulateDayCycle = false;
    bool mWalkerMode = false;
    float mDayTime = 0.0f;
    float mDayTimeSlider = 0.0f;

    // Clouds
    float mCloudsHeight = 6000.0f;
    float mCloudScaleFactor = 0.0005f;
    int mScatSamples = 4;
    int mScatSamplesLow = 1;
    float mScatSampleDist = 1.2f;
    float mCloudDensity = 0.93f;
    float mCloudCutOff = 0.3f;
    glw::Material mCloudsMat;
    glw::Material mCloudsBlackmat;
    glw::Mesh mCloudsPlane;

    // Water
    float mWaterHeight = 211.0f;
    float mWaterScale = 9216.0f;
    float mWaveSize = 50.0f;
    float mWaveStrenght = 0.2f;
    float mNormStrenght = 1.0f;
    glm::vec2 mWaveSpeed = glm::vec2(0.0005f, 0.0005f);
    glm::vec2 mWaveSpeed2 = glm::vec2(-0.0004f, 0.0004f);
    float mWaterShinyFactor = 500.0f;
    glm::vec3 mSpecColor = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 mWaterTint = glm::vec3(0.23f, 0.35f, 0.57f);
    float mWaterTintFactor = 0.0f;

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

    // Bloom
    glw::RenderTarget mBloomRt;
    glw::RenderTarget mBloomRtV;
    glw::RenderTarget mBloomFinal;
    glw::Material mBloomRtMat;
    glw::Material mBloomMergeMat;
    int kBloomHorizontal = 1;
    int kBloomVertical = 0;
    float mBloomSampleIntensity = 0.15f;

    // Lens flares
    glw::RenderTarget mThresholdRt; 
    glw::Material mThresholdRtMat;
    glm::vec4 mThresholdScale = glm::vec4(0.01f);
    glm::vec4 mThresholdBias = glm::vec4(0.1f);
    glw::RenderTarget mLensFeaturesRt; 
    glw::Material mLensFeaturesRtMat;
    int mGhostSamples = 6;
    float mGhostDispers = 0.3f;
    float mHaloWidth = 0.49f;
    float mChromDistort = 4.0f;
    glw::RenderTarget mLensBlurHRt;
    glw::RenderTarget mLensBlurVRt;
    glw::Texture mLensDustTex;
    glw::Texture mLensStarTex;
    glw::RenderTarget mLensMergeRt;
    glw::Material mLensMergeRtMat;
    glm::mat3 mScaleBias1;
    glm::mat3 mScaleBias2;
    glm::mat3 mRotation;
    glm::mat3 mLensStartTrans;

    // Tone map
    glw::RenderTarget mToneMapRt;
    glw::Material mToneMapRtMat;

    // FXAA
    glw::RenderTarget mFxaaRt;
    glw::Material mFxaaRtMat;

    // God rays
    glw::RenderTarget mGodRaysBlackPRt;
    glw::RenderTarget mGodRaysRt;
    glw::Material mGodRaysMat;
    int mGodRaysSamples = 100;
    float mGodRaysExposure = 0.0003f;
    float mGodRaysDecay = 0.94f;
    float mGodRaysDensity = 0.3f;
    float mGodRaysWeight = 150.0f;
    glm::vec3 mSunNormCoords;
};
