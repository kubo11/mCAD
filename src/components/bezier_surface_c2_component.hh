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

 private:
  static unsigned int s_new_id;
  SurfacePointsVector m_bezier_points;

  void create_points(float size_u, float size_v);
};

#endif  // MCAD_GEOMETRY_BEZIER_SURFACE_C2_COMPONENT