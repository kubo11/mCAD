#ifndef MCAD_GEOMETRY_BEZIER_CURVE_C0_COMPONENT
#define MCAD_GEOMETRY_BEZIER_CURVE_C0_COMPONENT

#include "mge.hh"

#include "bezier_curve_component.hh"

struct BezierCurveC0Component : public BezierCurveComponent {
  BezierCurveC0Component(const mge::EntityVector& points, mge::Entity& self, mge::Entity& polygon);
  virtual ~BezierCurveC0Component() = default;

  static std::string get_new_name() { return "BezierCurveC0 " + std::to_string(s_new_id++); }

  std::vector<GeometryVertex> generate_geometry() const;
  std::vector<GeometryVertex> generate_polygon_geometry() const;

  virtual void set_base(BezierCurveBase base) override;

  virtual void update_curve(mge::Entity& entity) override;

 private:
  static unsigned int s_new_id;
};

#endif  // MCAD_GEOMETRY_BEZIER_CURVE_C0_COMPONENT