#version 460

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

layout (location = 2) in vec2 inPosition;
layout (location = 3) in vec2 inSize;

layout (location = 4) in vec4 inUVRect;

layout (location = 5) in vec4 inColor;

layout (location = 6) in uint inTextureIndex;

layout (location = 0) out vec2 fragUV;
layout (location = 1) out vec4 fragColor;
layout (location = 2) out flat uint textureIndex;

layout (push_constant) uniform PushConstants {
    mat4 projection;
} pushConstants;

// Will pack / unpack later during optimization
// vec4 unpackColor(uint c) {
//     return vec4(
//         ((c >> 0) & 0xFF) / 255.0,
//         ((c >> 8) & 0xFF) / 255.0,
//         ((c >> 16) & 0xFF) / 255.0,
//         ((c >> 24) & 0xFF) / 255.0
//     );
// }

void main() {
    vec2 worldPos = inPosition + (position * inSize);

    gl_Position = pushConstants.projection * vec4(worldPos, 0.0, 1.0);

    fragUV = mix(inUVRect.xy, inUVRect.zw, uv);
    textureIndex = inTextureIndex;
    fragColor = inColor;//unpackColor(inColor);
}