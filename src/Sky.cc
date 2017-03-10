/*
    Sky.cc nachocpol@gmail.com
*/

#include "Sky.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "imgui.h"
#include "GLFW/glfw3.h"

const float kPi = 3.141517f;

Sky::Sky():
    mKr(0.0025f),
    mESun(14.0f),
    mKm(0.0035f),
    mWaveLength(0.65f,0.57f,0.45f),
    mSamples(12),
    mFSamples((float)mSamples),
    mOuterRadius(kAtmosphereR),
    mInnerRadius(kEarthR),
    mRScaleDepth(0.35f),
    mMScaleDepth(0.1f),
    mG(-0.990f),
    SunPosition(0.0f,0.5f,1.0f)
{
    mPow4WaveLength = glm::pow(mWaveLength, glm::vec3(4.0f));
    m3InvWavelength = 1.0f / mPow4WaveLength;
    mOuterRadius2 = pow(mOuterRadius, 2.0f);
    mInnerRadius2 = pow(mInnerRadius, 2.0f);
    mKrESun = mKr * mESun;
    mKmESun = mKm * mESun;
    mKr4PI = mKr * 4.0f * kPi;
    mKm4PI = mKm * 4.0f * kPi;
    mScale = 1.0f / (mOuterRadius - mInnerRadius);
    mScaleOverScaleDepth = mScale / mRScaleDepth;
    mG2 = pow(mG, 2.0f);
}

Sky::~Sky()
{
}

void Sky::Init()
{
    MeshBasicVertexData md;
    LoadMeshFromFile("../data/meshes/spherehi.obj", md);
    mSkyMesh.Init(md.vertex, md.ele);
    mSkyMaterial.Init
    (
        "../data/shaders/sky/sky2.vs",
        "../data/shaders/sky/sky2.fs"
    );
}

void Sky::Render()
{
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    mSkyMaterial.Use();

    GLuint p = mSkyMaterial.Id;
    glw::SetUniform3f("uSunPosition",p, &SunPosition.x);
    glw::SetUniform1i("uSamples", p, &mSamples);
    glw::SetUniform1f("uFSamples", p, &mFSamples);
    glw::SetUniform1f("uInnerRadius", p, &mInnerRadius);
    glw::SetUniform1f("uG", p, &mG);
    glw::SetUniform1f("uG2", p, &mG2);

    glw::SetUniform3f("u3InvWavelength", p, &m3InvWavelength.x);
    glw::SetUniform1f("uKrESun", p, &mKrESun);
    glw::SetUniform1f("uKmESun", p, &mKmESun);
    glw::SetUniform1f("uKr4PI", p, &mKr4PI);
    glw::SetUniform1f("uKm4PI", p, &mKm4PI);
    glw::SetUniform1f("uScale", p, &mScale);
    glw::SetUniform1f("uScaleDepth", p, &mRScaleDepth);
    glw::SetUniform1f("uScaleOverScaleDepth", p, &mScaleOverScaleDepth);

    glm::mat4 strans = glm::mat4();
    strans = glm::translate(strans, glm::vec3(0.0f, 0.0f, 0.0f));
    strans = glm::scale(strans, glm::vec3(kAtmosphereR * 2.0f)); //radius of sphere is 0.5f
    glw::SetTransform(mSkyMaterial.Id, &strans[0][0]);
    mSkyMesh.Render();
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void Sky::RenderUi()
{
    ImGui::Begin("Nature 3.0");
    {
        ImGui::Text("Sky");
        ImGui::InputFloat("Rayleigh scattering constant(sky)", &mKr);
        ImGui::InputFloat("Sun brightness constant(sky)", &mESun);
        ImGui::InputFloat("Mie scattering constant(sky)", &mKm);
        ImGui::InputFloat3("Particles wavelength(sky)", &mWaveLength.x);
        ImGui::InputInt("Samples(sky)", &mSamples);
        ImGui::InputFloat("Outer Radius(sky)", &mOuterRadius);
        ImGui::InputFloat("Inner Radius(sky)", &mInnerRadius);
        ImGui::InputFloat("Rayleigh Scale Depth(sky)", &mRScaleDepth);
        ImGui::InputFloat("Mie Scale Depth(sky)", &mMScaleDepth);
        ImGui::InputFloat("Mie phase asymmetry factor(sky)", &mG);
        ImGui::Separator();
    }
    ImGui::End();

    // Update values
    mPow4WaveLength = glm::pow(mWaveLength, glm::vec3(4.0f));
    m3InvWavelength = 1.0f / mPow4WaveLength;
    mOuterRadius2 = pow(mOuterRadius, 2.0f);
    mInnerRadius2 = pow(mInnerRadius, 2.0f);
    mKrESun = mKr * mESun;
    mKmESun = mKm * mESun;
    mKr4PI = mKr * 4.0f * kPi;
    mKm4PI = mKm * 4.0f * kPi;
    mScale = 1.0f / (mOuterRadius - mInnerRadius);
    mScaleOverScaleDepth = mScale / mRScaleDepth;
}
