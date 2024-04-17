#ifndef MCAD_GEOMETRY_BEZIER_COMPONENT_HH
#define MCAD_GEOMETRY_BEZIER_COMPONENT_HH

#include "mge.hh"

#include "geometry_vertex.hh"

struct BezierComponent {
  BezierComponent() {}
  BezierComponent(BezierComponent&&) = default;
  inline BezierComponent& operator=(BezierComponent&& other) { return *this; }
  static std::string get_new_name() {
    return "Bezier " + std::to_string(s_new_id++);
  }

  void on_construct(mge::Entity& entity);
  void on_update(mge::Entity& entity);

  void update();

 private:
  static unsigned int s_new_id;
  static fs::path s_draw_shader_path;
  static fs::path s_compute_shader_path;

  std::vector<GeometryVertex> generate_geometry();
};

#endif  // MCAD_GEOMETRY_BEZIER_COMPONENT_HH