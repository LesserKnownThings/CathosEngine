#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 vColor;

layout (set = 0, binding = 0) uniform Camera {
    mat4 projectionView;
} camera;

layout (location = 0) flat out vec4 color; 

void main() {
    gl_Position = camera.projectionView * vec4(position, 1.0);
    color = vColor;
}
