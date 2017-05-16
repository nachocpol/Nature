/*
    Camera.cc nachocpol@gmail.com
*/

#include "Camera.h"
#include "gtc/matrix_transform.hpp"
#include "GLFW/glfw3.h"
#include "Input.h"

Camera::Camera():
    mUp(0.0f,1.0f,0.0),
    mPosition(10048.0f,50.0f, 10048.0f),
    mFront(mPosition + glm::vec3(0.0f,0.0f,-1.0f)),
    mNear(0.2f),
    mFar(1000000.0f)
{
    SetFov(85.0f);
}

Camera::~Camera()
{
}

void Camera::Init(float aspect)
{
    Projection = glm::perspective(mFovRad, aspect, mNear, mFar);
	//Projection = PerspectiveInverseMap(aspect);
    View = glm::lookAt(mPosition, mFront, mUp);
    CameraFrustrum.SetPlanes(Projection * View);
    mAspect = aspect;
}

void Camera::Resize(float aspect)
{
    Projection = glm::perspective(mFovRad, aspect, mNear, mFar);
	//Projection = PerspectiveInverseMap(aspect);
    CameraFrustrum.SetPlanes(Projection * View);
    mAspect = aspect;
}

void Camera::Update()
{
	// Orbit camera
	if (CameraOrbit)
	{
		glm::vec3 orbitCenter = glm::vec3(9000.0f, 0.0, 9000.0f);
		mPosition.x = orbitCenter.x + sin(glfwGetTime()) * 500.0f;
		mPosition.z = orbitCenter.z + cos(glfwGetTime()) * 500.0f;
		View = glm::lookAt(mPosition,
			orbitCenter,
			glm::vec3(0.0f, 1.0f, 0.0f));
		CameraFrustrum.SetPlanes(Projection * View);
		return;
	}

	// Fly camera
    if (Input::GetInstance()->GetKey(INPUT_KEY_L))
    {
        LockMouse = !LockMouse;
    }
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
	if (UpdateFustrum)
	{
		CameraFrustrum.SetPlanes(Projection * View);
	}
}

void Camera::SetYInverse(bool newInv)
{
    mYInversed = newInv;
}

void Camera::SetFov(float angle)
{
    mFovDegree = angle;
    mFovRad = glm::radians(angle);
    Projection = glm::perspective(mFovRad, mAspect, mNear, mFar);
	//Projection = PerspectiveInverseMap(mAspect);
}

void Camera::UpdateMouseLook()
{
    if (LockMouse)return;
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

glm::mat4 Camera::PerspectiveInverseMap(float aspect)
{
	glm::mat4 stdPerspective = glm::perspective(mFovRad, aspect, mNear, mFar);
	return stdPerspective;
	/*
	tdPerspective[2][2] = mNear / (mNear - mFar);
	stdPerspective[2][3] = stdPerspective[2][2] == 0.0f ? mNear : (mFar * mNear / (mFar - mNear));
	return stdPerspective;
	*/

	/*
	return glm::mat4
	(
		mFovRad/aspect, 0.0f,	0.0f,	0.0f,
		0.0f,			mFovRad,0.0f,	0.0f,
		0.0f,			0.0f,	0.0f,	-1.0f,
		0.0f,			0.0f,	mNear,	0.0f
	);
	*/

	/*
	float f = 1.0f / tan(mFovRad * 0.5f);
	return glm::mat4
	(
		f/aspect,		0.0f,	0.0f,	0.0f,
		0.0f,			f,		0.0f,	0.0f,
		0.0f,			0.0f,	0.0f,	-1.0f,
		0.0f,			0.0f,	mNear,	0.0f
	);
	*/
}
