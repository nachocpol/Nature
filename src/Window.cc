/*
    Window.cc nachocpol@gmail.com
*/

#include "Window.h"
#include "Input.h"

Window::Window():
    mWindowHandle(nullptr),
    mTitle("Default"),
    mSize(1,1)
{
}

Window::~Window()
{
}

bool Window::Init(const char * title, glm::uvec2 size)
{
    glfwInit();

	// Monitor and vid mode
	auto mainMonitor = glfwGetPrimaryMonitor();
	auto vidMode = glfwGetVideoMode(mainMonitor);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    mWindowHandle = glfwCreateWindow(vidMode->width, vidMode->height, title, mainMonitor, nullptr);
    if (!mWindowHandle)
    {
        return false;
    }
    glfwMakeContextCurrent(mWindowHandle);

    Input::GetInstance()->SetWindow(mWindowHandle);
    return true;
}

bool Window::Events()
{
    glfwPollEvents();
    return glfwWindowShouldClose(mWindowHandle);
}

void Window::Swap()
{
    glfwSwapBuffers(mWindowHandle);
}

void Window::Release()
{
    glfwTerminate();
}
