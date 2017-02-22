/*
    Types.cc
*/

#include "Types.h"
#include "gtc/matrix_access.hpp"
#include "stb_image.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

BasicVertex::BasicVertex(   float px, float py, float pz,
                            float ux, float uy)
{
    Position = glm::vec3(px, py, pz);
    Uv = glm::vec2(ux, uy);
}

void Frustrum::SetPlanes(glm::mat4 vp)
{
    /*
    Planes[kNear] = SetValues
    (
        vp[2][0] + vp[3][0],
        vp[2][1] + vp[3][1],
        vp[2][2] + vp[3][2],
        vp[2][3] + vp[3][3]
    );
    Planes[kFar] = SetValues
    (
       -vp[2][0] + vp[3][0],
       -vp[2][1] + vp[3][1],
       -vp[2][2] + vp[3][2],
       -vp[2][3] + vp[3][3]
    );
    Planes[kBottom] = SetValues
    (
        vp[1][0] + vp[3][0],
        vp[1][1] + vp[3][1],
        vp[1][2] + vp[3][2],
        vp[1][3] + vp[3][3]
    );
    Planes[kTop] = SetValues
    (
       -vp[1][0] + vp[3][0],
       -vp[1][1] + vp[3][1],
       -vp[1][2] + vp[3][2],
       -vp[1][3] + vp[3][3]
    );
    Planes[kLeft] = SetValues
    (
        vp[0][0] + vp[3][0],
        vp[0][1] + vp[3][1],
        vp[0][2] + vp[3][2],
        vp[0][3] + vp[3][3]
    );
    Planes[kRight] = SetValues
    (
       -vp[0][0] + vp[3][0],
       -vp[0][1] + vp[3][1],
       -vp[0][2] + vp[3][2],
       -vp[0][3] + vp[3][3]
    );
    */

    glm::vec4 rowX = glm::row(vp, 0);
    glm::vec4 rowY = glm::row(vp, 1);
    glm::vec4 rowZ = glm::row(vp, 2);
    glm::vec4 rowW = glm::row(vp, 3);

    Planes[0] = SetValues(rowW + rowX);
    Planes[1] = SetValues(rowW - rowX);
    Planes[2] = SetValues(rowW + rowY);
    Planes[3] = SetValues(rowW - rowY);
    Planes[4] = SetValues(rowW + rowZ);
    Planes[5] = SetValues(rowW - rowZ);
}

FrustrumResult Frustrum::SphereTest(BoundingSphere & bs)
{
    for (unsigned int i = 0; i < 6; i++)
    {
        float dist =    Planes[i].x * bs.Position.x +
                        Planes[i].y * bs.Position.y +
                        Planes[i].z * bs.Position.z + 
                        Planes[i].w - bs.Radius;
        if (dist > 0.0f) return kOutside;
    }
    return kInside;
}

glm::vec4 Frustrum::SetValues(glm::vec4 p)
{
    glm::vec3 n = glm::vec3(p.x, p.y, p.z);
    float l = glm::length(n);
    return -p / l;
}

void LoadMeshFromFile(const char * file, MeshBasicVertexData& md)
{
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    tinyobj::attrib_t at;
    std::string err;
    bool ret = tinyobj::LoadObj(&at, &shapes, &materials, &err, file);
    if (!err.empty()) 
    {
        printf("ERROR:%s\n", err.c_str());
    }
    if (!ret) 
    {
        exit(1);
    }

    // Build vertex data
    for (unsigned int i = 0; i < at.vertices.size(); i += 3)
    {
        // Positions
        float px = at.vertices[i + 0];
        float py = at.vertices[i + 1];
        float pz = at.vertices[i + 2];
        // Uvs
        float ux, uy;
        ux = uy = 0.0f;
        if (at.texcoords.size() > 0)
        {
            //ux = at.texcoords[i + 0];
            //uy = at.texcoords[i + 1];
        }
        // Normals
        md.vertex.push_back(BasicVertex(px, py, pz, ux, uy));
    }

    // Build indices
    for (unsigned int s = 0; s < shapes.size(); s++)
    {
        for (unsigned int i = 0; i < shapes[s].mesh.indices.size(); i++)
        {
            unsigned int idx = shapes[s].mesh.indices[i].vertex_index;
            md.ele.push_back(idx);
        }
    }

    printf(" LOG: Loaded mesh:%s\n", shapes[0].name.c_str());
}

void GenerateSphere(float radius, int div, MeshBasicVertexData & md)
{
    float const R = 1.0f / (float)(div - 1);
    float const S = 1.0f / (float)(div - 1);
    float M_PI = 3.1415f;
    float M_PI_2 = M_PI * 0.5f;

    int r, s;
    float ux, uy;
    float px, py, pz;

    // Vertex data
    for (r = 0; r < div; r++)
    {
        for (s = 0; s < div; s++)
        {
            float const y = sin(-M_PI_2 + M_PI * r * R);
            float const x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
            float const z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);
            // Texcoord
            ux = s*S;
            uy = r*R;
            // Vertex
            px = x * radius;
            py = y * radius;
            pz = z * radius;
            // Normal
            //*n++ = x;
            //*n++ = y;
            //*n++ = z;
            md.vertex.push_back(BasicVertex(px, py, pz, ux, uy));
        }
    } 

    // Index
    for (r = 0; r < div; r++)
    {
        for (s = 0; s < div; s++)
        {
            md.ele.push_back(r * div + s);
            md.ele.push_back(r * div + (s + 1));
            md.ele.push_back((r + 1) * div + (s + 1));
            md.ele.push_back((r + 1) * div + s);
        }
    }
}

void LoadTextureFromFile(TextureDef& def)
{
    def.Data = stbi_load(def.Path, &def.Size.x, &def.Size.y, &def.ElePerPixel, 0);
}

void LoadTextureFromFileF(TextureDefF& def)
{
    def.Data = stbi_loadf(def.Path, &def.Size.x, &def.Size.y, &def.ElePerPixel, 0);
}
