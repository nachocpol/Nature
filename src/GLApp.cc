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

GLApp::GLApp()
{
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

    glClearColor(0.45f, 0.75f, 0.95f, 1.0f);
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
        mTerrain.Render(true, glm::vec4(0.0f, 1.0f, 0.0f, -(mWaterHeight + 0.01f)));

        // Clouds
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        mCloudsMat.Use();
        glm::mat4 ctrans = glm::mat4();
        ctrans = glm::translate(ctrans, glm::vec3(512.0f, mCloudsHeight, 512.0f));
        ctrans = glm::scale(ctrans, glm::vec3(4096.0f));
        glw::SetTransform(mCloudsMat.Id, &ctrans[0][0]);
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
        mTerrain.Render(true, glm::vec4(0.0f, -1.0f, 0.0f, mWaterHeight + 0.01f));
    }
    mWaterRefracRt.Disable();

    // Render scene
    mBaseRt.Enable();
    {
        // Terrain
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        mTerrain.Render(false);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        // Water
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        mWaterMaterial.Use();
        glm::mat4 wtrans = glm::mat4();
        wtrans = glm::translate(wtrans, glm::vec3(512.0f,mWaterHeight, 512.0f));
        wtrans = glm::scale(wtrans, glm::vec3(mWaterScale));
        glw::SetTransform(mWaterMaterial.Id, &wtrans[0][0]);

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, mWaterReflecRt.RenderTexture.Id);
        int loc = glGetUniformLocation(mWaterMaterial.Id, "uReflectionTexture");
        glUniform1i(loc, 0);

        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, mWaterRefracRt.RenderTexture.Id);
        loc = glGetUniformLocation(mWaterMaterial.Id, "uRefractionTexture");
        glUniform1i(loc, 1);

        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, mWaterDuDvTexture.Id);
        loc = glGetUniformLocation(mWaterMaterial.Id, "uDudvTexture");
        glUniform1i(loc, 2);

        glActiveTexture(GL_TEXTURE0 + 3);
        glBindTexture(GL_TEXTURE_2D, mWaterRefracRt.DepthTexture.Id);
        loc = glGetUniformLocation(mWaterMaterial.Id, "uRefractDepth");
        glUniform1i(loc, 3);

        glActiveTexture(GL_TEXTURE0 + 4);
        glBindTexture(GL_TEXTURE_2D, mWaterNormTexture.Id);
        loc = glGetUniformLocation(mWaterMaterial.Id, "uNormTexture");
        glUniform1i(loc, 4);

        mWaterMesh.Draw();
        glDisable(GL_BLEND);

        // Clouds
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        mCloudsMat.Use();
        glm::mat4 ctrans = glm::mat4();
        ctrans = glm::translate(ctrans, glm::vec3(512.0f, mCloudsHeight, 512.0f));
        ctrans = glm::scale(ctrans, glm::vec3(4096.0f));
        glw::SetTransform(mCloudsMat.Id, &ctrans[0][0]);
        mCloudsPlane.Draw();
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
    }
    mBaseRt.Disable();

    // Draw
    mBaseMatRt.Use();
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, mBaseRt.RenderTexture.Id);
    glUniform1i(glGetUniformLocation(mBaseMatRt.Id, "uColorTexture"), 0);
    mBaseQuadRt.Draw();

    RenderUi();
    mWindow.Swap();
}

void GLApp::RenderUi()
{
    ImGui_ImplGlfwGL3_NewFrame();

    ImGui::Begin("Nature 3.0");
    {
        ImGui::Text("Time:          %f", mTime);
        ImGui::Text("Delta Time:    %f", mDeltaTime);
        ImGui::Text("FPS:           %i", mFps);
        ImGui::Separator();
        ImGui::DragFloat("Camera speed", &mCamera.Speed, 0.1f, 0.01f, 4.0f);
        ImGui::DragFloat("Camera sensitivity", &mCamera.Sensitivity, 0.05f, 0.1f, 1.0f);
        ImGui::Separator();
    }
    ImGui::End();

    mTerrain.RenderUi();

    ImGui::Render();
}

void GLApp::Release()
{
    ImGui_ImplGlfwGL3_Shutdown();
    mWindow.Release();
}

