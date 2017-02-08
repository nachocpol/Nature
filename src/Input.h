/*
    Input.h nachocpol@gmail.com
*/

#pragma once

#include "Types.h"
#include "GLFW/glfw3.h"

class Input
{
public:
    static Input* GetInstance();
    void SetWindow(GLFWwindow* window);
    bool GetKey(int key);
    float GetMouseX();
    float GetMouseY();

private:
    Input();
    ~Input();
    static Input* mInstance;
    GLFWwindow* mWindow;
};
