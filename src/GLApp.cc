/*
    GLApp.cc nachocpol@gmail.com
*/

#define GLEW_STATIC
#include "GL/glew.h"
#include <iostream>
#include "Input.h"
#include "GlApp.h"
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/random.hpp"

GLApp::GLApp()
{
    mSunDirection = glm::vec3(0.0f, 0.25f, -1.0f);
}

GLApp::~GLApp()
{
    
}

bool GLApp::Init()
{
    mViewport = glm::uvec4(0, 0, 1050, 780);
    glm::vec2 initWindowSize = glm::vec2(mViewport.z, mViewport.w);

    // Initialize window
    if (!mWindow.Init("Nature 3.0", glm::uvec2(mViewport.z, mViewport.w)))
    {
        printf("ERROR: Failed window creation.\n");
        return false;
    }

    // Initialize opengl
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) 
    {
        printf("ERROR: Failed to initialize opengl.\n");
        return false;
    }

    // Init IMGUI
    ImGui_ImplGlfwGL3_Init(mWindow.GetHandle(), true);

    mCamera.Init((float)mViewport.z / (float)mViewport.w);

    // Init terrain
    mTerrain.Init();
    mTerrain.CamPos = mCamera.GetPositionPtr();

    // Init pass constants (UBO)
    mPassConst.Init();

    // Init scene render target
    std::vector<BasicVertex> vert =
    {
        BasicVertex(-1,-1,-1,    0,0),
        BasicVertex(1,-1,-1,    1,0),
        BasicVertex(1, 1,-1,    1,1),
        BasicVertex(-1, 1,-1,    0,1)
    };
    std::vector<unsigned int> ele =
    {
        0,1,2,
        0,2,3
    };
    mBaseRt.Init(glm::vec2(initWindowSize), true);
    mBaseMatRt.Init("../data/shaders/rt.vs", "../data/shaders/rt.fs");
    mBaseQuadRt.Init(vert, ele);

    // Init clouds 
    std::vector<BasicVertex> wVert =
    {
        BasicVertex(-1.0f,0.0f,-1.0f,     0.0f,0.0f),
        BasicVertex(1.0f,0.0f,-1.0f,     1.0f,0.0f),
        BasicVertex(1.0f,0.0f, 1.0f,     1.0f,1.0f),
        BasicVertex(-1.0f,0.0f, 1.0f,     0.0f,1.0f)
    };
    std::vector<unsigned int> cele =
    {
        0,2,1,
        0,3,2
    };
    mCloudsMat.Init
    (
        "../data/shaders/clouds/clouds.vs",
        "../data/shaders/clouds/clouds.fs"
    );
    mCloudsBlackmat.Init
    (
        "../data/shaders/clouds/clouds.vs",
        "../data/shaders/clouds/cloudsblack.fs"
    );
    mCloudsPlane.Init(wVert, cele);

    // Init water
    mWaterReflecRt.Init(initWindowSize / 2.0f, true);
    mWaterRefracRt.Init(initWindowSize / 2.0f, true);
    mWaterMesh.Init(wVert, cele);
    mWaterMaterial.Init("../data/shaders/water.vs", "../data/shaders/water.fs");
    mWaterDuDvTexture.Init(TextureDef("../data/textures/wdudv.png", glm::vec2(0.0f), TextureUsage::kTexturing));
    mWaterNormTexture.Init(TextureDef("../data/textures/wnorm.png", glm::vec2(0.0f), TextureUsage::kTexturing));

    // Init lut texture
    mLutColors.resize(256 * 256 * 4);
    for (unsigned int y = 0; y < 256; y++)
    {
        for (unsigned int x = 0; x < 256; x++)
        {
            unsigned int idx = y * 256 + x;
            mLutColors[idx].R = (unsigned char)glm::clamp(int(glm::linearRand(0.0f, 1.0f) * 255.0f), 0, 255);
            mLutColors[idx].B = (unsigned char)glm::clamp(int(glm::linearRand(0.0f, 1.0f) * 255.0f), 0, 255);
        }
    }
    for (unsigned int y = 0; y < 256; y++)
    {
        for (unsigned int x = 0; x < 256; x++)
        {
            int x2 = (x - 37) & 255;
            int y2 = (y - 17) & 255;
            unsigned int idx = y2 * 256 + x2;
            mLutColors[idx].G = mLutColors[idx].R;
            mLutColors[idx].A = mLutColors[idx].B;
        }
    }
    TextureDef mLutDef = TextureDef("", glm::vec2(256, 256), TextureUsage::kSampling);
    mLutDef.Data = &mLutColors[0].R;
    mLutTexture.Init(mLutDef);
    mTerrain.LutTexture = &mLutTexture;

    // Init sky
    mSky.Init();

    // Bloom
    mBloomRt.Init(initWindowSize / 4.0f);
    mBloomRtV.Init(initWindowSize / 4.0f);
    mBloomFinal.Init(initWindowSize);
    mBloomRtMat.Init
    (
        "../data/shaders/bloom/bloomblur.vs",
        "../data/shaders/bloom/bloomblur.fs"
    );
    mBloomMergeMat.Init
    (
        "../data/shaders/bloom/bloommerge.vs",
        "../data/shaders/bloom/bloommerge.fs"
    );

    // Lens flares
    // Downsample and threshold
    mThresholdRt.Init(initWindowSize / 4.0f);
    mThresholdRtMat.Init
    (
        "../data/shaders/lensflares/lfthreshold.vs",
        "../data/shaders/lensflares/lfthreshold.fs"
    );
    // Feature generation
    mLensFeaturesRt.Init(initWindowSize / 4.0f);
    mLensFeaturesRtMat.Init
    (
        "../data/shaders/lensflares/lffeatures.vs",
        "../data/shaders/lensflares/lffeatures.fs"
    );
    mLensBlurHRt.Init(initWindowSize / 4.0f);
    mLensBlurVRt.Init(initWindowSize / 4.0f);
    mLensDustTex.Init(TextureDef("../data/textures/lensdust.jpg", glm::vec2(0.0f), TextureUsage::kTexturing));
    mLensStarTex.Init(TextureDef("../data/textures/lensstar.jpg", glm::vec2(0.0f), TextureUsage::kTexturing));
    mLensMergeRt.Init(initWindowSize);
    mLensMergeRtMat.Init
    (
        "../data/shaders/lensflares/lmerge.vs",
        "../data/shaders/lensflares/lmerge.fs"
    );

    // Tone map
    mToneMapRt.Init(initWindowSize);
    mToneMapRtMat.Init
    (
        "../data/shaders/tonemap/tonemap.vs",
        "../data/shaders/tonemap/tonemap.fs"
    );

    // FXAA
    mFxaaRt.Init(initWindowSize);
    mFxaaRtMat.Init
    (
        "../data/shaders/aa/fxaa.vs",
        "../data/shaders/aa/fxaa.fs"
    );

    // God rays
    mGodRaysRt.Init(initWindowSize / 1.0f);
    mGodRaysBlackPRt.Init(initWindowSize);
    mGodRaysMat.Init
    (
        "../data/shaders/godrays/godrays.vs",
        "../data/shaders/godrays/godrays.fs"
    );

    // Set initial camera position
    mCamera.GetPositionPtr()->y = mTerrain.GetHeight(mCamera.GetPosition().x, mCamera.GetPosition().z) + 2.0f;

	// Init gpu debug timers
	mFrameTimer.Init();
	mTerrainTimerReflect.Init();
	mTerrainTimerRefract.Init();
	mTerrainTimerFinal.Init();

	mCloudsFinal.Init();
	mWaterFinal.Init();
	mPostProcessing.Init();

	mBloomFinalTimer.Init();
	mLensFlaresFinal.Init();
	mToneMapFinal.Init();
	mFxaaFinal.Init();

	mSkyReflect.Init();
	mSkyFinal.Init();

	// GL setup
	//glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	//glClearDepth(0.0f);
	//glDepthFunc(GL_GREATER);

    return true;
}

