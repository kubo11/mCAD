#ifndef MCAD_VERTICES_GRID_VERTEX
#define MCAD_VERTICES_GRID_VERTEX

#include "mge.hh"

struct GridVertex {
  glm::vec2 position = {0.0f, 0.0f};

  GridVertex() : position({0.0f, 0.0f}) {}
  GridVertex(const glm::vec2& position) : position(position) {}

  inline static std::vector<mge::VertexAttribute> get_vertex_attributes() { return {{2, GL_FLOAT}}; }
};

#endif  // MCAD_VERTICES_GRID_VERTEX