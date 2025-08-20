#include "nv12_to_rgba_shader.h"
#include <iostream>
#include <vector>

// OpenGL扩展函数指针定义
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORM1IPROC glUniform1i = nullptr;
PFNGLUNIFORM2FPROC glUniform2f = nullptr;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;
PFNGLACTIVETEXTUREPROC glActiveTexture_ext = nullptr;

// 初始化OpenGL扩展函数
bool InitializeOpenGLExtensions() {
    glCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
    glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
    glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)SDL_GL_GetProcAddress("glDeleteProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");
    glUniform2f = (PFNGLUNIFORM2FPROC)SDL_GL_GetProcAddress("glUniform2f");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
    glGenBuffers = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
    glActiveTexture_ext = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");
    
    // 检查所有函数是否成功加载
    return glCreateShader && glShaderSource && glCompileShader && glGetShaderiv &&
           glGetShaderInfoLog && glDeleteShader && glCreateProgram && glAttachShader &&
           glLinkProgram && glGetProgramiv && glGetProgramInfoLog && glUseProgram &&
           glDeleteProgram && glGetUniformLocation && glUniform1i && glUniform2f &&
           glGenVertexArrays && glBindVertexArray && glDeleteVertexArrays &&
           glGenBuffers && glBindBuffer && glBufferData && glDeleteBuffers &&
           glVertexAttribPointer && glEnableVertexAttribArray && glGenFramebuffers &&
           glDeleteFramebuffers && glActiveTexture_ext;
}

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D yTexture;
uniform sampler2D uvTexture;
uniform vec2 textureSize;

void main()
{
    float y = texture(yTexture, TexCoord).r;
    vec2 uv = texture(uvTexture, TexCoord).rg;
    
    float u = uv.r - 0.5;
    float v = uv.g - 0.5;
    
    // YUV to RGB conversion (BT.601)
    float r = y + 1.402 * v;
    float g = y - 0.344136 * u - 0.714136 * v;
    float b = y + 1.772 * u;
    
    FragColor = vec4(r, g, b, 1.0);
}
)";

OpenGLRenderer::OpenGLRenderer() 
    : m_program(0), m_vertexShader(0), m_fragmentShader(0),
      m_yTexture(0), m_uvTexture(0), m_framebuffer(0),
      m_rgbaTexture(0), m_vao(0), m_vbo(0) {
}

OpenGLRenderer::~OpenGLRenderer() {
    Cleanup();
}

bool OpenGLRenderer::Initialize() {
    // 创建着色器
    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    if (!CompileShader(m_vertexShader, vertexShaderSource) ||
        !CompileShader(m_fragmentShader, fragmentShaderSource)) {
        return false;
    }
    
    // 创建程序
    m_program = glCreateProgram();
    glAttachShader(m_program, m_vertexShader);
    glAttachShader(m_program, m_fragmentShader);
    
    if (!LinkProgram()) {
        return false;
    }
    
    // 获取uniform位置
    m_yTextureLocation = glGetUniformLocation(m_program, "yTexture");
    m_uvTextureLocation = glGetUniformLocation(m_program, "uvTexture");
    m_textureSizeLocation = glGetUniformLocation(m_program, "textureSize");
    
    // 创建纹理
    glGenTextures(1, &m_yTexture);
    glGenTextures(1, &m_uvTexture);
    glGenTextures(1, &m_rgbaTexture);
    
    // 创建帧缓冲
    glGenFramebuffers(1, &m_framebuffer);
    
    // 设置四边形
    SetupQuad();
    
    return true;
}

bool OpenGLRenderer::CompileShader(GLuint shader, const std::string& source) {
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        return false;
    }
    
    return true;
}

bool OpenGLRenderer::LinkProgram() {
    glLinkProgram(m_program);
    
    GLint success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
        return false;
    }
    
    return true;
}

void OpenGLRenderer::SetupQuad() {
    float vertices[] = {
        // positions   // texture coords
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 0.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    GLuint ebo;
    glGenBuffers(1, &ebo);
    
    glBindVertexArray(m_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void OpenGLRenderer::RenderToScreen(const uint8_t* nv12Data, int width, int height) {
    if (!nv12Data) return;
    
    // 只在第一次或尺寸改变时更新纹理参数
    static int lastWidth = 0, lastHeight = 0;
    bool sizeChanged = (width != lastWidth || height != lastHeight);
    
    static int renderCallCount = 0;
    renderCallCount++;
    if (renderCallCount <= 3) {
        std::cout << "OpenGL RenderToScreen " << renderCallCount << ": " << width << "x" << height 
                  << ", sizeChanged: " << (sizeChanged ? "yes" : "no") << std::endl;
    }
    
    // 更新Y纹理数据
    glActiveTexture_ext(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_yTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nv12Data);
    
    if (sizeChanged) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    // 更新UV纹理数据
    glActiveTexture_ext(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_uvTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width / 2, height / 2, 0, GL_RG, GL_UNSIGNED_BYTE, 
                nv12Data + width * height);
    
    if (sizeChanged) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        lastWidth = width;
        lastHeight = height;
    }
    
    // 使用着色器程序
    glUseProgram(m_program);
    
    // 设置uniform（只在尺寸改变时更新）
    glUniform1i(m_yTextureLocation, 0);
    glUniform1i(m_uvTextureLocation, 1);
    if (sizeChanged) {
        glUniform2f(m_textureSizeLocation, static_cast<float>(width), static_cast<float>(height));
    }
    
    // 渲染四边形
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

bool OpenGLRenderer::ConvertFrame(const uint8_t* nv12Data, int width, int height, 
                                   uint8_t* rgbaOutput) {
    // 这个函数可以用于离屏渲染到纹理然后读取RGBA数据
    // 这里简化实现，直接使用CPU转换
    
    const uint8_t* yPlane = nv12Data;
    const uint8_t* uvPlane = nv12Data + width * height;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int yIndex = y * width + x;
            int uvIndex = (y / 2) * width + (x & ~1);
            
            float yVal = yPlane[yIndex] / 255.0f;
            float uVal = (uvPlane[uvIndex] - 128) / 255.0f;
            float vVal = (uvPlane[uvIndex + 1] - 128) / 255.0f;
            
            float r = yVal + 1.402f * vVal;
            float g = yVal - 0.344136f * uVal - 0.714136f * vVal;
            float b = yVal + 1.772f * uVal;
            
            int rgbaIndex = (y * width + x) * 4;
            rgbaOutput[rgbaIndex + 0] = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, r * 255.0f)));
            rgbaOutput[rgbaIndex + 1] = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, g * 255.0f)));
            rgbaOutput[rgbaIndex + 2] = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, b * 255.0f)));
            rgbaOutput[rgbaIndex + 3] = 255;
        }
    }
    
    return true;
}

void OpenGLRenderer::Cleanup() {
    if (m_program) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
    
    if (m_vertexShader) {
        glDeleteShader(m_vertexShader);
        m_vertexShader = 0;
    }
    
    if (m_fragmentShader) {
        glDeleteShader(m_fragmentShader);
        m_fragmentShader = 0;
    }
    
    if (m_yTexture) {
        glDeleteTextures(1, &m_yTexture);
        m_yTexture = 0;
    }
    
    if (m_uvTexture) {
        glDeleteTextures(1, &m_uvTexture);
        m_uvTexture = 0;
    }
    
    if (m_rgbaTexture) {
        glDeleteTextures(1, &m_rgbaTexture);
        m_rgbaTexture = 0;
    }
    
    if (m_framebuffer) {
        glDeleteFramebuffers(1, &m_framebuffer);
        m_framebuffer = 0;
    }
    
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
}