void GLApp::Update()
{
    mRunning = !mWindow.Events();

    if (mSimulateDayCycle)
    {
        mSunDirection.y = cos(mTime * 0.5);
        mSunDirection.x = sin(mTime * 0.5);
    }
    else
    {
        mDayTime = glm::mix(0.0f, glm::pi<float>(), mDayTimeSlider);
        mSunDirection.y = sin(mDayTime);
        mSunDirection.x = cos(mDayTime);
    }

    mCamera.Update();
    mTerrain.Update(mCamera.CameraFrustrum);

    mPassConst.PView = mCamera.View;
    mPassConst.PProjection = mCamera.Projection;
    mPassConst.PCamPos = mCamera.GetPosition();
    mPassConst.PTime = mTime;
    mPassConst.PCamNear = mCamera.GetNear();
    mPassConst.PCamFar = mCamera.GetFar();
    mPassConst.PAspect = (float)mViewport.z / (float)mViewport.w;
    mPassConst.Update();    

    mSky.SunPosition = mSunDirection;
    mTerrain.SunPosition = mSunDirection;

    // Firs person walker
    if (mWalkerMode)
    {
        glm::vec3 curCamPos = mCamera.GetPosition();
        float terrainHeight = mTerrain.GetHeight(mCamera.GetPosition().x, mCamera.GetPosition().z);
        if (terrainHeight < mWaterHeight)terrainHeight = mWaterHeight;
        curCamPos.y = glm::mix
        (
            curCamPos.y, 
            terrainHeight + 10.0f, 
            mDeltaTime * 10.0f
        );
        mCamera.SetPosition(curCamPos);
    }

    // Show/hide gui btn
    if (Input::GetInstance()->GetKey(INPUT_KEY_0))
    {
        mShowGui = !mShowGui;
    }
}

