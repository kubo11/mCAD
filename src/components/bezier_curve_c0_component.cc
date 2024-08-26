#include "bezier_curve_c0_component.hh"

unsigned int BezierCurveC0Component::s_new_id = 1;

BezierCurveC0Component::BezierCurveC0Component(const mge::EntityVector& points, mge::Entity& self, mge::Entity& polygon)
    : BezierCurveComponent(BezierCurveBase::BSpline, points, self, polygon) {
  for (auto& data : m_control_points) {
    data.first = data.second.get().register_on_update<mge::TransformComponent, BezierCurveC0Component>(
        &BezierCurveC0Component::update_curve, this);
  }
}

std::vector<GeometryVertex> BezierCurveC0Component::generate_geometry() const {
  std::vector<GeometryVertex> points;
  if (m_control_points.empty()) {
    return points;
  }
  int size = m_control_points.size() <= 4 ? 4 : (m_control_points.size() + 1) / 3 * 4;
  points.reserve(size);
  for (int i = 0; i < m_control_points.size(); ++i) {
    if (i > 0 && i % 3 == 0) {
      points.emplace_back(m_control_points[i].second.get().get_component<mge::TransformComponent>().get_position());
    }
    points.emplace_back(m_control_points[i].second.get().get_component<mge::TransformComponent>().get_position());
  }
  for (int i = points.size(); i < size; ++i) {
    points.emplace_back(glm::vec3{std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                                  std::numeric_limits<float>::quiet_NaN()});
  }

  return points;
}

std::vector<GeometryVertex> BezierCurveC0Component::generate_polygon_geometry() const {
  std::vector<GeometryVertex> points(m_control_points.size());
  for (int i = 0; i < m_control_points.size(); ++i) {
    points[i] = {m_control_points[i].second.get().get_component<mge::TransformComponent>().get_position()};
  }
  return points;
}

void BezierCurveC0Component::set_base(BezierCurveBase base) {}

void BezierCurveC0Component::update_curve(mge::Entity& entity) {
  if (m_blocked_updates_count > 0) {
    --m_blocked_updates_count;
    return;
  }
  update_renderables(generate_geometry(), generate_polygon_geometry());
  update_position();
}