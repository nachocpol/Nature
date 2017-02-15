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
    glw::Mesh ChunkMesh;
    BoundingSphere BSphere;
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
    bool FrustrumCulling = false;
    bool VisualDebug = false;

    glw::Texture* LutTexture;

private:
    void InitMeshAsGrid(glw::Mesh& mesh,unsigned int size, float eleSize);
    void RenderChunk(Chunk& c);

    // Chunks
    std::vector<Chunk> mChunks;
    std::vector<Chunk> mChunksVisible;
    
    // Material
    glw::MaterialTess mTerrainMaterial;

    // Textures
    glw::Texture mHeightMap;
    glw::Texture mSplatMap;
    glw::Texture mGrassTexture;
    glw::Texture mCliffTexture;

    // Frustrum culling
    std::vector<float> mChunkHeight;
    // Debug
    glw::Mesh mSphereMesh;
    glw::Material mSphereMat;
};
