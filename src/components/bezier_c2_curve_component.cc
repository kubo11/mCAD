#include "bezier_c2_curve_component.hh"

#include "../events/events.hh"
#include "selectible_component.hh"

unsigned int BezierC2CurveComponent::s_new_id = 1;

BezierC2CurveComponent::BezierC2CurveComponent(const mge::EntityVector& points, mge::Entity& polygon)
    : m_base(BezierCurveBase::BSpline), m_polygon(polygon) {
  m_control_points = points;
  create_bernstein_points();
  update_bernstein_points();
}

BezierC2CurveComponent::~BezierC2CurveComponent() {
  m_polygon.destroy();
  for (auto& point : m_bernstein_points) {
    point.get().destroy();
  }
}

void BezierC2CurveComponent::set_polygon_status(bool status) {
  m_polygon.patch<mge::RenderableComponent<GeometryVertex>>([&status](auto& renderable) {
    if (status) {
      renderable.enable();
    } else {
      renderable.disable();
    }
  });
}

std::vector<GeometryVertex> BezierC2CurveComponent::generate_geometry() const {
  std::vector<GeometryVertex> points;
  if (m_bernstein_points.empty()) {
    return points;
  }
  int size = m_bernstein_points.size() <= 4 ? 4 : (m_bernstein_points.size() + 1) / 3 * 4;
  points.reserve(size);
  for (int i = 0; i < m_bernstein_points.size(); ++i) {
    if (i > 0 && i % 3 == 0) {
      points.emplace_back(m_bernstein_points[i].get().get_component<mge::TransformComponent>().get_position());
    }
    points.emplace_back(m_bernstein_points[i].get().get_component<mge::TransformComponent>().get_position());
  }
  for (int i = points.size(); i < size; ++i) {
    points.emplace_back(glm::vec3{std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                                  std::numeric_limits<float>::quiet_NaN()});
  }

  return points;
}

std::vector<GeometryVertex> BezierC2CurveComponent::generate_polygon_geometry() const {
  std::vector<GeometryVertex> points(m_control_points.size());
  for (int i = 0; i < m_control_points.size(); ++i) {
    points[i] = {m_control_points[i].get().get_component<mge::TransformComponent>().get_position()};
  }
  return points;
}

void BezierC2CurveComponent::add_point(mge::Entity& point) {
  if (m_base == BezierCurveBase::BSpline) {
    m_control_points.push_back(point);
    if (m_control_points.size() < 4) return;
    unsigned int new_point_count = 3;
    if (m_control_points.size() - new_point_count < 4) new_point_count++;

    for (int i = 0; i < new_point_count; ++i) {
      AddPointEvent event;
      SendEvent(event);
      auto& point = event.point.value().get();
      point.remove_component<mge::TagComponent>();
      m_bernstein_points.push_back(point);
    }

    update_bernstein_points();
  } else {
    // TODO
  }
}

void BezierC2CurveComponent::remove_point(mge::Entity& point) {
  if (m_base == BezierCurveBase::BSpline) {
    unsigned int idx = std::find(m_control_points.begin(), m_control_points.end(), point) - m_control_points.begin();

    unsigned int to_delete = 0;
    if (m_control_points.size() == 4) {
      to_delete = 4;
    } else if (m_control_points.size() > 4) {
      to_delete = 3;
    }
    for (int i = to_delete - 1; i >= 0; --i) {
      m_bernstein_points.erase(m_bernstein_points.begin() + i);
    }

    m_control_points.erase(m_control_points.begin() + idx);
  } else {
    // TODO
  }
}

bool BezierC2CurveComponent::get_polygon_status() const {
  return m_polygon.get_component<mge::RenderableComponent<GeometryVertex>>().is_enabled();
}

BezierCurveBase BezierC2CurveComponent::get_base() const { return m_base; }

void BezierC2CurveComponent::set_base(BezierCurveBase base) {
  if (m_base == base) return;
  m_base = base;
  if (m_base == BezierCurveBase::Bernstein) {
    for (auto& point : m_bernstein_points) {
      point.get().patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
          [](auto& renderable) { renderable.enable(); });
    }
  } else {
    for (auto& point : m_bernstein_points) {
      point.get().patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
          [](auto& renderable) { renderable.disable(); });
    }
  }
}

void BezierC2CurveComponent::create_bernstein_points() {
  unsigned int segments = static_cast<int>(m_control_points.size()) - 3;
  if (segments > 0) {
    for (int i = 0; i < 3 * segments + 1; ++i) {
      AddPointEvent event;
      SendEvent(event);
      auto& point = event.point.value().get();
      point.remove_component<mge::TagComponent>();
      m_bernstein_points.push_back(point);
    }
  }
}

