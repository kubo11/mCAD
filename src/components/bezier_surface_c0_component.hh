#ifndef MCAD_GEOMETRY_BEZIER_SURFACE_C0_COMPONENT
#define MCAD_GEOMETRY_BEZIER_SURFACE_C0_COMPONENT

#include "mge.hh"

#include "bezier_surface_component.hh"

struct BezierSurfaceC0Component : public BezierSurfaceComponent {
  BezierSurfaceC0Component(unsigned int patch_count_u, unsigned int patch_count_v, float size_u, float size_v,
                           BezierSurfaceWrapping wrapping, mge::Entity& self, mge::Entity& grid);

  static std::string get_new_name() { return "BezierSurfaceC0 " + std::to_string(s_new_id++); }

  virtual void update_surface(mge::Entity& entity) override;

 private:
  static unsigned int s_new_id;

  void create_points(float size_u, float size_v);
};

#endif  // MCAD_GEOMETRY_BEZIER_SURFACE_C0_COMPONENT