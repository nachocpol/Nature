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
    mSunDirection = glm::vec3(0.0f, 0.2f, 1.0f);
}

GLApp::~GLApp()
{
    
}

bool GLApp::Init()
{
    mViewport = glm::uvec4(0, 0, 1280, 1024);

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
    mTerrain.Init();
    mPassConst.Init();

    // Init scene render target
    std::vector<BasicVertex> vert =
    {
        BasicVertex(-1,-1,0,    0,0),
        BasicVertex(1,-1,0,    1,0),
        BasicVertex(1, 1,0,    1,1),
        BasicVertex(-1, 1,0,    0,1)
    };
    std::vector<unsigned int> ele =
    {
        0,1,2,
        0,2,3
    };
    mBaseRt.Init(glm::vec2(mViewport.z, mViewport.w), true);
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
    mCloudsMat.Init("../data/shaders/clouds.vs",
                    "../data/shaders/clouds.fs");
    mCloudsPlane.Init(wVert, cele);

    // Init water
    mWaterReflecRt.Init(glm::vec2(mViewport.z/2.0f, mViewport.w/2.0f), true);
    mWaterRefracRt.Init(glm::vec2(mViewport.z / 2.0f, mViewport.w / 2.0f), true);
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
    return true;
}

void GLApp::Update()
{
    mRunning = !mWindow.Events();

    mCamera.Update();
    mTerrain.Update(mCamera.CameraFrustrum);

    mPassConst.PView = mCamera.View;
    mPassConst.PProjection = mCamera.Projection;
    mPassConst.PCamPos = mCamera.GetPosition();
    mPassConst.PTime = mTime;
    mPassConst.PCamNear = mCamera.GetNear();
    mPassConst.PCamFar = mCamera.GetFar();
    mPassConst.Update();

    mSky.SunPosition = mSunDirection;
    mTerrain.SunPosition = mSunDirection;
}

