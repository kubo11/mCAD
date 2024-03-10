#ifndef CMAD_GEOMETRY_GEOMETRY_VERTEX_HH
#define CMAD_GEOMETRY_GEOMETRY_VERTEX_HH

#include "mge.hh"

struct GeometryVertex {
  glm::vec3 position = {0.0f, 0.0f, 0.0f};

  GeometryVertex() : position({0.0f, 0.0f, 0.0f}) {}
  GeometryVertex(const glm::vec3& position) : position(position) {}

  inline static std::vector<mge::VertexAttribute> get_vertex_attributes() {
    return {{3, GL_FLOAT}};
  }
};

#endif  // CMAD_GEOMETRY_GEOMETRY_VERTEX_HH