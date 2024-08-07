#ifndef MCAD_VERTICES_POINT_INSTANCED_VERTEX
#define MCAD_VERTICES_POINT_INSTANCED_VERTEX

#include "mge.hh"

struct PointInstancedVertex {
  glm::vec3 position;
  glm::vec3 color;

  PointInstancedVertex() : position{0.0f, 0.0f, 0.0f}, color{0.0f, 0.0f, 0.0f} {}
  PointInstancedVertex(const glm::vec3& position, const glm::vec3& color) : position(position), color(color) {}

  inline static std::vector<mge::VertexInstanceAttribute> get_vertex_attributes() {
    return {{.size = 3, .type = GL_FLOAT, .divisor = 1}, {.size = 3, .type = GL_FLOAT, .divisor = 1}};
  }
};

inline bool operator==(const PointInstancedVertex& lhs, const PointInstancedVertex& rhs) {
  return lhs.position == rhs.position && lhs.color == rhs.color;
}

#endif  // MCAD_VERTICES_POINT_INSTANCED_VERTEX