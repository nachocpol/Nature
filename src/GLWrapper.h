/*
    GLWrapper.h nachocpol@gmail.com
*/

#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include "Types.h"
#include <vector>

namespace glw
{
    // This is the binding point for the
    // pass constants (projection,view,time etc)
    const int kPassBinding = 0;

    void GetError();

    void SetTextureParameter(TextureDef def);

    void SetClipPlane(GLint indx, glm::vec4 plane,GLuint program);

    void SetTransform(GLint program, float* trans);

    struct Mesh
    {
        void Init(std::vector<BasicVertex> vertex,std::vector<unsigned int> ele);
        void Draw();

        GLuint Id;
        GLuint Ebo;
        GLuint Vbo;
        GLuint NumElements;
    };

    struct Shader
    {
        Shader();
        void Init(const char* path,ShaderType type);
        void Release();

        GLuint Id;
    };

    struct Material
    {
        Material();
        bool Init(const char* vs,const char* fs);
        void Use();
        void Release();

        Shader Vs;
        Shader Fs;
        GLuint Id;
    };

    struct PassConstants
    {
        void Init();
        void Update();

        GLuint Id;
        glm::mat4 PView;
        glm::mat4 PProjection;
        glm::vec3 PCamPos;
        float PTime;
        float PCamNear;
        float PCamFar;
    };

    struct Texture
    {
        //Texture():Id(0), Def(TextureDef()){}
        void Init(TextureDef def);
        void Resize(glm::vec2 size);
        void Release();

        GLuint Id;
        TextureDef Def;
    };

    struct RenderTarget
    {
        void Init(glm::vec2 size,bool hasDepth = false);
        void Resize(glm::vec2 size);
        void Release();
        void Enable();
        void Disable();

        bool HasDepth;
        glm::vec2 Size;
        GLuint Id;
        Texture RenderTexture;
        Texture DepthTexture;
    };
}
