/*
    App.cc nachocpol@gmail.com
*/

#pragma once

#include "App.h"
#include "GLFW/glfw3.h"
#include "glm.hpp"

App::App():
    mRunning(true),
    mTime(0.0f),
    mDeltaTime(0.0f),
    mFpsCurUpdate(kFpsUpdateTime)
{
}

App::~App()
{
}

void App::Start()
{
    if (Init())
    {
        // Set timer to 0
        glfwSetTime(0.0f);

        while (mRunning)
        {
            // Time app running
            mTime = glfwGetTime();

            // ----------
            Update();
            Render();
            // ----------

            // Get delta time
            mDeltaTime = glfwGetTime() - mTime;

            // Calculate FPS, we record the fps with
            // some delay so we can actually read the values
            mFpsCurUpdate += mDeltaTime;
            if (mFpsCurUpdate >= kFpsUpdateTime)
            {
                mFpsCurUpdate = 0.0f;
                mFps = 1.0f / mDeltaTime;
            }
        }
    }
    else
    {
        // Failed to initialize
    }
    Release();
}