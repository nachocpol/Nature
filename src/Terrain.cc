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

const float kPi = 3.141517f;

Terrain::Terrain():
    ChunkSide(16), 
    ElementSide(16),
    mKr(0.0035f),
    mESun(14.0f),
    mKm(0.002f),
    mWaveLength(0.65f, 0.57f, 0.45f),
    mSamples(1),
    mFSamples((float)mSamples),
    mOuterRadius(kAtmosphereR),
    mInnerRadius(kEarthR),
    mRScaleDepth(0.35f),
    mMScaleDepth(0.1f),
    mG(-0.990f),
    SunPosition(0.0f, 0.5f, 1.0f)
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
        "../data/shaders/terrain/terrain.vs",
        "../data/shaders/terrain/terrain.fs",
        "../data/shaders/terrain/terrain.tc",
        "../data/shaders/terrain/terrain.te"
    );
    mTerrainMaterialInst.Init
    (
        "../data/shaders/terrain/terraininst.vs",
        "../data/shaders/terrain/terrain.fs",
        "../data/shaders/terrain/terrain.tc",
        "../data/shaders/terrain/terrain.te"
    );

    mGrassTexture.Init(TextureDef("../data/textures/grass.jpg", glm::vec2(0.0f), TextureUsage::kTexturing));
    mCliffTexture.Init(TextureDef("../data/textures/cliff.png", glm::vec2(0.0f), TextureUsage::kTexturing));
    mHeightMap.Init(TextureDef("../data/hmaps/lagodix/lagodixhm.png", glm::vec2(0.0f), TextureUsage::kTexturing));
    mSplatMap.Init(TextureDef("../data/hmaps/lagodix/lagodixsplat.png", glm::vec2(0.0f), TextureUsage::kTexturing));
    mSnowTexture.Init(TextureDef("../data/textures/snow.png", glm::vec2(0.0f), TextureUsage::kTexturing));
    mNormal.Init(TextureDef("../data/hmaps/lagodix/lagodixnorm.png", glm::vec2(0.0f), TextureUsage::kTexturing));

    // Load the hmpa so we can sample to find the chunks y pos
    TextureDef hMap;
    hMap.Path = mHeightMap.Def.Path;
    LoadTextureFromFile(hMap);

    // Load hmap float....
    mHmapF.Path = mHeightMap.Def.Path;
    LoadTextureFromFileF(mHmapF);
    
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
            mChunks[idx].RealChunkPos = bSpherePos * MapScale; // Set position!

            // Find terrain height
            unsigned int yDataIdx = (int)bSpherePos.z * HeightMapSize + (int)bSpherePos.x;
            unsigned char yData = hMap.Data[yDataIdx];
            bSpherePos.y = ((float)yData / 255.0f) * 200.0f;
            mChunks[idx].BSphere = BoundingSphere(bSpherePos * MapScale, diagonal * MapScale); 

            // Add grass
            AddGrass(mChunks[idx],p);
        }
    }

    // Init instanced mesh (chunks)
    InitMeshAsGrid(mChunkMeshInstance.IMesh, ElementSide, ElementSize);
    mChunkMeshInstance.IMesh.DMode = DrawMode::kPatches3;
    mChunkMeshInstance.InitInstances(ChunkSide * ChunkSide, BufferUsage::kDynamic);
    curTransforms.resize(mChunks.size());

    // Init vegetation materials
    mGrassMaterial.Init
    (
        "../data/shaders/vegetation/grass.vs", 
        "../data/shaders/vegetation/grass.fs",
        "../data/shaders/vegetation/grass.gs"
    );

    // Init vegetation meshes
    MeshBasicVertexData gMd;
    gMd.vertex =
    {
        BasicVertex(-0.5f,0.0f,0.0f,0.0f,1.0f),
        BasicVertex( 0.5f,0.0f,0.0f,1.0f,1.0f),
        BasicVertex( 0.5f,1.0f,0.0f,1.0f,0.0f),
        BasicVertex(-0.5f,1.0f,0.0f,0.0f,0.0f),

        BasicVertex( 0.0f,0.0f,0.5f,0.0f,1.0f),
        BasicVertex( 0.0f,0.0f,-0.5f,1.0f,1.0f),
        BasicVertex( 0.0f,1.0f,-0.5f,1.0f,0.0f),
        BasicVertex( 0.0f,1.0f, 0.5f,0.0f,0.0f)
    };
    gMd.ele =
    {
        0,1,2,
        0,2,3,
        4,5,6,
        4,6,7
    };
    mGrassInstanceMesh.IMesh.Init(gMd.vertex,gMd.ele);
    mGrassInstanceMesh.InitInstances(mMaxGrassChunk, BufferUsage::kDynamic);

    // Grass 1
    mGrass1AlbedoTex.Init(TextureDef("../data/textures/grass_01/diffus.tga", glm::vec2(0.0f), TextureUsage::kTexturing));
    mGrass1OpacityTex.Init(TextureDef("../data/textures/grass_01/opacity.tga", glm::vec2(0.0f), TextureUsage::kTexturing));
}

