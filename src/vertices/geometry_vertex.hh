#ifndef MCAD_VERTICES_GEOMETRY_VERTEX
#define MCAD_VERTICES_GEOMETRY_VERTEX

#include "mge.hh"

struct GeometryVertex {
  glm::vec3 position = {0.0f, 0.0f, 0.0f};

  GeometryVertex() : position({0.0f, 0.0f, 0.0f}) {}
  GeometryVertex(const glm::vec3& position) : position(position) {}

  inline static std::vector<mge::VertexAttribute> get_vertex_attributes() {
    return {{3, GL_FLOAT}};
  }
};

#endif  // MCAD_VERTICES_GEOMETRY_VERTEX