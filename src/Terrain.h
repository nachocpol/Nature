/*
    Terrain.h nachocpol@gmail.com
*/

#pragma once

#include "glm.hpp"
#include "GLWrapper.h"

struct Chunk
{
    Chunk(){}
    glm::vec2 ChunkPosition;
    glm::vec3 RealChunkPos;
    glw::Mesh ChunkMesh;
    BoundingSphere BSphere;
    glw::Mesh ChunkGrass;   // per chunk grass
};

class Terrain
{
public:
    Terrain();
    ~Terrain();
    void Init();
    void Update(Frustrum viewFrust);
    void Render(bool useClip,glm::vec4 plane = glm::vec4(0.0f),bool blackPass = false);
    void RenderUi();
    glw::Material* GetMaterial() { return &mTerrainMaterial; }
    float GetHeight(float x,float z);
    int GetHeightMapId() { return mHeightMap.Id; }

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
    float HeightScale = 400.0f;
    bool FrustrumCulling = true;
    bool VisualDebug = false;

    glw::Texture* LutTexture;
    glm::vec3* CamPos;
    glm::vec3 SunPosition;

private:
    void InitMeshAsGrid(glw::Mesh& mesh,unsigned int size, float eleSize);
    void RenderChunk(Chunk& c);
    void RenderInstanced(std::vector<Chunk>& chunks);
    void AddGrass(Chunk& chunk,glm::ivec2 p);
    void RenderBlackPass();

    // Chunks
    std::vector<Chunk> mChunks;
    std::vector<Chunk> mChunksVisible;
    glw::InstancedMesh mChunkMeshInstance;
    std::vector<glm::mat4> curTransforms;

    // Material
    glw::Material mTerrainMaterialInst;
    glw::Material mTerrainMaterial;
    glw::Material mTerrainBlackMaterial;

    // Textures
    glw::Texture mHeightMap;
    glw::Texture mSplatMap;
    glw::Texture mGrassTexture;
    glw::Texture mCliffTexture;
    glw::Texture mSnowTexture;
    glw::Texture mNormal;
    glw::Texture mGrassNormal;
    glw::Texture mCliffNormal;
    glw::Texture mSnowNormal;

    // Frustrum culling
    std::vector<float> mChunkHeight;

    // Debug
    glw::Mesh mSphereMesh;
    glw::Material mSphereMat;
    bool mDrawWire = false;
    bool mUseInstancing = true;

    // Texturing
    float mTiling1 = 5000.0f;
    float mTiling2 = 200.0f;

    // Vegetation
    unsigned int mGrassCnt = 0;
    unsigned int mMaxGrassChunk = 0;
    glw::MaterialGeo mGrassMaterial;
    float mLodRange = 300.0f;
    bool mGrassWire = false;

    // Grass 1
    glw::Texture mGrass1AlbedoTex;
    glw::Texture mGrass1OpacityTex;

    // Terrain heigthmap (float texture)
    TextureDefF mHmapF;

    // Moon direction
    glm::vec3 mMoonDirection;
    float mNightAten = 0.0f;
    float mDayAten = 1.0f;
};