void Terrain::Update(Frustrum viewFrust)
{
    // Perform frustum culling
    if (FrustrumCulling)
    {
        mChunksVisible.clear();
        for (unsigned int i = 0; i < mChunks.size(); i++)
        {
            if (viewFrust.SphereTest(mChunks[i].BSphere) == kInside)
            {
                mChunksVisible.push_back(mChunks[i]);
            }
        }
    }
}

void Terrain::Render(bool useClip, glm::vec4 plane)
{
    // Set the current material
    GLuint p = 0;
    if (mUseInstancing)
    {
        mTerrainMaterialInst.Use();
        p = mTerrainMaterialInst.Id;
    }
    else
    {
        mTerrainMaterial.Use();
        p = mTerrainMaterial.Id;
    }

    if (useClip)
    {
        glw::SetClipPlane(0, plane, p);
    }
    else
    {
        glw::SetClipPlane(0, glm::vec4(0.0f, 1.0f, 0.0f, 99999.0f), p);
    }

    // Textures
    glw::SetUniformTexture("uGrassTexture", p, mGrassTexture.Id, 0);
    glw::SetUniformTexture("uHeightMap", p, mHeightMap.Id, 1);
    glw::SetUniformTexture("uSplatTexture", p, mSplatMap.Id, 2);
    glw::SetUniformTexture("uCliffTexture", p, mCliffTexture.Id, 3);
    glw::SetUniformTexture("uLutTexture", p, LutTexture->Id, 4);
    glw::SetUniformTexture("uSnowTexture", p, mSnowTexture.Id, 5);
    glw::SetUniformTexture("uNormalTexture", p, mNormal.Id, 6);

    // Random stuff
    glw::SetUniform1f("uTiling1", p, &mTiling1);
    glw::SetUniform1f("uTiling2", p, &mTiling2);

    // Scattering parameters
    glw::SetUniform3f("uSunPosition", p, &SunPosition.x);
    glw::SetUniform1i("uSamples", p, &mSamples);
    glw::SetUniform1f("uInnerRadius", p, &mInnerRadius);
    glw::SetUniform3f("u3InvWavelength", p, &m3InvWavelength.x);
    glw::SetUniform1f("uKrESun", p, &mKrESun);
    glw::SetUniform1f("uKmESun", p, &mKmESun);
    glw::SetUniform1f("uKr4PI", p, &mKr4PI);
    glw::SetUniform1f("uKm4PI", p, &mKm4PI);
    glw::SetUniform1f("uScale", p, &mScale);
    glw::SetUniform1f("uScaleDepth", p, &mRScaleDepth);
    glw::SetUniform1f("uScaleOverScaleDepth", p, &mScaleOverScaleDepth);

    // Draw terrain
    if (mUseInstancing)
    {
        // Render visible chunks
        if (FrustrumCulling)
        {
            RenderInstanced(mChunksVisible);
        }
        // Render all chunks
        else
        {
            RenderInstanced(mChunks);
        }
    }
    else
    {
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
    }

    // Draw grass
    glDisable(GL_CULL_FACE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    mGrassMaterial.Use();
    if (mUseInstancing)
    {
        for (unsigned int i = 0; i < mChunksVisible.size(); i++)
        {
            mChunksVisible[i].ChunkGrass.Render();
        }
    }
    else
    {

    }
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);

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
            glw::SetTransform(mSphereMat.Id, &curModel[0][0]);
            mSphereMesh.Render();
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void Terrain::RenderUi()
{
    ImGui::Begin("Nature 3.0");
    {
        ImGui::Text("Terrain");
        ImGui::InputFloat("Texture tiling 1", &mTiling1);
        ImGui::InputFloat("Texture tiling 2", &mTiling2);
        ImGui::LabelText("ChunkSide", "%i", ChunkSide);
        ImGui::LabelText("ElementSide", "%i", ElementSide);
        ImGui::LabelText("ElementSize", "%f", ElementSize);
        ImGui::LabelText("HeightMapSize", "%i", HeightMapSize);
        ImGui::LabelText("Chunks", "%i/%i", mChunksVisible.size(), mChunks.size());
        ImGui::Checkbox("Frustrum culling", &FrustrumCulling);
        ImGui::Checkbox("Visual debug", &VisualDebug);
        ImGui::Checkbox("Draw wireframe", &mDrawWire);
        ImGui::Checkbox("Use instancing", &mUseInstancing);
        ImGui::InputFloat("Rayleigh scattering constant(terrain)", &mKr);
        ImGui::InputFloat("Sun brightness constant(terrain)", &mESun);
        ImGui::InputFloat("Mie scattering constant(terrain)", &mKm);
        ImGui::InputFloat3("Particles wavelength(terrain)", &mWaveLength.x);
        ImGui::InputInt("Samples(terrain)", &mSamples);
        ImGui::InputFloat("Outer Radius(terrain)", &mOuterRadius);
        ImGui::InputFloat("Inner Radius(terrain)", &mInnerRadius);
        ImGui::InputFloat("Rayleigh Scale Depth(terrain)", &mRScaleDepth);
        ImGui::InputFloat("Mie Scale Depth(terrain)", &mMScaleDepth);
        ImGui::InputFloat("Mie phase asymmetry factor(terrain)", &mG);
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
    if (mDrawWire){glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);}

    // Build model for current chunk
    glm::mat4 curModel;
    curModel = glm::mat4();
    curModel = glm::scale(curModel, glm::vec3(MapScale));
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

    GLuint p = mTerrainMaterial.Id;

    // Model matrix
    glw::SetTransform(p, &curModel[0][0]);

    c.ChunkMesh.Render();
    
    if(mDrawWire){glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);}
}

