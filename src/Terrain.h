/*
    Terrain.h nachocpol@gmail.com
*/

#pragma once

#include "glm.hpp"
#include "GLWrapper.h"

struct Chunk
{
    glm::vec2 ChunkPosition;
    glw::Mesh ChunkMesh;
};

class Terrain
{
public:
    Terrain();
    ~Terrain();
    void Init();
    void Update();
    void Draw(bool useClip,glm::vec4 plane = glm::vec4(0.0f))
        ;
    glw::Material* GetMaterial() { return &mTerrainMaterial; }

    // Number of chunks at each side of the
    // terrain. TotalChunk = ChunkSide * ChunkSide
    unsigned int ChunkSide;
    // How many elements (tris) has each chunk
    unsigned int ElementSide;
    // Size of each tri
    float ElementSize;

private:
    void InitMeshAsGrid(glw::Mesh& mesh,unsigned int size, float eleSize);
    std::vector<Chunk> mChunks;
    glw::Material mTerrainMaterial;

    glw::Texture mHeightMap;
    glw::Texture mSplatMap;
    glw::Texture mGrassTexture;
    glw::Texture mCliffTexture;
};
