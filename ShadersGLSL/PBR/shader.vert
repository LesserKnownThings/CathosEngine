#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 uv;

layout (location = 5) in ivec4 boneIDS;
layout (location = 6) in vec4 weights;

layout (set = 0, binding = 0) uniform Camera {
    mat4 projection;
    mat4 view;
} camera;

layout (push_constant, std430) uniform SharedConstants {
    mat4 model;
} sharedConstants;

layout (location = 0) out vec2 fragUV;

void main() {
    gl_Position = camera.projection * camera.view  * sharedConstants.model * vec4(position, 1.0);
    fragUV = uv;
}