void Terrain::RenderInstanced(std::vector<Chunk>& chunks)
{
    if (mDrawWire) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }

    curTransforms.resize(chunks.size());
    for (unsigned int i = 0; i < chunks.size(); i++)
    {
        // Build model for current chunk
        glm::mat4 curModel;
        curModel = glm::mat4();
        curModel = glm::scale(curModel, glm::vec3(MapScale));
        curModel = glm::translate
        (
            curModel,
            glm::vec3
            (
                chunks[i].ChunkPosition.x * (ElementSide - 1) * ElementSize,
                0.0f,
                chunks[i].ChunkPosition.y * (ElementSide - 1) * ElementSize
            )
        );
        curTransforms[i] = curModel;
    }
    mChunkMeshInstance.Render(curTransforms);

    if (mDrawWire) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }
}

void Terrain::AddGrass(Chunk& chunk,glm::ivec2 p)
{
    // Data to build the point mesh
    std::vector<BasicVertexPoint> grassVertex;
    std::vector<unsigned int> grassEle;

    // Starting and end points of the chunk
    glm::vec2 cStart = glm::ivec2(p.x, p.y);
    cStart *= (ElementSide - 1) * ElementSize;
    glm::vec2 cEnd = cStart + glm::vec2((ElementSide - 1) * ElementSize);
    glm::mat4 vTrans;
    
    // Add grass
    float grassDensity = 16.0f;
    for (float ci = cStart.x; ci < cEnd.x; ci += ElementSize / grassDensity)
    {
        for (float cj = cStart.y; cj < cEnd.y; cj += ElementSize / grassDensity)
        {
            vTrans = glm::mat4();
            unsigned int vIdx = (int)cj * HeightMapSize + (int)ci;
            float vY = mHmapF.Data[vIdx] * 200.0f * MapScale;
            if (vY > 200.0f || vY < 24.0f)continue;
            glm::vec3 vp = glm::vec3(ci, 0.0f, cj) * MapScale;
            glm::vec2 randPos = glm::diskRand(10.0f);
            vp.x += randPos.x;
            vp.z += randPos.y;
            vp.y = vY;
            vTrans = glm::translate(vTrans, vp);
            vTrans = glm::scale(vTrans, glm::vec3(2.0f));
            grassVertex.push_back(BasicVertexPoint(vp.x, vp.y, vp.z));
            grassEle.push_back(grassEle.size());
        }
    }
    chunk.ChunkGrass.Init(grassVertex, grassEle);
    chunk.ChunkGrass.DMode = DrawMode::kPoints;
}

