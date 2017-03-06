/*
    Terrain.h nachocpol@gmail.com
*/

#pragma once

#include "glm.hpp"
#include "GLWrapper.h"

struct ChunkDeco
{
    std::vector<glm::mat4> GrassTransforms;
};

struct Chunk
{
    Chunk(){}
    glm::vec2 ChunkPosition;
    glm::vec3 RealChunkPos;
    glw::Mesh ChunkMesh;
    BoundingSphere BSphere;
    ChunkDeco Deco;
};

class Terrain
{
public:
    Terrain();
    ~Terrain();
    void Init();
    void Update(Frustrum viewFrust);
    void Render(bool useClip,glm::vec4 plane = glm::vec4(0.0f));
    void RenderUi();
    glw::MaterialTess* GetMaterial() { return &mTerrainMaterial; }

    // Number of chunks at each side of the
    // terrain. TotalChunk = ChunkSide * ChunkSide
    unsigned int ChunkSide;
    // How many elements (tris) has each chunk
    unsigned int ElementSide;
    // Size of each tri (it is auto initialized)
    float ElementSize;
    // Size of the heightmap used
    unsigned int HeightMapSize = 2048;
    float MapScale = 9.0f;
    bool FrustrumCulling = true;
    bool VisualDebug = false;

    glw::Texture* LutTexture;
    glm::vec3* CamPos;
    glm::vec3 SunPosition;

private:
    void InitMeshAsGrid(glw::Mesh& mesh,unsigned int size, float eleSize);
    void RenderChunk(Chunk& c);
    void RenderInstanced(std::vector<Chunk>& chunks);

    // Chunks
    std::vector<Chunk> mChunks;
    std::vector<Chunk> mChunksVisible;
    glw::InstancedMesh mChunkMeshInstance;
    std::vector<glm::mat4> curTransforms;

    // Material
    glw::MaterialTess mTerrainMaterialInst;
    glw::MaterialTess mTerrainMaterial;

    // Textures
    glw::Texture mHeightMap;
    glw::Texture mSplatMap;
    glw::Texture mGrassTexture;
    glw::Texture mCliffTexture;
    glw::Texture mSnowTexture;
    glw::Texture mNormal;
    glw::Texture mGrassBlades;

    // Frustrum culling
    std::vector<float> mChunkHeight;

    // Debug
    glw::Mesh mSphereMesh;
    glw::Material mSphereMat;
    bool mDrawWire = false;
    bool mUseInstancing = true;

    // Texturing
    float mTiling1 = 200.0f;
    float mTiling2 = 100.0f;

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

    // Vegetation
    unsigned int mGrassCnt = 0;
    unsigned int mMaxGrassChunk = 0;
    glw::Material mGrassMaterial;
    glw::InstancedMesh mGrassInstanceMesh;
    // Grass 1
    glw::Texture mGrass1AlbedoTex;
    glw::Texture mGrass1OpacityTex;
};
