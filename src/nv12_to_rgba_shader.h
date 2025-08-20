#pragma once

#include "renderer_interface.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <string>

// OpenGL扩展函数指针
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM2FPROC glUniform2f;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLACTIVETEXTUREPROC glActiveTexture_ext;

// 初始化OpenGL扩展函数
bool InitializeOpenGLExtensions();

class OpenGLRenderer : public IRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer() override;

    bool Initialize() override;
    void Cleanup() override;
    
    bool ConvertFrame(const uint8_t* nv12Data, int width, int height, 
                     uint8_t* rgbaOutput) override;
    
    void RenderToScreen(const uint8_t* nv12Data, int width, int height) override;
    
    RendererType GetType() const override { return RendererType::OpenGL; }

private:
    GLuint m_program;
    GLuint m_vertexShader;
    GLuint m_fragmentShader;
    GLuint m_yTexture;
    GLuint m_uvTexture;
    GLuint m_framebuffer;
    GLuint m_rgbaTexture;
    GLuint m_vao;
    GLuint m_vbo;
    
    GLint m_yTextureLocation;
    GLint m_uvTextureLocation;
    GLint m_textureSizeLocation;
    
    bool CompileShader(GLuint shader, const std::string& source);
    bool LinkProgram();
    void SetupQuad();
};