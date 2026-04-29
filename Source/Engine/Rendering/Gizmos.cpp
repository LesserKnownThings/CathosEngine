#include "Gizmos.h"
#include "Math/Math.h"
#include <vector>

Gizmos& Gizmos::Get()
{
    static Gizmos instance;
    return instance;
}

void Gizmos::DrawCube(const glm::vec4& color, const glm::vec3& pos, const glm::vec3& he)
{
    auto& verts = Gizmos::Get().verts;

    glm::vec3 corners[8] = {
        pos + glm::vec3(-he.x, -he.y, -he.z),
        pos + glm::vec3(he.x, -he.y, -he.z),
        pos + glm::vec3(he.x, he.y, -he.z),
        pos + glm::vec3(-he.x, he.y, -he.z),
        pos + glm::vec3(-he.x, -he.y, he.z),
        pos + glm::vec3(he.x, -he.y, he.z),
        pos + glm::vec3(he.x, he.y, he.z),
        pos + glm::vec3(-he.x, he.y, he.z),
    };

    auto addLine = [&](int a, int b)
    {
        verts.push_back(GizmosVertex{ corners[a], color });
        verts.push_back({ corners[b], color });
    };

    // bottom
    addLine(0, 1);
    addLine(1, 2);
    addLine(2, 3);
    addLine(3, 0);
    // top
    addLine(4, 5);
    addLine(5, 6);
    addLine(6, 7);
    addLine(7, 4);
    // sides
    addLine(0, 4);
    addLine(1, 5);
    addLine(2, 6);
    addLine(3, 7);
}

void Gizmos::DrawSphere(const glm::vec4& color, const glm::vec3& center, float radius, int32_t segments)
{
    std::vector<GizmosVertex>& verts = Gizmos::Get().verts;

    auto drawCircle = [&](int axisA, int axisB)
    {
        for (int i = 0; i < segments; i++)
        {
            float t0 = (i / (float)segments) * 2.0f * Math::PI;
            float t1 = ((i + 1) / (float)segments) * 2.0f * Math::PI;

            glm::vec3 p0(0.0f);
            glm::vec3 p1(0.0f);

            p0[axisA] = cos(t0) * radius;
            p0[axisB] = sin(t0) * radius;

            p1[axisA] = cos(t1) * radius;
            p1[axisB] = sin(t1) * radius;

            verts.push_back({ center + p0, color });
            verts.push_back({ center + p1, color });
        }
    };

    // XY
    drawCircle(0, 1);
    // XZ
    drawCircle(0, 2);
    // YZ
    drawCircle(1, 2);
}
