#ifndef MCAD_GEOMETRY_BEZIER_CURVE_COMPONENT
#define MCAD_GEOMETRY_BEZIER_CURVE_COMPONENT

#include "mge.hh"

#include "../vertices/geometry_vertex.hh"

enum class BezierCurveBase { BSpline, Bernstein };

struct BezierCurveComponent {
  BezierCurveComponent(BezierCurveBase base, const mge::EntityVector& points, mge::Entity& self, mge::Entity& polygon);
  virtual ~BezierCurveComponent();

  bool get_polygon_status() const;
  void set_polygon_status(bool status);
  mge::Entity& get_polygon() { return m_polygon; }

  const std::vector<std::pair<unsigned int, std::reference_wrapper<mge::Entity>>>& get_control_points() const {
    return m_control_points;
  }

  virtual void add_point(mge::Entity& point);
  virtual void remove_point(mge::Entity& point);

  virtual void set_base(BezierCurveBase base);
  BezierCurveBase get_base() const;

  virtual void update_curve(mge::Entity& entity) = 0;
  virtual void update_curve_by_self(mge::Entity& entity);
  void update_position();

 protected:
  std::vector<std::pair<unsigned int, std::reference_wrapper<mge::Entity>>> m_control_points;
  mge::Entity& m_polygon;
  mge::Entity& m_self;
  BezierCurveBase m_base;
  bool m_block_updates;
  unsigned int m_blocked_updates_count;

  template <class T, class N>
  void update_renderables(std::vector<T> curve_vertices, std::vector<N> polygon_vertices) {
    if (m_block_updates) return;
    m_self.patch<mge::RenderableComponent<T>>([this, &curve_vertices](auto& renderable) {
      auto& vertex_buffer = renderable.get_vertex_array().get_vertex_buffer();
      vertex_buffer.bind();
      vertex_buffer.copy(curve_vertices);
      vertex_buffer.unbind();
    });
    m_polygon.patch<mge::RenderableComponent<N>>([this, &polygon_vertices](auto& renderable) {
      auto& vertex_buffer = renderable.get_vertex_array().get_vertex_buffer();
      vertex_buffer.bind();
      vertex_buffer.copy(polygon_vertices);
      vertex_buffer.unbind();
    });
  }
};

#endif  // MCAD_GEOMETRY_BEZIER_C0_CURVE_COMPONENT