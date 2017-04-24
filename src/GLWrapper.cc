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
            std::cout << " GL_ERROR:" << er << "(invalid enum):" << msg << std::endl;
            break;
        case GL_INVALID_VALUE:
            std::cout << " GL_ERROR:" << er << "(invalid value):" << msg << std::endl;
            break;
        case GL_INVALID_OPERATION:
            std::cout << " GL_ERROR:" << er << "(invalid operation):" << msg << std::endl;
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            std::cout << " GL_ERROR:" << er << "(invalid framebuffer operation):" << msg << std::endl;
            break;
        case GL_OUT_OF_MEMORY:
            std::cout << " GL_ERROR:" << er << "(out of memory):" << msg << std::endl;
            break;
        default:
            std::cout << " GL_ERROR:" << er << "(no description):" << msg << std::endl;
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
    case kSampling:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
    GLint loc = glGetUniformLocation(program, "uModel");
#ifdef _DEBUG
    if(loc == -1)
        printf("GL_ERROR: Could not set uniform:%s\n", "uModel");
#endif
    glUniformMatrix4fv(loc, 1, GL_FALSE, trans);
}

void glw::SetUniform1i(const char* name,GLint program, int * value)
{
    GLint loc = glGetUniformLocation(program, name);
#ifdef _DEBUG
    if (loc == -1)
        printf("GL_ERROR: Could not set uniform:%s\n", name);
#endif
    glUniform1iv(loc,1,value);
}

void glw::SetUniform1f(const char* name, GLint program, float * value)
{
    GLint loc = glGetUniformLocation(program, name);
#ifdef _DEBUG
    if(loc == -1)
        printf("GL_ERROR: Could not set uniform:%s\n", name);
#endif
    glUniform1fv(loc, 1, value);
}

void glw::SetUniform2f(const char * name, GLint program, float * value)
{
    GLint loc = glGetUniformLocation(program, name);
#ifdef _DEBUG
    if (loc == -1)
        printf("GL_ERROR: Could not set uniform:%s\n", name);
#endif
    glUniform2fv(loc, 1, value);
}

void glw::SetUniform3f(const char* name, GLint program, float * value)
{
    GLint loc = glGetUniformLocation(program, name);
#ifdef _DEBUG
    if (loc == -1)
        printf("GL_ERROR: Could not set uniform:%s\n", name);
#endif
    glUniform3fv(loc, 1, value);
}

void glw::SetUniform4f(const char * name, GLint program, float * value)
{
    GLint loc = glGetUniformLocation(program, name);
#ifdef _DEBUG
    if (loc == -1)
        printf("GL_ERROR: Could not set uniform:%s\n", name);
#endif
    glUniform4fv(loc, 1, value);
}

void glw::SetUniformTexture(const char * name, GLint program, unsigned int texId,unsigned int bindId)
{
    glActiveTexture(GL_TEXTURE0 + bindId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glUniform1i(glGetUniformLocation(program, name), bindId);
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

void glw::Mesh::Init(std::vector<BasicVertexPoint> vertex, std::vector<unsigned int> ele)
{
    GLsizeiptr eleSize = sizeof(unsigned int) * ele.size();
    GLsizeiptr vertexSize = sizeof(BasicVertexPoint) * vertex.size();
    NumElements = ele.size();

    GLsizeiptr posSize = sizeof(glm::vec3);

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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, posSize, (GLvoid*)0);
        glEnableVertexAttribArray(0);
    }
    glBindVertexArray(0);
}
void glw::Mesh::Init(std::vector<BasicVertexPoint2> vertex, std::vector<unsigned int> ele)
{
    GLsizeiptr eleSize = sizeof(unsigned int) * ele.size();
    GLsizeiptr vertexSize = sizeof(BasicVertexPoint2) * vertex.size();
    NumElements = ele.size();

    GLsizeiptr posSize = sizeof(glm::vec2);

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
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, posSize, (GLvoid*)0);
        glEnableVertexAttribArray(0);
    }
    glBindVertexArray(0);
}