void GLApp::Render()
{
    glm::ivec2 ws;
    glfwGetWindowSize(mWindow.GetHandle(), &ws.x, &ws.y);
    if (ws.x != mViewport.z || ws.y != mViewport.w)
    {
        printf(" INFO: Window was resized(%i,%i)\n", ws.x, ws.y);
        mViewport.z = ws.x;
        mViewport.w = ws.y;
        mCamera.Resize((float)ws.x / (float)ws.y);
        // Resize render targets
        mBaseRt.Resize(ws);
        mWaterReflecRt.Resize(ws);
        mWaterRefracRt.Resize(ws);
    }

    glClearColor(0.3f,0.3f,0.3f, 1.0f);
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
        mSky.Render();

        // Terrain
        mTerrain.Render(true, glm::vec4(0.0f, 1.0f, 0.0f, -(mWaterHeight + 0.01f)));

        // Clouds
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        mCloudsMat.Use();
        
        glm::mat4 ctrans = glm::mat4();
        ctrans = glm::translate(ctrans, glm::vec3(50000.0f, mCloudsHeight, 50000.0f));
        ctrans = glm::scale(ctrans, glm::vec3(100000.0f, 0.0f, 100000.0f));

        glw::SetUniformTexture("uLutTexture", mCloudsMat.Id, mLutTexture.Id, 0);
        glw::SetUniform1f("uScaleFactor", mCloudsMat.Id, &mCloudScaleFactor);
        glw::SetTransform(mCloudsMat.Id, &ctrans[0][0]);
        glw::SetUniform3f("uSundir",mCloudsMat.Id, &mSunDirection.x);
        glw::SetUniform1i("uScatSamples", mCloudsMat.Id, &mScatSamplesLow);
        glw::SetUniform1f("uSampleDist", mCloudsMat.Id, &mScatSampleDist);

        mCloudsPlane.Draw();
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
        mTerrain.Render(true, glm::vec4(0.0f, -1.0f, 0.0f, (mWaterHeight ) + 0.01f));
    }
    mWaterRefracRt.Disable();

    // Render scene
    mBaseRt.Enable();
    {
        // Sky
        mSky.Render();

        // Terrain
        mTerrain.Render(false);

        // Water
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        mWaterMaterial.Use();
        glm::mat4 wtrans = glm::mat4();
        wtrans = glm::scale(wtrans, glm::vec3(mWaterScale,1.0f,mWaterScale));
        wtrans = glm::translate(wtrans, glm::vec3(0.0f,mWaterHeight, 0.0f));
        
        unsigned int wp = mWaterMaterial.Id;

        glw::SetUniformTexture("uReflectionTexture", wp, mWaterReflecRt.RenderTexture.Id, 0);
        glw::SetUniformTexture("uRefractionTexture", wp, mWaterRefracRt.RenderTexture.Id, 1);
        glw::SetUniformTexture("uDudvTexture", wp, mWaterDuDvTexture.Id, 2);
        glw::SetUniformTexture("uRefractDepth", wp, mWaterRefracRt.DepthTexture.Id, 3);
        glw::SetUniformTexture("uNormTexture", wp, mWaterNormTexture.Id, 4);
        glw::SetTransform(mWaterMaterial.Id, &wtrans[0][0]);
        glw::SetUniform1f("uWaveSize", wp, &mWaveSize);
        glw::SetUniform1f("uWaveStrength", wp, &mWaveStrenght);
        glw::SetUniform1f("uNormStrenght", wp, &mNormStrenght);
        glw::SetUniform3f("uWaveSpeed", wp, &mWaveSpeed.x);
        glw::SetUniform3f("uWaveSpeed2", wp, &mWaveSpeed2.x);
        glw::SetUniform3f("uSundir", wp, &mSunDirection.x);
        glw::SetUniform1f("uWaterShinyFactor", wp, &mWaterShinyFactor);
        glw::SetUniform3f("uSpecColor", wp, &mSpecColor.x);
        glw::SetUniform3f("uWaterTint", wp, &mWaterTint.x);
        glw::SetUniform1f("uWaterTintFactor", wp, &mWaterTintFactor);

        mWaterMesh.Draw();
        glDisable(GL_BLEND);
        
        // Clouds
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        mCloudsMat.Use();

        glm::mat4 ctrans = glm::mat4();
        ctrans = glm::translate(ctrans, glm::vec3(50000.0f, mCloudsHeight, 50000.0f));
        ctrans = glm::scale(ctrans, glm::vec3(100000.0f, 0.0f, 100000.0f));

        glw::SetUniformTexture("uLutTexture", mCloudsMat.Id, mLutTexture.Id, 0);
        glw::SetUniform1f("uScaleFactor", mCloudsMat.Id, &mCloudScaleFactor);
        glw::SetTransform(mCloudsMat.Id, &ctrans[0][0]);
        glw::SetUniform3f("uSundir", mCloudsMat.Id, &mSunDirection.x);
        glw::SetUniform1i("uScatSamples", mCloudsMat.Id, &mScatSamples);
        glw::SetUniform1f("uSampleDist", mCloudsMat.Id, &mScatSampleDist);

        mCloudsPlane.Draw();
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
    }
    mBaseRt.Disable();

    // Draw
    mBaseMatRt.Use();
    glw::SetUniformTexture("uColorTexture", mBaseMatRt.Id, mBaseRt.RenderTexture.Id, 0);
    mBaseQuadRt.Draw();

    RenderUi();
    mWindow.Swap();
}

void GLApp::RenderUi()
{
    ImGui_ImplGlfwGL3_NewFrame();

    ImGui::Begin("Nature 3.0");
    {
        // Debug info
        ImGui::Text("Time:          %f", mTime);
        ImGui::Text("Delta Time:    %f", mDeltaTime);
        ImGui::Text("FPS:           %i", mFps);
        ImGui::Separator();

        // Parameters
        ImGui::InputFloat3("Sun direction", &mSunDirection.x);
        ImGui::Separator();

        // Camera
        ImGui::Text("Camera");
        ImGui::DragFloat("Camera speed", &mCamera.Speed, 0.1f, 0.01f, 4.0f);
        ImGui::DragFloat("Camera sensitivity", &mCamera.Sensitivity, 0.05f, 0.1f, 1.0f);
        ImGui::Checkbox("Camera Locked", &mCamera.LockMouse);
        float tmpFov = *mCamera.GetFov();
        ImGui::InputFloat("Camera FOV", &tmpFov);
        mCamera.SetFov(tmpFov);
        ImGui::Separator();

        // Water
        ImGui::Text("Water");
        ImGui::InputFloat("Water height", &mWaterHeight);
        ImGui::InputFloat("Wave size", &mWaveSize);
        ImGui::InputFloat("Wave strength", &mWaveStrenght);
        ImGui::InputFloat("Normal strength", &mNormStrenght);
        ImGui::InputFloat3("Wave speed", &mWaveSpeed.x);
        ImGui::InputFloat3("Wave speed 2", &mWaveSpeed2.x);
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
