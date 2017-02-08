/*
    Camera.cc nachocpol@gmail.com
*/

#include "Camera.h"
#include "gtc/matrix_transform.hpp"
#include "GLFW/glfw3.h"
#include "Input.h"

Camera::Camera():
    mUp(0.0f,1.0f,0.0),
    mPosition(-20.0f,40.0f,-20.0f),
    mFront(0.0f,0.0f,0.0f),
    mNear(0.1f),
    mFar(1000.0f),
    mFovRad(glm::radians(75.0f))
{
}

Camera::~Camera()
{
}

void Camera::Init(float aspect)
{
    Projection = glm::perspective(mFovRad, aspect, mNear, mFar);
    View = glm::lookAt(mPosition, mFront, mUp);
}

void Camera::Resize(float aspect)
{
    Projection = glm::perspective(mFovRad, aspect, mNear, mFar);
}

void Camera::Update()
{
    if (Input::GetInstance()->GetKey(INPUT_KEY_W))
    {
        mPosition += mFront * Speed;
    }
    if (Input::GetInstance()->GetKey(INPUT_KEY_S))
    {
        mPosition -= mFront * Speed;
    }
    if (Input::GetInstance()->GetKey(INPUT_KEY_A))
    {
        mPosition -= glm::cross(mFront, mUp) * Speed;
    }
    if (Input::GetInstance()->GetKey(INPUT_KEY_D))
    {
        mPosition += glm::cross(mFront, mUp) * Speed;
    }

    UpdateMouseLook();
    UpdateView();
}

void Camera::UpdateView()
{
    // Calc new front vector
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(mPitch)) * cos(glm::radians(mYaw));
    newFront.y = sin(glm::radians(mPitch));
    newFront.z = cos(glm::radians(mPitch)) * sin(glm::radians(mYaw));
    mFront = glm::normalize(newFront);

    View = glm::lookAt(mPosition,
        mPosition + mFront,
        mUp);
}

void Camera::SetYInverse(bool newInv)
{
    mYInversed = newInv;
}

void Camera::UpdateMouseLook()
{
    // Find mouse offset and cache values
    float curX = Input::GetInstance()->GetMouseX();
    float curY = Input::GetInstance()->GetMouseY();
    if (mYInversed) curY = -curY;
    float xOff = (curX - mLastX) * Sensitivity;
    float yOff = (curY - mLastY) * Sensitivity;
    mLastX = curX;
    mLastY = curY;

    mYaw += xOff;
    mPitch += yOff;

    // Clamp
    if (mPitch > 89.0f)mPitch = 89.0f;
    if (mPitch < -89.0f) mPitch = -89.0f;
}
