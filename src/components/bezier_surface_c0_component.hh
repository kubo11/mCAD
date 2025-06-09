#ifndef MCAD_GEOMETRY_BEZIER_SURFACE_C0_COMPONENT
#define MCAD_GEOMETRY_BEZIER_SURFACE_C0_COMPONENT

#include "mge.hh"

#include "bezier_surface_component.hh"

struct BezierSurfaceC0Component : public BezierSurfaceComponent {
  BezierSurfaceC0Component(unsigned int patch_count_u, unsigned int patch_count_v, float size_u, float size_v,
                           BezierSurfaceWrapping wrapping, mge::Entity& self, mge::Entity& grid);
  BezierSurfaceC0Component(std::vector<mge::EntityVector>& points, unsigned int patch_count_u,
                           unsigned int patch_count_v, BezierSurfaceWrapping wrapping, unsigned int line_count,
                           mge::Entity& self, mge::Entity& grid);

  static std::string get_new_name() { return "BezierSurfaceC0 " + std::to_string(s_new_id++); }

  virtual void update_surface(mge::Entity& entity) override;
  virtual SurfacePatchesVector get_patches() const override;

  glm::vec3 get_uv_pos(glm::vec2 uv) const;
  std::pair<glm::vec3, glm::vec3> get_uv_grad(glm::vec2 uv) const;

 private:
  static unsigned int s_new_id;

  void create_points(float size_u, float size_v);
};

inline float bernstein3_coef(int i) {
  return (i == 0 || i == 3) ? 1 : 3;
}

inline float bernstein2(int i, float t) {
  if (i < 0 || i > 2) {
    return 0;
  }
  if (i == 1) {
    return 2 * t * (1 - t);
  }
  return pow(t, i) * pow(1 - t, 2 - i);
}

inline float bernstein3(int i, float t) {
  if (i < 0 || i > 3) {
    return 0;
  }
  return bernstein3_coef(i) * pow(t, i) * pow(1 - t, 3 - i);
}

inline glm::vec3 c0_pos(glm::vec2 uv, glm::vec3 (&patch)[16]) {
  glm::vec3 result(0.0f, 0.0f, 0.0f);
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      result += bernstein3(i, uv.x) * bernstein3(j, uv.y) * patch[j * 4 + i];
  return result;
}

inline glm::vec3 c0_grad_u(glm::vec2 uv, glm::vec3 (&patch)[16]) {
  glm::vec3 result(0.0f, 0.0f, 0.0f);
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      result += 3 * (bernstein2(i - 1, uv.x) - bernstein2(i, uv.x)) * bernstein3(j, uv.y) * patch[j * 4 + i];
  return result;
}

inline glm::vec3 c0_grad_v(glm::vec2 uv, glm::vec3 (&patch)[16]) {
  glm::vec3 result(0.0f, 0.0f, 0.0f);
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      result += bernstein3(i, uv.x) * 3.0f * (bernstein2(j - 1, uv.y) - bernstein2(j, uv.y)) * patch[j * 4 + i];
  return result;
}

#endif  // MCAD_GEOMETRY_BEZIER_SURFACE_C0_COMPONENT