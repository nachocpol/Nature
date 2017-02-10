/*
    GLWrapper.cc nachocpol@gmail.com
*/

#include "GLWrapper.h"
#include "Utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

void glw::GetError()
{
    GLenum er = glGetError();
    const char* msg = ".";
    if (er != GL_NO_ERROR)
    {
        switch (er)
        {
        case GL_INVALID_ENUM:
            std::cout << "GL_ERROR:" << er << "(invalid enum):" << msg << std::endl;
            break;
        case GL_INVALID_VALUE:
            std::cout << "GL_ERROR:" << er << "(invalid value):" << msg << std::endl;
            break;
        case GL_INVALID_OPERATION:
            std::cout << "GL_ERROR:" << er << "(invalid operation):" << msg << std::endl;
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            std::cout << "GL_ERROR:" << er << "(invalid framebuffer operation):" << msg << std::endl;
            break;
        case GL_OUT_OF_MEMORY:
            std::cout << "GL_ERROR:" << er << "(out of memory):" << msg << std::endl;
            break;
        default:
            std::cout << "GL_ERROR:" << er << "(no description):" << msg << std::endl;
            break;
        }
    }
}

void glw::SetTextureParameter(TextureDef def)
{
    switch (def.Usage)  
    {
    case kTexturing:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        break;
    case KRenderTarget:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    case kRenderTargetDepth:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    }
}

void glw::SetClipPlane(GLint indx, glm::vec4 plane, GLuint program)
{
    glEnable(GL_CLIP_DISTANCE0 + indx);
    GLuint loc = glGetUniformLocation(program, "uClipPlane");
    glUniform4fv(loc, 1, &plane.x);
}

void glw::SetTransform(GLint program, float* trans)
{
    GLuint loc = glGetUniformLocation(program, "uModel");
    glUniformMatrix4fv(loc, 1, GL_FALSE, trans);
}

void glw::Mesh::Init(std::vector<BasicVertex> vertex, std::vector<unsigned int> ele)
{
    GLsizeiptr eleSize = sizeof(unsigned int) * ele.size();
    GLsizeiptr vertexSize = sizeof(BasicVertex) * vertex.size();
    NumElements = ele.size();

    GLsizeiptr posSize = sizeof(glm::vec3);
    GLsizeiptr uvSize = sizeof(glm::vec2);

    glGenVertexArrays(1, &Id);
    glBindVertexArray(Id);
    {
        // Generate and upload element buffer
        glGenBuffers(1, &Ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleSize, ele.data(), GL_STATIC_DRAW);

        // Generate and upload vertex buffer
        glGenBuffers(1, &Vbo);
        glBindBuffer(GL_ARRAY_BUFFER, Vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexSize, vertex.data(), GL_STATIC_DRAW);

        // Setup vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, posSize + uvSize, (GLvoid*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, posSize + uvSize,(GLvoid*)posSize);
        glEnableVertexAttribArray(1);

    }
    glBindVertexArray(0);
}

