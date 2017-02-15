/*
    Terrain.cc nachocpol@gmail.com
    When we tell request a grid of three elements,
    we must have in mind that this is the
    output:
    ·   ·   ·
    ·   ·   ·
    ·   ·   ·
    It will have 3 VERTEX on each side not 3 "quads"
    that's why we have to use (ElementSide-1) when we
    perform the translations etc
*/

#include "Terrain.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/random.hpp"
#include "imgui.h"

#include <string>

#include "GLFW/glfw3.h"

Terrain::Terrain():
    ChunkSide(16),
    ElementSide(4)
{
}

Terrain::~Terrain()
{
}

void Terrain::Init()
{
    MeshBasicVertexData md;
    LoadMeshFromFile("../data/meshes/sphere.obj", md);
    mSphereMesh.Init(md.vertex, md.ele);
    mSphereMat.Init("../data/shaders/debug.vs", "../data/shaders/debug.fs");

    // Ignoring decimal part
    ElementSize = HeightMapSize / (ChunkSide*(ElementSide-1));
    mTerrainMaterial.Init
    (
        "../data/shaders/terrain.vs",
        "../data/shaders/terrain.fs",
        "../data/shaders/terrain.tc",
        "../data/shaders/terrain.te"
    );

    mGrassTexture.Init(TextureDef("../data/textures/grass.jpg", glm::vec2(0.0f), TextureUsage::kTexturing));
    mCliffTexture.Init(TextureDef("../data/textures/cliff.png", glm::vec2(0.0f), TextureUsage::kTexturing));
    mHeightMap.Init(TextureDef("../data/hmaps/hm2k.png", glm::vec2(0.0f), TextureUsage::kTexturing));
    mSplatMap.Init(TextureDef("../data/hmaps/splat2k.png", glm::vec2(0.0f), TextureUsage::kTexturing));

    // Load the hmpa so we can sample to find the chunks y pos
    TextureDef hMap;
    hMap.Path = "../data/hmaps/hm2k.png";
    LoadTextureFromFile(hMap);
    
    // Bounding sphere radius (sin(45) because chunks are squares
    // hypotenuse = opposite / sin(angle)
    float diagonal = ((ElementSide - 1) * ElementSize) / sin(45.0f);
    float radius = diagonal * 0.5f;

    mChunks.resize(ChunkSide * ChunkSide);
    for (unsigned int i = 0; i < ChunkSide; i++)
    {
        for (unsigned int j = 0; j < ChunkSide; j++)
        {
            unsigned int idx = i * ChunkSide + j;
            glm::vec2 p = glm::vec2(i, j);
            mChunks[idx].ChunkPosition = p;
            InitMeshAsGrid(mChunks[idx].ChunkMesh, ElementSide, ElementSize);
            mChunks[idx].ChunkMesh.DMode = DrawMode::kPatches3;

            // Build bounding sphere
            // Set the chuks position as the initial position
            glm::vec3 bSpherePos = glm::vec3(p.x, 0.0f, p.y);
            bSpherePos *= (ElementSide - 1) * ElementSize;
            // Translate it by the radius so it is centered
            bSpherePos = glm::vec3(bSpherePos.x + radius, 0.0f, bSpherePos.z + radius);
            // Find terrain height
            unsigned int yDataIdx = (int)bSpherePos.z * HeightMapSize + (int)bSpherePos.x;
            unsigned char yData = hMap.Data[yDataIdx];
            bSpherePos.y = ((float)yData / 255.0f) * 100.0f;
            // printf("LOG:BSphere x:%f, y:%f ,z:%f \n", bSpherePos.x,bSpherePos.y, bSpherePos.z);
            // We use the diagonal because the sphere mesh radius is 0.5
            mChunks[idx].BSphere = BoundingSphere(bSpherePos, diagonal); 
                                                                                
        }
    }
}

void Terrain::Update(Frustrum viewFrust)
{
    // Perform frustum culling
    if (FrustrumCulling)
    {
        float initTime = glfwGetTime();
        mChunksVisible.clear();
        for (unsigned int i = 0; i < mChunks.size(); i++)
        {
            if (viewFrust.SphereTest(mChunks[i].BSphere) == kInside)
            {
                mChunksVisible.push_back(mChunks[i]);
            }
        }
        float elapsed = glfwGetTime() - initTime;
        //printf(" LOG:Time to cull terrain:%f\n", elapsed * 1000.0f);
    }
}

void Terrain::Render(bool useClip, glm::vec4 plane)
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

    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D, LutTexture->Id);
    glUniform1i(glGetUniformLocation(mTerrainMaterial.Id, "uLutTexture"), 4);

    // Render visible chunks
    if (FrustrumCulling)
    {
        for (unsigned int i = 0; i < mChunksVisible.size(); i++)
        {
            RenderChunk(mChunksVisible[i]);
        }
    }
    // Render all chunks
    else
    {
        for (unsigned int i = 0; i < mChunks.size(); i++)
        {
            RenderChunk(mChunks[i]);
        }
    }

    // Debug chunks
    if (VisualDebug)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glm::mat4 curModel;
        unsigned int loc = 0;
        for (unsigned int i = 0; i < mChunks.size(); i++)
        {
            mSphereMat.Use();
            curModel = glm::mat4();
            curModel = glm::translate(curModel, mChunks[i].BSphere.Position);
            curModel = glm::scale(curModel, glm::vec3(mChunks[i].BSphere.Radius));
            loc = glGetUniformLocation(mSphereMat.Id, "uModel");
            glUniformMatrix4fv(loc, 1, GL_FALSE, &curModel[0][0]);
            mSphereMesh.Draw();
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void Terrain::RenderUi()
{
    ImGui::Begin("Nature 3.0");
    {
        ImGui::Text("Terrain");
        ImGui::LabelText("ChunkSide", "%i", ChunkSide);
        ImGui::LabelText("ElementSide", "%i", ElementSide);
        ImGui::LabelText("ElementSize", "%f", ElementSize);
        ImGui::LabelText("HeightMapSize", "%i", HeightMapSize);
        ImGui::LabelText("Chunks", "%i/%i", mChunksVisible.size(), mChunks.size());
        ImGui::Checkbox("Frustrum culling", &FrustrumCulling);
        ImGui::Checkbox("Visual debug", &VisualDebug);
        ImGui::Separator();
    }
    ImGui::End();
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
            vertex[index].Position = glm::vec3(x *eleSize, 0.0f, z*eleSize);    
        }
    }

    // Build index data
    unsigned int faceCnt = (size-1) * (size-1) * 2;
    ele.resize(faceCnt * 3);
    unsigned int k = 0;
    for (unsigned int i = 0; i < size-1; i++)  // -1 normal / -2 debug chunk border
    {
        for (unsigned int j = 0; j < size-1; j++)
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

void Terrain::RenderChunk(Chunk & c)
{
    // Build model for current chunk
    glm::mat4 curModel;
    curModel = glm::mat4();
    curModel = glm::translate
    (
        curModel,
        glm::vec3
        (
            c.ChunkPosition.x * (ElementSide - 1) * ElementSize,
            0.0f,
            c.ChunkPosition.y * (ElementSide - 1) * ElementSize
        )
    );

    unsigned int loc = glGetUniformLocation(mTerrainMaterial.Id, "uModel");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &curModel[0][0]);
    loc = glGetUniformLocation(mTerrainMaterial.Id, "uChunkPos");
    glUniform2fv(loc, 1, &c.ChunkPosition.x);
    c.ChunkMesh.Draw();
}

