#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 uv;

layout (location = 5) in ivec4 boneIDS;
layout (location = 6) in vec4 weights;

layout (set = 0, binding = 0) uniform Camera {
    mat4 projectionView;
} camera;

layout (set = 1, binding = 0) readonly buffer InstanceBuffer {
    mat4 models[];
} instanceBuffer; 

layout (location = 0) out vec2 fragUV;
layout (location = 1) flat out int vInstanceIndex; 

void main() {
    vInstanceIndex = gl_InstanceIndex;
    
    mat4 model = instanceBuffer.models[gl_InstanceIndex];
    gl_Position = camera.projectionView * model * vec4(position, 1.0);
    fragUV = uv;
}