void glw::Mesh::Draw()
{
    glBindVertexArray(Id);
    {
        glDrawElements(GL_TRIANGLES, NumElements, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

glw::Shader::Shader():
    Id(0)
{
}

void glw::Shader::Init(const char * path, ShaderType type)
{
    GLenum t;
    switch (type)
    {
    case kVertex:
        t = GL_VERTEX_SHADER;
        break;
    case kFragment:
        t = GL_FRAGMENT_SHADER;
        break;
    default:
        break;
    }
    
    // Load shader source (log error?)
    std::string src = util::LoadTextFromFile(path);
    const GLchar* cSrc = src.c_str();

    GLint s = -1;
    Id = glCreateShader(t);
    glShaderSource(Id, 1,&cSrc, nullptr);
    glCompileShader(Id);
    glGetShaderiv(Id, GL_COMPILE_STATUS,&s);
    if (!s)
    {
        printf(" ERROR: Failed to compile shader:%s.\n", path);
    }

}

void glw::Shader::Release()
{
    glDeleteShader(Id);
}

glw::Material::Material():
    Id(0)
{
}

bool glw::Material::Init(const char * vs, const char * fs)
{
    // Vertex shader
    Vs.Init(vs, ShaderType::kVertex);

    // Fragment shader
    Fs.Init(fs, ShaderType::kFragment);

    // Program and link
    GLint s = -1;
    GLchar log[512];
    Id = glCreateProgram();
    glAttachShader(Id, Vs.Id);
    glAttachShader(Id, Fs.Id);
    glLinkProgram(Id);
    glGetProgramiv(Id, GL_LINK_STATUS, &s);
    if (!s)
    {
        glGetProgramInfoLog(Id, 512, nullptr, log);
        printf(" ERROR: Failed to create material:\n %s \n", log);
        Release();
        return false;
    }

    // Setup pass ubo
    glUniformBlockBinding(Id, glGetUniformBlockIndex(Id, "uPass"), kPassBinding);

    return true;
}

void glw::Material::Use()
{
    glUseProgram(Id);
}

void glw::Material::Release()
{
    Vs.Release();
    Fs.Release();
    glDeleteProgram(Id);
}

void glw::PassConstants::Init()
{
    const int mat4Size = sizeof(glm::mat4);
    const int vec3Size = sizeof(glm::vec3);
    const int floatSize = sizeof(float);
    int size = (2 * mat4Size) + (1 * vec3Size) + (3 * floatSize);

    glCreateBuffers(1, &Id);
    glBindBuffer(GL_UNIFORM_BUFFER, Id);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, kPassBinding, Id, 0, size);
}

void glw::PassConstants::Update()
{
    const int mat4Size = sizeof(glm::mat4);
    const int vec3Size = sizeof(glm::vec3);
    const int floatSize = sizeof(float);

    glBindBuffer(GL_UNIFORM_BUFFER, Id);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, mat4Size, &PView[0][0]);
    glBufferSubData(GL_UNIFORM_BUFFER, mat4Size, mat4Size, &PProjection[0][0]);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * mat4Size, vec3Size, &PCamPos.x);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * mat4Size + vec3Size, floatSize,&PTime);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * mat4Size + vec3Size + (1 * floatSize), floatSize, &PCamNear);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * mat4Size + vec3Size + (2 * floatSize), floatSize, &PCamFar);
 
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void glw::Texture::Init(TextureDef def)
{
    Def = def;

    glGenTextures(1, &Id);
    glBindTexture(GL_TEXTURE_2D, Id);
    
    unsigned char* data = nullptr;
    GLint elePerPixel = 0;

    if (Def.Path &&  !Def.Data)
    {
        GLint x;
        GLint y;
        data = stbi_load(def.Path, &x, &y, &elePerPixel, 0);
        if (!data)
        {
            printf(" ERROR: Could not load image:%s\n", def.Path);
        }
        else
        {
            Def.Size = glm::vec2(x, y);
        }
    }
    if (Def.Data)
    {
        data = Def.Data;
    }

    // Initialize depending on the usage of the texture
    switch (Def.Usage)
    {
    case kTexturing:
        switch (elePerPixel)
        {
        case 1:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Def.Size.x, Def.Size.y, 0, GL_RED, GL_UNSIGNED_BYTE, data);
            SetTextureParameter(Def);
            break;
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Def.Size.x, Def.Size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            SetTextureParameter(Def);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Def.Size.x, Def.Size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            SetTextureParameter(Def);
            break;
        default:   //hack to work with procedural texture
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Def.Size.x, Def.Size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            SetTextureParameter(Def);
            break;
        }
        break;
    case KRenderTarget: 
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Def.Size.x, Def.Size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        SetTextureParameter(Def);
        break;
    case kRenderTargetDepth:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, Def.Size.x, Def.Size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
        SetTextureParameter(Def);
        break;
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    if(!Def.Data)
        stbi_image_free(data);
}

void glw::Texture::Resize(glm::vec2 size)
{
    Def.Size = size;
    glBindTexture(GL_TEXTURE_2D, Id);
    switch (Def.Usage)
    {
    case kTexturing:
        break;
    case KRenderTarget:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Def.Size.x, Def.Size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        break;
    case kRenderTargetDepth:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, Def.Size.x, Def.Size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        break;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void glw::Texture::Release()
{
    glDeleteTextures(1, &Id);
}

void glw::RenderTarget::Init(glm::vec2 size, bool hasDepth)
{
    Size = size;
    TextureDef texDef;

    // Render texture
    texDef.Path = nullptr;
    texDef.Size = Size;
    texDef.Usage = KRenderTarget;
    RenderTexture.Init(texDef);

    // Depth texture
    HasDepth = hasDepth;
    if (hasDepth)
    {
        texDef.Path = nullptr;
        texDef.Size = size;
        texDef.Usage = kRenderTargetDepth;
        DepthTexture.Init(texDef);
    }

    // Setup fbo
    glGenFramebuffers(1, &Id);
    glBindFramebuffer(GL_FRAMEBUFFER, Id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTexture.Id, 0);
    if (HasDepth)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, DepthTexture.Id, 0);
    }
    // Check status
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf(" ERROR:Framebuffer is not complete (%i).\n", status);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void glw::RenderTarget::Resize(glm::vec2 size)
{
    RenderTexture.Resize(size);
    if (HasDepth)
    {
        DepthTexture.Resize(size);
    }
    Size = size;
}

void glw::RenderTarget::Release()
{
    RenderTexture.Release();
    if (HasDepth)
    {
        DepthTexture.Release();
    }
}

void glw::RenderTarget::Enable()
{
    glBindFramebuffer(GL_FRAMEBUFFER, Id);
    glViewport(0, 0, Size.x, Size.y);
    GLbitfield clearMask = GL_COLOR_BUFFER_BIT;
    if (HasDepth)
    {
        clearMask |= GL_DEPTH_BUFFER_BIT;
    }
    glClear(clearMask);
}

void glw::RenderTarget::Disable()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

