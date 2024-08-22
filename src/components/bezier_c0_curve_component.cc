#include "bezier_c0_curve_component.hh"
#include "selectible_component.hh"

unsigned int BezierC0CurveComponent::s_new_id = 1;

BezierC0CurveComponent::BezierC0CurveComponent(const mge::EntityVector& points, mge::Entity& polygon)
    : m_control_points(points), m_polygon(polygon) {}

BezierC0CurveComponent::~BezierC0CurveComponent() { m_polygon.destroy(); }

void BezierC0CurveComponent::set_polygon_status(bool status) {
  m_polygon.patch<mge::RenderableComponent<GeometryVertex>>([&status](auto& renderable) {
    if (status) {
      renderable.enable();
    } else {
      renderable.disable();
    }
  });
}

std::vector<GeometryVertex> BezierC0CurveComponent::generate_geometry() const {
  std::vector<GeometryVertex> points;
  if (m_control_points.empty()) {
    return points;
  }
  int size = m_control_points.size() <= 4 ? 4 : (m_control_points.size() + 1) / 3 * 4;
  points.reserve(size);
  for (int i = 0; i < m_control_points.size(); ++i) {
    if (i > 0 && i % 3 == 0) {
      points.emplace_back(m_control_points[i].get().get_component<mge::TransformComponent>().get_position());
    }
    points.emplace_back(m_control_points[i].get().get_component<mge::TransformComponent>().get_position());
  }
  for (int i = points.size(); i < size; ++i) {
    points.emplace_back(glm::vec3{std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                                  std::numeric_limits<float>::quiet_NaN()});
  }

  return points;
}

std::vector<GeometryVertex> BezierC0CurveComponent::generate_polygon_geometry() const {
  std::vector<GeometryVertex> points(m_control_points.size());
  for (int i = 0; i < m_control_points.size(); ++i) {
    points[i] = {m_control_points[i].get().get_component<mge::TransformComponent>().get_position()};
  }
  return points;
}

void BezierC0CurveComponent::add_point(mge::Entity& control_point) { m_control_points.emplace_back(control_point); }

void BezierC0CurveComponent::remove_point(mge::Entity& control_point) {
  m_control_points.erase(std::remove(m_control_points.begin(), m_control_points.end(), control_point),
                         m_control_points.end());
}

bool BezierC0CurveComponent::get_polygon_status() const {
  return m_polygon.get_component<mge::RenderableComponent<GeometryVertex>>().is_enabled();
}