void GLApp::Render()
{
    // Handle resizing
    glm::ivec2 ws;
    glfwGetWindowSize(mWindow.GetHandle(), &ws.x, &ws.y);
    if (ws.x != mViewport.z || ws.y != mViewport.w)
    {
        printf(" INFO: Window was resized(%i,%i)\n", ws.x, ws.y);
        mViewport.z = ws.x;
        mViewport.w = ws.y;
        mCamera.Resize((float)ws.x / (float)ws.y);

        glm::vec2 windowDown4 = ws;
        windowDown4 *= 0.25f;
        glm::vec2 windowDown2 = ws;
        windowDown2 *= 0.5f;

        // Resize render targets
        mBaseRt.Resize(ws);
        mWaterReflecRt.Resize(windowDown2);
        mWaterRefracRt.Resize(windowDown2);
        mThresholdRt.Resize(windowDown4);
        mLensFeaturesRt.Resize(windowDown4);
        mBloomRt.Resize(windowDown4);
        mBloomRtV.Resize(windowDown4);
        mBloomFinal.Resize(ws);
        mLensBlurHRt.Resize(windowDown4);
        mLensBlurVRt.Resize(windowDown4);
        mLensMergeRt.Resize(ws);
        mToneMapRt.Resize(ws);
        mFxaaRt.Resize(ws);
        mGodRaysBlackPRt.Resize(ws);
        mGodRaysRt.Resize(ws);
    }

	mFrameTimer.Start();

    // Set clear color to black so the god rays can be blended with
    // the scene
    glClearColor(0.0f,0.0f,0.0f, 1.0f);
    glViewport( mViewport.x, mViewport.y,
                mViewport.z, mViewport.w);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Water reflections
    float wOff = mCamera.GetPosition().y - mWaterHeight;
    mCamera.Move(0.0f, -(wOff * 2.0f), 0.0f);
    mCamera.SetPitch(-mCamera.GetPitch());
    mCamera.UpdateView();
    mPassConst.PView = mCamera.View;
    mPassConst.Update();
    mWaterReflecRt.Enable();
    {
        // Sky
		mSkyReflect.Start();
        mSky.Render();
		mSkyReflect.End();

        // Terrain
		mTerrainTimerReflect.Start();
        mTerrain.Render(true, glm::vec4(0.0f, 1.0f, 0.0f, -(mWaterHeight + 0.1f)));
		mTerrainTimerReflect.End();

        // Clouds
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        mCloudsMat.Use();
        
        glm::mat4 ctrans = glm::mat4();
        ctrans = glm::translate(ctrans, glm::vec3(0.0f, mCloudsHeight, 0.0f));
        ctrans = glm::scale(ctrans, glm::vec3(100000.0f, 0.0f, 100000.0f));

        glw::SetUniformTexture("uLutTexture", mCloudsMat.Id, mLutTexture.Id, 0);
        glw::SetUniform1f("uScaleFactor", mCloudsMat.Id, &mCloudScaleFactor);
        glw::SetTransform(mCloudsMat.Id, &ctrans[0][0]);
        glw::SetUniform3f("uSundir",mCloudsMat.Id, &mSunDirection.x);
        glw::SetUniform1i("uScatSamples", mCloudsMat.Id, &mScatSamplesLow);
        glw::SetUniform1f("uSampleDist", mCloudsMat.Id, &mScatSampleDist);
        glw::SetUniform1f("uCutOff", mCloudsMat.Id, &mCloudCutOff);
        glw::SetUniform1f("uDensity", mCloudsMat.Id, &mCloudDensity);

        mCloudsPlane.Render();

        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
    }
    mWaterReflecRt.Disable();

    // Water refractions
    mCamera.Move(0.0f, 2 * wOff, 0.0f);
    mCamera.SetPitch(-mCamera.GetPitch());
    mCamera.UpdateView();
    mPassConst.PView = mCamera.View;
    mPassConst.Update();
    mWaterRefracRt.Enable();
    {
		mTerrainTimerRefract.Start();
        mTerrain.Render(true, glm::vec4(0.0f, -1.0f, 0.0f, (mWaterHeight ) + 0.1f));
		mTerrainTimerRefract.End();
    }
    mWaterRefracRt.Disable();

    // Render scene
    mBaseRt.Enable();
    {
        // Sky
		mSkyFinal.Start();
        mSky.Render();
		mSkyFinal.End();

        // Terrain
		mTerrainTimerFinal.Start();
        mTerrain.Render(false);
		mTerrainTimerFinal.End();

        // Water
		mWaterFinal.Start();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        mWaterMaterial.Use();
        glm::mat4 wtrans = glm::mat4();
        wtrans = glm::translate(wtrans, glm::vec3(mWaterScale,mWaterHeight, mWaterScale));
        wtrans = glm::scale(wtrans, glm::vec3(mWaterScale,1.0f,mWaterScale));
        
        unsigned int wp = mWaterMaterial.Id;

        glw::SetUniformTexture("uReflectionTexture", wp, mWaterReflecRt.RenderTexture.Id, 0);
        glw::SetUniformTexture("uRefractionTexture", wp, mWaterRefracRt.RenderTexture.Id, 1);
        glw::SetUniformTexture("uDudvTexture", wp, mWaterDuDvTexture.Id, 2);
        glw::SetUniformTexture("uNormTexture", wp, mWaterNormTexture.Id, 3);
        glw::SetUniformTexture("uTerrainTexture", wp, mTerrain.GetHeightMapId(), 4);
        glw::SetTransform(mWaterMaterial.Id, &wtrans[0][0]);
        glw::SetUniform1f("uWaveSize", wp, &mWaveSize);
        glw::SetUniform1f("uWaveStrength", wp, &mWaveStrenght);
        glw::SetUniform1f("uNormStrenght", wp, &mNormStrenght);
        glw::SetUniform2f("uWaveSpeed", wp, &mWaveSpeed.x);
        glw::SetUniform2f("uWaveSpeed2", wp, &mWaveSpeed2.x);
        glw::SetUniform3f("uSundir", wp, &mSunDirection.x);
        glw::SetUniform1f("uWaterShinyFactor", wp, &mWaterShinyFactor);
        glw::SetUniform3f("uSpecColor", wp, &mSpecColor.x);
        glw::SetUniform3f("uWaterTint", wp, &mWaterTint.x);
        glw::SetUniform1f("uWaterTintFactor", wp, &mWaterTintFactor);
        glw::SetUniform1f("uTerrainHeightScale", wp, &mTerrain.HeightScale);
        glw::SetUniform1f("uTerrainScale", wp, &mTerrain.MapScale);

        mWaterMesh.Render();
        glDisable(GL_BLEND);
		mWaterFinal.End();

        // Clouds
		mCloudsFinal.Start();
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        mCloudsMat.Use();

        glm::mat4 ctrans = glm::mat4();
        ctrans = glm::translate(ctrans, glm::vec3(25000.0f, mCloudsHeight, 25000.0f));
        ctrans = glm::scale(ctrans, glm::vec3(100000.0f, 0.0f, 100000.0f));

        glw::SetUniformTexture("uLutTexture", mCloudsMat.Id, mLutTexture.Id, 0);
        glw::SetUniform1f("uScaleFactor", mCloudsMat.Id, &mCloudScaleFactor);
        glw::SetTransform(mCloudsMat.Id, &ctrans[0][0]);
        glw::SetUniform3f("uSundir", mCloudsMat.Id, &mSunDirection.x);
        glw::SetUniform1i("uScatSamples", mCloudsMat.Id, &mScatSamples);
        glw::SetUniform1f("uSampleDist", mCloudsMat.Id, &mScatSampleDist);
        glw::SetUniform1f("uCutOff", mCloudsMat.Id, &mCloudCutOff);
        glw::SetUniform1f("uDensity", mCloudsMat.Id, &mCloudDensity);

        mCloudsPlane.Render();

        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
		mCloudsFinal.End();
    }
    mBaseRt.Disable();

    // God rays (black pass)
    mGodRaysBlackPRt.Enable();
    {
        // Sky
        mSky.Render();

        // Terrain
        mTerrain.Render(false, glm::vec4(0.0f), true);

        // Clouds
		/*
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        mCloudsBlackmat.Use();

        glm::mat4 ctrans = glm::mat4();
        ctrans = glm::translate(ctrans, glm::vec3(0.0f, mCloudsHeight, 0.0f));
        ctrans = glm::scale(ctrans, glm::vec3(100000.0f, 0.0f, 100000.0f));

        glw::SetUniformTexture("uLutTexture", mCloudsBlackmat.Id, mLutTexture.Id, 0);
        glw::SetUniform1f("uScaleFactor", mCloudsBlackmat.Id, &mCloudScaleFactor);
        glw::SetTransform(mCloudsBlackmat.Id, &ctrans[0][0]);
        glw::SetUniform3f("uSundir", mCloudsBlackmat.Id, &mSunDirection.x);
        glw::SetUniform1i("uScatSamples", mCloudsBlackmat.Id, &mScatSamplesLow);
        glw::SetUniform1f("uSampleDist", mCloudsBlackmat.Id, &mScatSampleDist);

        mCloudsPlane.Render();
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
		*/
    }
    mGodRaysBlackPRt.Disable();
    

    // God rays
    mGodRaysRt.Enable();
    {
        // Sun screen space
        glm::vec3 sunEyePos = mCamera.GetPosition() + (mSunDirection);
        glm::vec4 sunProj =  mCamera.Projection * mCamera.View * glm::vec4(sunEyePos, 1.0f);
        // Save projected z
        float sunZ = sunProj.z;
        sunProj /= sunProj.w;
        sunProj = (sunProj + 1.0f) * 0.5f;
        mSunNormCoords = glm::vec3(sunProj.x, sunProj.y,sunZ);

        unsigned int grId = mGodRaysMat.Id;
        mGodRaysMat.Use();
        glw::SetUniformTexture("uColorTexture", grId, mGodRaysBlackPRt.RenderTexture.Id, 0);
        glw::SetUniform3f("uSunScreenSpace", grId,&mSunNormCoords.x);
        glw::SetUniform3f("uSunDirection", grId, &mSunDirection.x);
        glw::SetUniform1i("uSamples", grId, &mGodRaysSamples);
        glw::SetUniform1f("uExposure", grId, &mGodRaysExposure);
        glw::SetUniform1f("uDecay", grId, &mGodRaysDecay);
        glw::SetUniform1f("uDensity", grId, &mGodRaysDensity);
        glw::SetUniform1f("uWeight", grId, &mGodRaysWeight);
        mBaseQuadRt.Render();
    }
    mGodRaysRt.Disable();
   

	mPostProcessing.Start();
    // Bloom
    // Horizontal blur
	mBloomFinalTimer.Start();
    mBloomRt.Enable();
    {
        mBloomRtMat.Use();
        glw::SetUniformTexture("uColorTexture", mBloomRtMat.Id, mBaseRt.RenderTexture.Id, 0);
        glw::SetUniform1i("uHorizontal", mBloomRtMat.Id, &kBloomHorizontal);
        glw::SetUniform1f("uIntensity", mBloomRtMat.Id, &mBloomSampleIntensity);
        mBaseQuadRt.Render();
    }
    mBloomRt.Disable();

    // Vertical blur
    mBloomRtV.Enable();
    {
        mBloomRtMat.Use();
        glw::SetUniformTexture("uColorTexture", mBloomRtMat.Id, mBloomRt.RenderTexture.Id, 0);
        glw::SetUniform1i("uHorizontal", mBloomRtMat.Id, &kBloomVertical);
        glw::SetUniform1f("uIntensity", mBloomRtMat.Id, &mBloomSampleIntensity);
        mBaseQuadRt.Render();
    }
    mBloomRtV.Disable();

    // Bloom merge
    mBloomFinal.Enable();
    {
        mBloomMergeMat.Use();
        glw::SetUniformTexture("uColorTexture", mBloomMergeMat.Id, mBaseRt.RenderTexture.Id, 0);
        glw::SetUniformTexture("uBloomTexture", mBloomMergeMat.Id, mBloomRtV.RenderTexture.Id, 1);
        mBaseQuadRt.Render();
    }
    mBloomFinal.Disable();
	mBloomFinalTimer.End();

    // Lens flares
    // Threshold
	mLensFlaresFinal.Start();
    mThresholdRt.Enable();
    {
        mThresholdRtMat.Use();
        glw::SetUniform4f("uScale", mThresholdRtMat.Id, &mThresholdScale.x);
        glw::SetUniform4f("uBias", mThresholdRtMat.Id, &mThresholdBias.x);
        glw::SetUniformTexture("uColorTexture", mThresholdRtMat.Id, mBloomFinal.RenderTexture.Id, 0);
        mBaseQuadRt.Render();
    }
    mThresholdRt.Disable();

    // Features
    mLensFeaturesRt.Enable();
    {
        mLensFeaturesRtMat.Use();
        glw::SetUniform1i("uGhostSamples", mLensFeaturesRtMat.Id, &mGhostSamples);
        glw::SetUniform1f("uGhostDispers", mLensFeaturesRtMat.Id, &mGhostDispers);
        glw::SetUniform1f("uHaloWidth", mLensFeaturesRtMat.Id, &mHaloWidth);
        glw::SetUniform1f("uDistortion", mLensFeaturesRtMat.Id, &mChromDistort);
        glw::SetUniformTexture("uColorTexture", mLensFeaturesRtMat.Id, mThresholdRt.RenderTexture.Id, 0);
        mBaseQuadRt.Render();
    }
    mLensFeaturesRt.Disable();

    // Lens blur horizontal
    static float kIntensityHack = 1.0f;
    mLensBlurHRt.Enable();
    {
        mBloomRtMat.Use();
        glw::SetUniformTexture("uColorTexture", mBloomRtMat.Id, mLensFeaturesRt.RenderTexture.Id, 0);
        glw::SetUniform1i("uHorizontal", mBloomRtMat.Id, &kBloomHorizontal);
        glw::SetUniform1f("uIntensity", mBloomRtMat.Id, &kIntensityHack);
        mBaseQuadRt.Render();
    }
    mLensBlurHRt.Disable();

    // Lens blur vertical
    mLensBlurVRt.Enable();
    {
        mBloomRtMat.Use();
        glw::SetUniformTexture("uColorTexture", mBloomRtMat.Id, mLensBlurHRt.RenderTexture.Id, 0);
        glw::SetUniform1i("uHorizontal", mBloomRtMat.Id, &kBloomVertical);
        glw::SetUniform1f("uIntensity", mBloomRtMat.Id, &kIntensityHack);
        mBaseQuadRt.Render();
    }
    mLensBlurVRt.Disable();

    // Lens merge
    mLensMergeRt.Enable();
    {
        mLensMergeRtMat.Use();
        glw::SetUniformTexture("uColorTexture", mLensMergeRtMat.Id, mBloomFinal.RenderTexture.Id, 0);
        glw::SetUniformTexture("uLens", mLensMergeRtMat.Id, mLensBlurVRt.RenderTexture.Id, 1);
        glw::SetUniformTexture("uLensDust", mLensMergeRtMat.Id, mLensDustTex.Id, 2);
        glw::SetUniformTexture("uLensStar", mLensMergeRtMat.Id, mLensStarTex.Id, 3);
        mBaseQuadRt.Render();
    }
    mLensMergeRt.Disable();
	mLensFlaresFinal.End();

    // Tone map & god rays merge
	mToneMapFinal.Start();
    mToneMapRt.Enable();
    {
        mToneMapRtMat.Use();
        glw::SetUniformTexture("uColorTexture", mToneMapRtMat.Id, mLensMergeRt.RenderTexture.Id, 0);
        glw::SetUniformTexture("uGodRaysTexture", mToneMapRtMat.Id, mGodRaysRt.RenderTexture.Id, 1);
        mBaseQuadRt.Render();
    }
    mToneMapRt.Disable();
	mToneMapFinal.End();

    // Fxaa
	mFxaaFinal.Start();
    mFxaaRt.Enable();
    {
        mFxaaRtMat.Use();
        glw::SetUniformTexture("uColorTexture", mFxaaRtMat.Id, mToneMapRt.RenderTexture.Id, 0);
        mBaseQuadRt.Render();
    }
    mFxaaRt.Disable();
	mFxaaFinal.End();
	mPostProcessing.End();

    // Final draw
    mBaseMatRt.Use();
    glw::SetUniformTexture("uColorTexture", mBaseMatRt.Id, mFxaaRt.RenderTexture.Id, 0);
    mBaseQuadRt.Render();


    if(!mShowGui)
        RenderUi();

    mWindow.Swap();
	mFrameTimer.End();
}

