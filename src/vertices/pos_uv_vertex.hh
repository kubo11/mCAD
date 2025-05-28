#ifndef MCAD_VERTICES_POS_UV_VERTEX
#define MCAD_VERTICES_POS_UV_VERTEX

#include "mge.hh"

struct PosUvVertex {
  glm::vec3 position = {0.0f, 0.0f, 0.0f};
  glm::vec2 uv = {0.0f, 0.0f};

  PosUvVertex() : position({0.0f, 0.0f, 0.0f}), uv({0.0f, 0.0f}) {}
  PosUvVertex(const glm::vec3& position, const glm::vec2& uv) : position(position), uv(uv) {}

  inline static std::vector<mge::VertexAttribute> get_vertex_attributes() { return {{3, GL_FLOAT}, {2, GL_FLOAT}}; }
};

#endif  // MCAD_VERTICES_POS_UV_VERTEX