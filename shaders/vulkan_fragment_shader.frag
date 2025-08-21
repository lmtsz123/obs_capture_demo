#version 450

layout(binding = 0) uniform sampler2D yTexture;
layout(binding = 1) uniform sampler2D uvTexture;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
    float y = texture(yTexture, fragTexCoord).r;
    vec2 uv = texture(uvTexture, fragTexCoord).rg;
    
    // NV12 to RGB conversion using ITU-R BT.601 standard
    // Normalize Y from [16/255, 235/255] to [0, 1]
    y = (y * 255.0 - 16.0) / 219.0;
    
    // Normalize UV from [16/255, 240/255] to [-0.5, 0.5]
    float u = (uv.r * 255.0 - 128.0) / 224.0;
    float v = (uv.g * 255.0 - 128.0) / 224.0;
    
    // ITU-R BT.601 conversion matrix
    float r = y + 1.402 * v;
    float g = y - 0.344136 * u - 0.714136 * v;
    float b = y + 1.772 * u;
    
    // Clamp to [0, 1] range
    r = clamp(r, 0.0, 1.0);
    g = clamp(g, 0.0, 1.0);
    b = clamp(b, 0.0, 1.0);
    
    outColor = vec4(r, g, b, 1.0);
}