void BezierC2CurveComponent::update_control_points(mge::Entity& bernstein_point) {
  unsigned int idx =
      std::find(m_bernstein_points.begin(), m_bernstein_points.end(), bernstein_point) - m_bernstein_points.begin();
  if (idx == 0) {
    m_control_points[0].get().patch<mge::TransformComponent>([&bernstein_points = m_bernstein_points,
                                                              &control_points = m_control_points](auto& transform) {
      transform.set_position(4.0f * bernstein_points[0].get().get_component<mge::TransformComponent>().get_position() -
                             control_points[1].get().get_component<mge::TransformComponent>().get_position() -
                             2.0f * bernstein_points[1].get().get_component<mge::TransformComponent>().get_position());
    });
  } else if (idx == m_bernstein_points.size() - 1) {
    unsigned int control_point_count = m_control_points.size();
    unsigned int bernstein_point_count = m_bernstein_points.size();
    m_control_points[control_point_count - 1].get().patch<mge::TransformComponent>(
        [&bernstein_points = m_bernstein_points, &control_points = m_control_points, control_point_count,
         bernstein_point_count](auto& transform) {
          transform.set_position(
              4.0f * bernstein_points[bernstein_point_count - 1]
                         .get()
                         .get_component<mge::TransformComponent>()
                         .get_position() -
              control_points[control_point_count - 2].get().get_component<mge::TransformComponent>().get_position() -
              2.0f * bernstein_points[bernstein_point_count - 2]
                         .get()
                         .get_component<mge::TransformComponent>()
                         .get_position());
        });
  } else {
    unsigned int segment = idx / 3;
    int remainder = idx % 3;
    if (remainder == 0) {
      m_control_points[segment + 1].get().patch<mge::TransformComponent>(
          [&bernstein_points = m_bernstein_points, &control_points = m_control_points, idx, segment](auto& transform) {
            transform.set_position(
                3.0f / 2.0f * bernstein_points[idx].get().get_component<mge::TransformComponent>().get_position() -
                1.0f / 4.0f *
                    (control_points[segment].get().get_component<mge::TransformComponent>().get_position() +
                     control_points[segment + 2].get().get_component<mge::TransformComponent>().get_position()));
          });
    } else if (remainder == 1) {
      m_control_points[segment + 1].get().patch<mge::TransformComponent>([&bernstein_points = m_bernstein_points,
                                                                          &control_points = m_control_points, idx,
                                                                          segment](auto& transform) {
        transform.set_position(
            3.0f / 2.0f * bernstein_points[idx].get().get_component<mge::TransformComponent>().get_position() -
            1.0f / 2.0f * control_points[segment + 2].get().get_component<mge::TransformComponent>().get_position());
      });
    } else {
      m_control_points[segment + 2].get().patch<mge::TransformComponent>([&bernstein_points = m_bernstein_points,
                                                                          &control_points = m_control_points, idx,
                                                                          segment](auto& transform) {
        transform.set_position(
            3.0f / 2.0f * bernstein_points[idx].get().get_component<mge::TransformComponent>().get_position() -
            1.0f / 2.0f * control_points[segment + 1].get().get_component<mge::TransformComponent>().get_position());
      });
    }
  }
}

void BezierC2CurveComponent::update_bernstein_points() {
  if (m_bernstein_points.size() == 0) return;

  unsigned int segments = m_control_points.size() - 3;
  std::vector<glm::vec3> e(segments + 1);
  std::vector<glm::vec3> f(segments);
  std::vector<glm::vec3> g(segments);
  glm::vec3 fLast =
      (m_control_points[m_control_points.size() - 2].get().get_component<mge::TransformComponent>().get_position() +
       m_control_points[m_control_points.size() - 1].get().get_component<mge::TransformComponent>().get_position()) /
      2.0f;
  glm::vec3 gFirst = (m_control_points[0].get().get_component<mge::TransformComponent>().get_position() +
                      m_control_points[1].get().get_component<mge::TransformComponent>().get_position()) /
                     2.0f;

  for (int i = 0; i < segments; ++i) {
    f[i] = 2.0f / 3.0f * m_control_points[i + 1].get().get_component<mge::TransformComponent>().get_position() +
           1.0f / 3.0f * m_control_points[i + 2].get().get_component<mge::TransformComponent>().get_position();
    g[i] = 1.0f / 3.0f * m_control_points[i + 1].get().get_component<mge::TransformComponent>().get_position() +
           2.0f / 3.0f * m_control_points[i + 2].get().get_component<mge::TransformComponent>().get_position();
  }
  e[0] = (gFirst + f[0]) / 2.0f;
  e[segments] = (g[segments - 1] + fLast) / 2.0f;
  for (std::size_t i = 1; i < segments; ++i) {
    e[i] = (g[i - 1] + f[i]) / 2.0f;
  }

  for (int i = 0; i < segments; ++i) {
    m_bernstein_points[3 * i].get().patch<mge::TransformComponent>(
        [&e, i](auto& transform) { transform.set_position(e[i]); });
    m_bernstein_points[3 * i + 1].get().patch<mge::TransformComponent>(
        [&f, i](auto& transform) { transform.set_position(f[i]); });
    m_bernstein_points[3 * i + 2].get().patch<mge::TransformComponent>(
        [&g, i](auto& transform) { transform.set_position(g[i]); });
  }
  m_bernstein_points[3 * segments].get().patch<mge::TransformComponent>(
      [&e, segments](auto& transform) { transform.set_position(e[segments]); });
}