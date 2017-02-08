/*
    Window.h nachocpol@gmail.com
*/

#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm.hpp"

class Window
{
public:
    Window();
    ~Window();
    bool Init(const char* title,glm::uvec2 size);
    bool Events();
    void Swap();
    void Release();
    GLFWwindow* GetHandle() { return mWindowHandle; }

private:
    GLFWwindow* mWindowHandle;
    const char* mTitle;
    glm::uvec2 mSize;
};
