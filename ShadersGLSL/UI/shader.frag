#version 460

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in flat uint textureIndex;

layout(set = 0, binding = 0) uniform sampler2D globalTextures[1000];

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = fragColor * texture(globalTextures[textureIndex], fragUV);
    // outColor = vec4(fragUV, 0.0, 1.0);
}