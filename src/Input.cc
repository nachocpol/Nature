/*
    Input.cc nachocpol@gmail.com
*/

#include "Input.h"
#include <iostream>

Input* Input::mInstance = nullptr;

Input::Input()
{
}

Input::~Input()
{
}

Input* Input::GetInstance()
{
    if (!mInstance)
    {
        mInstance = new Input();
    }
    return mInstance;
}

void Input::SetWindow(GLFWwindow* window)
{
    if (!window)
    {
        printf(" LOG: Passed window is null, ignoring.\n");
        return;
    }
    if (mWindow)
    {
        printf(" LOG: Overriding input window handle.\n");
        mWindow = window;
        return;
    }
    if (window == mWindow)
    {
        printf(" LOG: Passed window is same as cached, ignoring.\n");
        return;
    }

    mWindow = window;
}

bool Input::GetKey(int key)
{
    if (glfwGetKey(mWindow, key) == GLFW_PRESS)
    {
        return true;
    }
    return false;
}

float Input::GetMouseX()
{
    double x = 0.0f;
    double y = 0.0f;
    glfwGetCursorPos(mWindow, &x, &y);
    return (float)x;
}

float Input::GetMouseY()
{
    double x = 0.0f;
    double y = 0.0f;
    glfwGetCursorPos(mWindow, &x, &y);
    return (float)y;
}
