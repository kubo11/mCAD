#ifndef MCAD_VERTICES_BEZIER_CURVE_C2_INTERP_VERTEX
#define MCAD_VERTICES_BEZIER_CURVE_C2_INTERP_VERTEX

#include "mge.hh"

struct BezierCurveC2InterpVertex {
  glm::vec3 a;
  glm::vec3 b;
  glm::vec3 c;
  glm::vec3 d;
  glm::vec3 next_point;
  float dt;

  BezierCurveC2InterpVertex() : a{}, b{}, c{}, d{}, next_point{}, dt{} {}
  BezierCurveC2InterpVertex(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d,
                            const glm::vec3& next_point, float dt)
      : a(a), b(b), c(c), d(d), next_point(next_point), dt(dt) {}

  inline static std::vector<mge::VertexAttribute> get_vertex_attributes() {
    return {{3, GL_FLOAT}, {3, GL_FLOAT}, {3, GL_FLOAT}, {3, GL_FLOAT}, {3, GL_FLOAT}, {1, GL_FLOAT}};
  }
};

#endif  // MCAD_VERTICES_BEZIER_CURVE_C2_INTERP_VERTEX