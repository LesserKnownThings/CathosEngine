#version 460

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in flat uint textureIndex;

layout(set = 0, binding = 0) uniform sampler2D globalTextures[1000];

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants
{
    layout(offset = 64) float pixelRange;
}
pushConstants;

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float ScreenPixelDistance()
{
    vec2 unitRange = vec2(pushConstants.pixelRange) / vec2(textureSize(globalTextures[textureIndex], 0));
    vec2 screenTexSize = vec2(1.0) / fwidth(fragUV);

    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

void main()
{
    vec3 msd = texture(globalTextures[textureIndex], fragUV).rgb;

    float sd = median(msd.x, msd.y, msd.z);

    float screenPixelDistance = ScreenPixelDistance() * (sd - 0.5);

    float opacity = clamp(screenPixelDistance + 0.5, 0.0, 1.0);

    if (opacity < 0.001)
        discard;

    outColor = vec4(fragColor.rgb, fragColor.a * opacity);
}
