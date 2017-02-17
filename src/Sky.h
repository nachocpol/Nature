/*
    Sky.h nachocpol@gmail.com
*/

#pragma once
#include "GLWrapper.h"

class Sky
{
public:
    Sky();
    ~Sky();
    void Init();
    void Render();
    void RenderUi();

    glm::vec3 SunPosition;

private:
    glw::Mesh mSkyMesh;
    glw::Material mSkyMaterial;

    // camera position
    // light position
	// Earth radius: 6300000 (m)
	// Earth + atmosphere 6400000 (m)
	
	// Some constants
	float kEarthR = 6300000.0f;
	float kAtmosphereR = 6500000.0f;

    // Atmospheric scattering values
    float mKr;                  // Rayleigh scattering constant
    float mESun;                // Sun brightness constant
    float mKm;                  // Mie scattering constant
    glm::vec3 mWaveLength;      // Particles wavelength
    int mSamples;               // Number of sample rays to use in integral equation
    float mFSamples;
    float mOuterRadius;     
    float mInnerRadius;
    float mRScaleDepth; 
    float mMScaleDepth;
    float mG;                   // The Mie phase asymmetry factor
    float mG2;

    glm::vec3  mPow4WaveLength;
    glm::vec3  m3InvWavelength;	// 1 / pow(wavelength, 4) for the red, green, and blue channels
    float mOuterRadius2;	    // fOuterRadius^2
    float mInnerRadius2;	    // fInnerRadius^2
    float mKrESun;			    // Kr * ESun
    float mKmESun;			    // Km * ESun
    float mKr4PI;			    // Kr * 4 * PI
    float mKm4PI;			    // Km * 4 * PI
    float mScale;			    // (1.0f / (m_fOuterRadius - m_fInnerRadius)) / m_fRayleighScaleDepth)
    float mScaleOverScaleDepth;	// fScale / fScaleDepth
};

