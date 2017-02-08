/*
    Terrain.cc nachocpol@gmail.com
*/

#include "Terrain.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/random.hpp"

#include <string>

Terrain::Terrain():
    ChunkSide(16),
    ElementSide(64),
    ElementSize(1.0f)
{
}

Terrain::~Terrain()
{
}

void Terrain::Init()
{
    mTerrainMaterial.Init("../data/shaders/terrain.vs",
        "../data/shaders/terrain.fs");

    mGrassTexture.Init(TextureDef("../data/textures/grass.jpg", glm::vec2(0.0f), TextureUsage::kTexturing));
    mCliffTexture.Init(TextureDef("../data/textures/cliff.png", glm::vec2(0.0f), TextureUsage::kTexturing));
    mHeightMap.Init(TextureDef("../data/hmaps/hm.png", glm::vec2(0.0f), TextureUsage::kTexturing));
    mSplatMap.Init(TextureDef("../data/hmaps/splat.png", glm::vec2(0.0f), TextureUsage::kTexturing));

    mChunks.resize(ChunkSide * ChunkSide);
    for (unsigned int i = 0; i < ChunkSide; i++)
    {
        for (unsigned int j = 0; j < ChunkSide; j++)
        {
            unsigned int idx = i * ChunkSide + j;
            mChunks[idx].ChunkPosition = glm::vec2(i, j);
            InitMeshAsGrid(mChunks[idx].ChunkMesh, ElementSide, ElementSize);
        }
    }
}

void Terrain::Update()
{

}

void Terrain::Draw(bool useClip, glm::vec4 plane)
{
    mTerrainMaterial.Use();
    if (useClip)
    {
        glw::SetClipPlane(0, plane, mTerrainMaterial.Id);
    }
    else
    {
        glw::SetClipPlane(0, glm::vec4(0.0f, 1.0f, 0.0f, 99999.0f), mTerrainMaterial.Id);
    }

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, mGrassTexture.Id);
    glUniform1i(glGetUniformLocation(mTerrainMaterial.Id, "uGrassTexture"), 0);

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, mHeightMap.Id);
    glUniform1i(glGetUniformLocation(mTerrainMaterial.Id, "uHeightMap"), 1);

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, mSplatMap.Id);
    glUniform1i(glGetUniformLocation(mTerrainMaterial.Id, "uSplatTexture"), 2);

    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, mCliffTexture.Id);
    glUniform1i(glGetUniformLocation(mTerrainMaterial.Id, "uCliffTexture"), 3);

    glm::mat4 curModel;
    for (unsigned int i = 0; i < mChunks.size(); i++)
    {
        // Build model for current chunk
        curModel = glm::mat4();
        curModel = glm::translate
        (  
            curModel, 
            glm::vec3
            (  
                mChunks[i].ChunkPosition.x * 63,
                0.0f,
                mChunks[i].ChunkPosition.y * 63
            )
        );

        unsigned int loc = glGetUniformLocation(mTerrainMaterial.Id, "uModel");
        glUniformMatrix4fv(loc, 1, GL_FALSE,&curModel[0][0]);
        loc = glGetUniformLocation(mTerrainMaterial.Id, "uChunkPos");
        glUniform2fv(loc, 1, &mChunks[i].ChunkPosition.x);
        mChunks[i].ChunkMesh.Draw();
    }
}

void Terrain::InitMeshAsGrid(glw::Mesh& mesh, unsigned int size, float eleSize)
{
    std::vector<BasicVertex> vertex;
    std::vector<unsigned int> ele;

    unsigned int vertexCount = size * size;
    vertex.resize(vertexCount);

    // Build vertex data
    for (unsigned int z = 0; z < size; z++)
    {
        for (unsigned int x = 0; x < size; x++)
        {
            unsigned int index = x * size + z;
            vertex[index].Position = glm::vec3(x , 0.0f, z);
        }
    }

    // Build index data
    unsigned int faceCnt = (size - 1) * (size - 1) * 2;
    ele.resize(faceCnt * 3);
    unsigned int k = 0;
    for (unsigned int i = 0; i < size - 1; i++)  // -1 normal / -2 debug chunk border
    {
        for (unsigned int j = 0; j < size - 1; j++)
        {
            ele[k] = i * size + j;
            ele[k + 1] = i * size + j + 1;
            ele[k + 2] = (i + 1) * size + j;
            
            ele[k + 3] = (i + 1) * size + j;
            ele[k + 4] = i * size + j + 1;
            ele[k + 5] = (i + 1) * size + j + 1;

            k += 6;
        }
    }

    mesh.Init(vertex, ele);
}

