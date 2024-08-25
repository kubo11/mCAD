#ifndef MCAD_GEOMETRY_BEZIER_CURVE_C2_INTERP_COMPONENT
#define MCAD_GEOMETRY_BEZIER_CURVE_C2_INTERP_COMPONENT

#include "mge.hh"

#include "../vertices/bezier_curve_c2_interp_vertex.hh"
#include "bezier_curve_component.hh"

struct BezierCurveC2InterpComponent : public BezierCurveComponent {
  BezierCurveC2InterpComponent(const mge::EntityVector& points, mge::Entity& self, mge::Entity& polygon);
  virtual ~BezierCurveC2InterpComponent() = default;

  static std::string get_new_name() { return "BezierCurveC2Interp " + std::to_string(s_new_id++); }

  std::vector<BezierCurveC2InterpVertex> generate_geometry() const;
  std::vector<GeometryVertex> generate_polygon_geometry() const;

  virtual void set_base(BezierCurveBase base) override;

  virtual void update_curve(mge::Entity& entity) override;

 private:
  static unsigned int s_new_id;
};

#endif  // MCAD_GEOMETRY_BEZIER_CURVE_C2_INTERP_COMPONENT