void GLApp::RenderUi()
{	
    ImGui_ImplGlfwGL3_NewFrame();

	ImGui::Begin("Frame timing");
	{
		ImGui::Text("Frame time:                %f", mFrameTimer.TimerMili);
		ImGui::Separator();
		ImGui::Text("Terrain reflect time:      %f", mTerrainTimerReflect.TimerMili);
		ImGui::Text("Terrain refract time:      %f", mTerrainTimerRefract.TimerMili);
		ImGui::Text("Terrain final time :       %f", mTerrainTimerFinal.TimerMili);
		ImGui::Separator();
		ImGui::Text("Clouds final time:         %f", mCloudsFinal.TimerMili);
		ImGui::Separator();
		ImGui::Text("Water final time:          %f", mWaterFinal.TimerMili);
		ImGui::Separator();
		ImGui::Text("Postprocessing final time: %f", mPostProcessing.TimerMili);
		ImGui::Text("Bloom final time:          %f", mBloomFinalTimer.TimerMili);
		ImGui::Text("Lens flare time:           %f", mLensFlaresFinal.TimerMili);
		ImGui::Text("ToneMap time:              %f", mToneMapFinal.TimerMili);
		ImGui::Text("Fxaa time:                 %f", mFxaaFinal.TimerMili);
		ImGui::Separator();
		ImGui::Text("Sky reflect time:          %f", mSkyReflect.TimerMili);
		ImGui::Text("Sky final time:            %f", mSkyFinal.TimerMili);
		ImGui::Separator();
	}
	ImGui::End();

    ImGui::Begin("Postprocessing");
    {
        // Bloom
        ImGui::Text("Bloom");
        ImGui::InputFloat("Bloom sample intensity", &mBloomSampleIntensity);
        ImGui::Separator();
        // Lens flares
        ImGui::Text("Lens flares");
        ImGui::InputFloat4("Threshold scale", &mThresholdScale.x);
        ImGui::InputFloat4("Threshold bias", &mThresholdBias.x);
        ImGui::InputInt("Ghost samples", &mGhostSamples);
        ImGui::InputFloat("Ghost dispers", &mGhostDispers);
        ImGui::InputFloat("Halo width", &mHaloWidth);
        ImGui::InputFloat("Chromatic distortion", &mChromDistort);
        ImGui::Separator();
        // God rays
        ImGui::Text("God rays");
        ImGui::InputInt("God rays samples", &mGodRaysSamples);
        ImGui::InputFloat("God rays exposure", &mGodRaysExposure);
        ImGui::InputFloat("God rays decay", &mGodRaysDecay);
        ImGui::InputFloat("God rays density", &mGodRaysDensity);
        ImGui::InputFloat("God rays weight", &mGodRaysWeight);
        ImGui::Separator();
    }
    ImGui::End();

    ImGui::Begin("Nature 3.0");
    {
        // Debug info
        ImGui::Text("Time:          %f", mTime);
        ImGui::Text("Delta Time:    %f", mDeltaTime);
        ImGui::Text("FPS:           %i", mFps);
        ImGui::Separator();

        // Parameters
        ImGui::InputFloat3("Sun direction", &mSunDirection.x);
        ImGui::Checkbox("Simulate day cycle", &mSimulateDayCycle);
        ImGui::SliderFloat("Day time", &mDayTimeSlider, 0.0f, 1.0f);
        ImGui::Separator();

        // Camera
        ImGui::Text("Camera");
		ImGui::Text("Camera position:%f,%f,%f", mCamera.GetPosition().x, mCamera.GetPosition().y, mCamera.GetPosition().z);
        ImGui::DragFloat("Camera speed", &mCamera.Speed, 0.1f, 0.01f, 4.0f);
        ImGui::DragFloat("Camera sensitivity", &mCamera.Sensitivity, 0.05f, 0.1f, 1.0f);
        ImGui::Checkbox("Camera Locked", &mCamera.LockMouse);
        ImGui::Checkbox("Walker mode", &mWalkerMode);
        float tmpFov = *mCamera.GetFov();
        ImGui::InputFloat("Camera FOV", &tmpFov);
        mCamera.SetFov(tmpFov);
		ImGui::Checkbox("Update fustrum", &mCamera.UpdateFustrum);
        ImGui::Separator();

        // Water
        ImGui::Text("Water");
        ImGui::InputFloat("Water height", &mWaterHeight);
        ImGui::InputFloat("Wave size", &mWaveSize);
        ImGui::InputFloat("Wave strength", &mWaveStrenght);
        ImGui::InputFloat("Normal strength", &mNormStrenght);
        ImGui::InputFloat2("Wave speed", &mWaveSpeed.x);
        ImGui::InputFloat2("Wave speed 2", &mWaveSpeed2.x);
        ImGui::InputFloat("Water specular factor", &mWaterShinyFactor);
        ImGui::InputFloat3("Water specular color", &mSpecColor.x);
        ImGui::InputFloat3("Water tint", &mWaterTint.x);
        ImGui::InputFloat("Water tint factor", &mWaterTintFactor);
        ImGui::Separator();

        // Clouds
        ImGui::Text("Clouds");
        ImGui::InputFloat("Clouds height", &mCloudsHeight);
        ImGui::InputFloat("Clouds scale factor", &mCloudScaleFactor);
        ImGui::InputInt("Clouds scatter samples", &mScatSamples);
        ImGui::InputFloat("Clouds scat sample dist", &mScatSampleDist);
        ImGui::SliderFloat("Clouds cutoff", &mCloudCutOff, 0.0f, 1.0f);
        ImGui::SliderFloat("Clouds density", &mCloudDensity, 0.0f, 1.0f);
        ImGui::Separator();
    }
    ImGui::End();

    mTerrain.RenderUi();
    mSky.RenderUi();

    ImGui::Render();
}

void GLApp::Release()
{
    ImGui_ImplGlfwGL3_Shutdown();
    mWindow.Release();
}
