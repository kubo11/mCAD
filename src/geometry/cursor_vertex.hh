#ifndef CMAD_GEOMETRY_CURSOR_VERTEX_HH
#define CMAD_GEOMETRY_CURSOR_VERTEX_HH

#include "mge.hh"

struct CursorVertex {
  glm::vec3 position = {0.0f, 0.0f, 0.0f};
  glm::vec3 color = {0.0f, 0.0f, 0.0f};

  CursorVertex() : position({0.0f, 0.0f, 0.0f}), color({0.0f, 0.0f, 0.0f}) {}
  CursorVertex(const glm::vec3& position, const glm::vec3& color)
      : position(position), color(color) {}

  inline static std::vector<mge::VertexAttribute> get_vertex_attributes() {
    return {{3, GL_FLOAT}, {3, GL_FLOAT}};
  }
};

#endif  // CMAD_GEOMETRY_CURSOR_VERTEX_HH