#pragma once

#include <glm/glm.hpp>
#include <vector>

struct GizmosVertex
{
    glm::vec3 position;
    glm::vec4 color;
};

class Gizmos
{
  public:
    static Gizmos& Get();

    static void DrawCube(const glm::vec4& color, const glm::vec3& position, const glm::vec3& halfExtents);
    static void DrawSphere(const glm::vec4& color, const glm::vec3& center, float radius, int32_t segments = 16);

  private:
    std::vector<GizmosVertex> verts;

    friend class RenderingSystem;
};