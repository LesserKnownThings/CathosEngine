#version 460

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 fragUV;
layout(location = 1) flat in int vInstanceIndex;

// TODO convert the size to constant_id
layout(set = 2, binding = 0) uniform sampler2D globalTextures[1000];
// Roughness
// Metallic
// Normal

struct MaterialData
{
    vec4 baseColor;
    int albedoIndex;
    int _padding[3];
};

layout(set = 1, binding = 1) readonly buffer Material
{
    MaterialData data[];
} material;

void main(){
    vec4 finalColor = texture(globalTextures[material.data[vInstanceIndex].albedoIndex], fragUV);
    fragColor = finalColor * material.data[vInstanceIndex].baseColor;
}