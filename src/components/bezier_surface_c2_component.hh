#ifndef MCAD_GEOMETRY_BEZIER_SURFACE_C2_COMPONENT
#define MCAD_GEOMETRY_BEZIER_SURFACE_C2_COMPONENT

#include "mge.hh"

#include "bezier_surface_component.hh"

struct BezierSurfaceC2Component : public BezierSurfaceComponent {
  BezierSurfaceC2Component(unsigned int patch_count_u, unsigned int patch_count_v, float size_u, float size_v,
                           BezierSurfaceWrapping wrapping, mge::Entity& self, mge::Entity& grid);
  BezierSurfaceC2Component(std::vector<mge::EntityVector>& points, unsigned int patch_count_u,
                           unsigned int patch_count_v, BezierSurfaceWrapping wrapping, unsigned int line_count,
                           mge::Entity& self, mge::Entity& grid);

  static std::string get_new_name() { return "BezierSurfaceC2 " + std::to_string(s_new_id++); }

  virtual std::vector<GeometryVertex> generate_geometry() const override;

  virtual void update_surface(mge::Entity& entity) override;
  void create_bezier_points();
  virtual SurfacePatchesVector get_patches() const override;

  glm::vec3 get_uv_pos(glm::vec2 uv) const;
  std::pair<glm::vec3, glm::vec3> get_uv_grad(glm::vec2 uv) const;

 private:
  static unsigned int s_new_id;
  SurfacePointsVector m_bezier_points;

  void create_points(float size_u, float size_v);
};

inline float deBoor_coef(int i, float t) {
  float t2 = t * t;
  float t3 = t2 * t;
  switch(i) {
  case -1:
    return (-t3 + 3.0f * t2 - 3.0f * t + 1.0f) / 6.0f;
  case 0:
    return (3.0f * t3 - 6.0f * t2 + 4.0f) / 6.0f;
  case 1:
    return (-3.0f * t3 + 3.0f * t2 + 3.0f * t + 1.0f) / 6.0f;
  case 2:
  default:
    return (t3) / 6.0f;
  }
}

inline float deBoor_der(int i, float t) {
  float t2der = 2.0f * t;
  float t3der = 3.0f * t * t;
  switch(i) {
  case -1:
    return (-t3der + 3.0f * t2der - 3.0f) / 6.0f;
  case 0:
    return (3.0f * t3der - 6.0f * t2der) / 6.0f;
  case 1:
    return (-3.0f * t3der + 3.0f * t2der + 3.0f) / 6.0f;
  case 2:
  default:
    return (t3der) / 6.0f;
  }
}

inline glm::vec3 c2_pos(glm::vec2 uv, glm::vec3 (&patch)[16]) {
  glm::vec3 result(0.0f, 0.0f, 0.0f);
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      result += deBoor_coef(i - 1, uv.x) * deBoor_coef(j - 1, uv.y) * patch[j * 4 + i];
  return result;
}

inline glm::vec3 c2_grad_u(glm::vec2 uv, glm::vec3 (&patch)[16]) {
  glm::vec3 result(0.0f, 0.0f, 0.0f);
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      result += deBoor_der(i - 1, uv.x) * deBoor_coef(j - 1, uv.y) * patch[j * 4 + i];
  return result;
}

inline glm::vec3 c2_grad_v(glm::vec2 uv, glm::vec3 (&patch)[16]) {
  glm::vec3 result(0.0f, 0.0f, 0.0f);
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      result += deBoor_coef(i - 1, uv.x) * deBoor_der(j - 1, uv.y) * patch[j * 4 + i];
  return result;
}

#endif  // MCAD_GEOMETRY_BEZIER_SURFACE_C2_COMPONENT