void glw::Mesh::Render()
{
    glBindVertexArray(Id);
    switch (DMode)
    {
    case kTriangles:
        glDrawElements(GL_TRIANGLES, NumElements, GL_UNSIGNED_INT, 0);
        break;
    case kPatches3:
        glPatchParameteri(GL_PATCH_VERTICES, 3);
        glDrawElements(GL_PATCHES, NumElements, GL_UNSIGNED_INT, 0);
        break;
    case kQuad:
        glDrawElements(GL_QUADS, NumElements / 1, GL_UNSIGNED_INT, 0);
        break;
    case kPoints:
        glDrawElements(GL_POINTS, NumElements, GL_UNSIGNED_INT, 0);
        break;
    }
    glBindVertexArray(0);
}

void glw::InstancedMesh::InitInstances(unsigned int maxInstances, BufferUsage usage)
{
    mMaxInstances = maxInstances;
    mUsage = usage;
    GLenum use;
    switch (mUsage)
    {
    case kStatic:
        use = GL_STATIC_DRAW;
        break;
    case kDynamic:
        use = GL_DYNAMIC_DRAW;
        break;
    default:
        break;
    }

    glBindVertexArray(IMesh.Id);
    {
        // Transforms matrix buffer
        glGenBuffers(1, &TransformsId);
        glBindBuffer(GL_ARRAY_BUFFER, TransformsId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4)*mMaxInstances, nullptr, use);
        
        // Setup the atribute
        GLuint location = 6;    // Maybe hit performance dunno   
        GLint components = 4;
        GLenum type = GL_FLOAT;
        GLboolean normalized = GL_FALSE;
        GLsizei datasize = sizeof(glm::mat4);
        char* pointer = 0;
        GLuint divisor = 1;

        for (unsigned int i = 0; i < 4; i++)
        {
            glEnableVertexAttribArray(location + i); 
            glVertexAttribPointer(location + i, components, type, normalized, datasize, pointer + i * sizeof(glm::vec4));
            glVertexAttribDivisor(location + i, divisor);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
}

void glw::InstancedMesh::Render(std::vector<glm::mat4>& transforms)
{
    // The transforms buffer is initialized once so we
    // check to prevent memory overlaping
    if (transforms.size() > mMaxInstances)
    {
        printf("GL_ERROR: transforms.size() should be equal or less than the requested initial size.\n");
        return;
    }

    glBindVertexArray(IMesh.Id);
    // Update the transforms buffer
    glBindBuffer(GL_ARRAY_BUFFER, TransformsId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * transforms.size(), transforms.data());
    //glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * transforms.size(), &transforms[0][0],GL_DYNAMIC_DRAW);
    switch (IMesh.DMode)
    {
    case kTriangles:
        glDrawElementsInstanced(GL_TRIANGLES, IMesh.NumElements, GL_UNSIGNED_INT, 0, transforms.size());
        break;
    case kPatches3:
        glPatchParameteri(GL_PATCH_VERTICES, 3);
        glDrawElementsInstanced(GL_PATCHES, IMesh.NumElements, GL_UNSIGNED_INT, 0, transforms.size());
        break;
    case kQuad:
        glDrawElementsInstanced(GL_QUADS, IMesh.NumElements / 1, GL_UNSIGNED_INT, 0, transforms.size());
        break;
    case kPoints:
        glDrawElementsInstanced(GL_POINTS, IMesh.NumElements, GL_UNSIGNED_INT, 0,transforms.size());
        break;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    case kTessControl:
        t = GL_TESS_CONTROL_SHADER;
        break;
    case kTessEval:
        t = GL_TESS_EVALUATION_SHADER;
        break;
    case kGeometry:
        t = GL_GEOMETRY_SHADER;
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
    Vs.Init(vs, ShaderType::kVertex);
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


glw::MaterialTess::MaterialTess():
    Id(0)
{

}

bool glw::MaterialTess::Init(const char * vs, const char * fs, const char * tc, const char * te)
{
    Vs.Init(vs, ShaderType::kVertex);
    Fs.Init(fs, ShaderType::kFragment);
    Tc.Init(tc, ShaderType::kTessControl);
    Te.Init(te, ShaderType::kTessEval);

    // Program and link
    GLint s = -1;
    GLchar log[512];
    Id = glCreateProgram();
    glAttachShader(Id, Vs.Id);
    glAttachShader(Id, Fs.Id);
    glAttachShader(Id, Tc.Id);
    glAttachShader(Id, Te.Id);
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

void glw::MaterialTess::Use()
{
    glUseProgram(Id);
}

void glw::MaterialTess::Release()
{
    Vs.Release();
    Fs.Release();
    Tc.Release();
    Te.Release();
    glDeleteProgram(Id);
}

glw::MaterialGeo::MaterialGeo():
    Id(0)
{
}

bool glw::MaterialGeo::Init(const char * vs, const char * fs, const char * gs)
{
    Vs.Init(vs, ShaderType::kVertex);
    Fs.Init(fs, ShaderType::kFragment);
    Gs.Init(gs, ShaderType::kGeometry);

    // Program and link
    GLint s = -1;
    GLchar log[512];
    Id = glCreateProgram();
    glAttachShader(Id, Vs.Id);
    glAttachShader(Id, Fs.Id);
    glAttachShader(Id, Gs.Id);
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

void glw::MaterialGeo::Use()
{
    glUseProgram(Id);
}

void glw::MaterialGeo::Release()
{
    Vs.Release();
    Fs.Release();
    Gs.Release();
}


void glw::PassConstants::Init()
{
    const int mat4Size = sizeof(glm::mat4);
    const int vec3Size = sizeof(glm::vec3);
    const int floatSize = sizeof(float);
    int size = (2 * mat4Size) + (1 * vec3Size) + (4 * floatSize);

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
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * mat4Size + vec3Size + (3 * floatSize), floatSize, &PAspect);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void glw::Texture::Init(TextureDef def)
{
    Def = def;

    glGenTextures(1, &Id);
    glBindTexture(GL_TEXTURE_2D, Id);
    
    unsigned char* data = nullptr;
    GLint elePerPixel = 0;
    GLint x;
    GLint y;
    
    if (Def.Path &&  !Def.Data)
    {
        data = stbi_load(def.Path, &x, &y, &elePerPixel, 0);
        Def.Data = data;
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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, Def.Size.x, Def.Size.y, 0, GL_RED, GL_FLOAT, stbi_loadf(def.Path, &x, &y, &elePerPixel, 0));
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Def.Size.x, Def.Size.y, 0, GL_RGB, GL_FLOAT, data);
        SetTextureParameter(Def);
        break;
    case kRenderTargetDepth:    
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, Def.Size.x, Def.Size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, Def.Size.x, Def.Size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
        SetTextureParameter(Def);
        break;
    case kSampling:
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Def.Size.x, Def.Size.y, 0, GL_RGB, GL_FLOAT, nullptr);
        break;
    case kRenderTargetDepth:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, Def.Size.x, Def.Size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
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

void glw::GpuTimer::Init()
{
	glGenQueries(2, &Ids[0]);

	// Dummy call so the double buffer works
	glQueryCounter(Ids[CurFront], GL_TIMESTAMP);
}

void glw::GpuTimer::Release()
{
	glDeleteQueries(2, &Ids[0]);
}

void glw::GpuTimer::Start()
{
	glBeginQuery(GL_TIME_ELAPSED, Ids[CurBack]);
}

float glw::GpuTimer::End()
{
	GLuint64 timeStart = 0;
	GLuint64 timeEnd = 0;

	glQueryCounter(Ids[CurFront][1], GL_TIMESTAMP);
	glGetQueryObjectui64v(Ids[CurFront][0], GL_QUERY_RESULT, &timeStart);
	glGetQueryObjectui64v(Ids[CurFront][1], GL_QUERY_RESULT, &timeEnd);
	float e = ((float)timeEnd - (float)timeStart) / 1000000.0f;

	// Swap buffers
	if (CurBack)
	{
		CurBack  = 0;
		CurFront = 1;
	}
	else
	{
		CurBack  = 1;
		CurFront = 0;
	}

	return e;
}


