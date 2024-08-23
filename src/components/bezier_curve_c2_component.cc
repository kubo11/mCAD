#include "bezier_curve_c2_component.hh"

#include "../events/events.hh"

unsigned int BezierCurveC2Component::s_new_id = 1;

BezierCurveC2Component::BezierCurveC2Component(const mge::EntityVector& points, mge::Entity& self, mge::Entity& polygon)
    : BezierCurveComponent(points, self, polygon), m_base(BezierCurveBase::BSpline) {
  create_bernstein_points();
  m_block_updates = true;
  update_bernstein_points();
  m_block_updates = false;
}

BezierCurveC2Component::~BezierCurveC2Component() {
  for (auto& point : m_bernstein_points) {
    point.second.get().destroy();
  }
}

std::vector<GeometryVertex> BezierCurveC2Component::generate_geometry() const {
  std::vector<GeometryVertex> points;
  if (m_bernstein_points.empty()) {
    return points;
  }
  int size = m_bernstein_points.size() <= 4 ? 4 : (m_bernstein_points.size() + 1) / 3 * 4;
  points.reserve(size);
  for (int i = 0; i < m_bernstein_points.size(); ++i) {
    if (i > 0 && i % 3 == 0) {
      points.emplace_back(m_bernstein_points[i].second.get().get_component<mge::TransformComponent>().get_position());
    }
    points.emplace_back(m_bernstein_points[i].second.get().get_component<mge::TransformComponent>().get_position());
  }
  for (int i = points.size(); i < size; ++i) {
    points.emplace_back(glm::vec3{std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                                  std::numeric_limits<float>::quiet_NaN()});
  }

  return points;
}

std::vector<GeometryVertex> BezierCurveC2Component::generate_polygon_geometry() const {
  std::vector<GeometryVertex> points(m_control_points.size());
  for (int i = 0; i < m_control_points.size(); ++i) {
    points[i] = {m_control_points[i].second.get().get_component<mge::TransformComponent>().get_position()};
  }
  return points;
}

void BezierCurveC2Component::add_point(mge::Entity& point) {
  if (m_base == BezierCurveBase::BSpline) {
    BezierCurveComponent::add_point(point);
    if (m_control_points.size() < 4) return;
    unsigned int new_point_count = 3;
    if (m_control_points.size() - new_point_count < 4) new_point_count++;

    for (int i = 0; i < new_point_count; ++i) {
      CreateBernsteinPointEvent event;
      SendEvent(event);
      auto& point = event.bernstein_point.value().get();
      m_bernstein_points.push_back(
          {point.register_on_update<mge::TransformComponent>(&BezierCurveC2Component::update_by_bernstein_point, this),
           point});
    }

    update_bernstein_points();
  } else {
    // TODO
  }
}

void BezierCurveC2Component::remove_point(mge::Entity& point) {
  if (m_base == BezierCurveBase::BSpline) {
    unsigned int to_delete = 0;
    if (m_control_points.size() == 4) {
      to_delete = 4;
    } else if (m_control_points.size() > 4) {
      to_delete = 3;
    }
    for (int i = to_delete - 1; i >= 0; --i) {
      m_bernstein_points.erase(m_bernstein_points.begin() + i);
    }

    BezierCurveComponent::remove_point(point);
  } else {
    // TODO
  }
}

BezierCurveBase BezierCurveC2Component::get_base() const { return m_base; }

void BezierCurveC2Component::set_base(BezierCurveBase base) {
  if (m_base == base) return;
  m_base = base;
  if (m_base == BezierCurveBase::Bernstein) {
    for (auto& point : m_bernstein_points) {
      point.second.get().patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
          [](auto& renderable) { renderable.enable(); });
    }
  } else {
    for (auto& point : m_bernstein_points) {
      point.second.get().patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
          [](auto& renderable) { renderable.disable(); });
    }
  }
}

void BezierCurveC2Component::create_bernstein_points() {
  unsigned int segments = static_cast<int>(m_control_points.size()) - 3;
  if (segments > 0) {
    for (int i = 0; i < 3 * segments + 1; ++i) {
      CreateBernsteinPointEvent event;
      SendEvent(event);
      auto& point = event.bernstein_point.value().get();
      m_bernstein_points.push_back(
          {point.register_on_update<mge::TransformComponent>(&BezierCurveC2Component::update_by_bernstein_point, this),
           point});
    }
  }
}

void BezierCurveC2Component::update_control_points(mge::Entity& bernstein_point) {
  unsigned int idx = std::find_if(m_bernstein_points.begin(), m_bernstein_points.end(),
                                  [&bernstein_point](auto& data) { return data.second.get() == bernstein_point; }) -
                     m_bernstein_points.begin();
  if (idx == 0) {
    m_control_points[0].second.get().patch<mge::TransformComponent>(
        [&bernstein_points = m_bernstein_points, &control_points = m_control_points](auto& transform) {
          transform.set_position(
              4.0f * bernstein_points[0].second.get().get_component<mge::TransformComponent>().get_position() -
              control_points[1].second.get().get_component<mge::TransformComponent>().get_position() -
              2.0f * bernstein_points[1].second.get().get_component<mge::TransformComponent>().get_position());
        });
  } else if (idx == m_bernstein_points.size() - 1) {
    unsigned int control_point_count = m_control_points.size();
    unsigned int bernstein_point_count = m_bernstein_points.size();
    m_control_points[control_point_count - 1].second.get().patch<mge::TransformComponent>([&bernstein_points =
                                                                                               m_bernstein_points,
                                                                                           &control_points =
                                                                                               m_control_points,
                                                                                           control_point_count,
                                                                                           bernstein_point_count](
                                                                                              auto& transform) {
      transform.set_position(
          4.0f * bernstein_points[bernstein_point_count - 1]
                     .second.get()
                     .get_component<mge::TransformComponent>()
                     .get_position() -
          control_points[control_point_count - 2].second.get().get_component<mge::TransformComponent>().get_position() -
          2.0f * bernstein_points[bernstein_point_count - 2]
                     .second.get()
                     .get_component<mge::TransformComponent>()
                     .get_position());
    });
  } else {
    unsigned int segment = idx / 3;
    int remainder = idx % 3;
    if (remainder == 0) {
      m_control_points[segment + 1].second.get().patch<mge::TransformComponent>([&bernstein_points = m_bernstein_points,
                                                                                 &control_points = m_control_points,
                                                                                 idx, segment](auto& transform) {
        transform.set_position(
            3.0f / 2.0f * bernstein_points[idx].second.get().get_component<mge::TransformComponent>().get_position() -
            1.0f / 4.0f *
                (control_points[segment].second.get().get_component<mge::TransformComponent>().get_position() +
                 control_points[segment + 2].second.get().get_component<mge::TransformComponent>().get_position()));
      });
    } else if (remainder == 1) {
      m_control_points[segment + 1].second.get().patch<mge::TransformComponent>([&bernstein_points = m_bernstein_points,
                                                                                 &control_points = m_control_points,
                                                                                 idx, segment](auto& transform) {
        transform.set_position(
            3.0f / 2.0f * bernstein_points[idx].second.get().get_component<mge::TransformComponent>().get_position() -
            1.0f / 2.0f *
                control_points[segment + 2].second.get().get_component<mge::TransformComponent>().get_position());
      });
    } else {
      m_control_points[segment + 2].second.get().patch<mge::TransformComponent>([&bernstein_points = m_bernstein_points,
                                                                                 &control_points = m_control_points,
                                                                                 idx, segment](auto& transform) {
        transform.set_position(
            3.0f / 2.0f * bernstein_points[idx].second.get().get_component<mge::TransformComponent>().get_position() -
            1.0f / 2.0f *
                control_points[segment + 1].second.get().get_component<mge::TransformComponent>().get_position());
      });
    }
  }
}

void BezierCurveC2Component::update_bernstein_points() {
  if (m_bernstein_points.size() == 0) return;

  unsigned int segments = m_control_points.size() - 3;
  std::vector<glm::vec3> e(segments + 1);
  std::vector<glm::vec3> f(segments);
  std::vector<glm::vec3> g(segments);
  glm::vec3 fLast = (m_control_points[m_control_points.size() - 2]
                         .second.get()
                         .get_component<mge::TransformComponent>()
                         .get_position() +
                     m_control_points[m_control_points.size() - 1]
                         .second.get()
                         .get_component<mge::TransformComponent>()
                         .get_position()) /
                    2.0f;
  glm::vec3 gFirst = (m_control_points[0].second.get().get_component<mge::TransformComponent>().get_position() +
                      m_control_points[1].second.get().get_component<mge::TransformComponent>().get_position()) /
                     2.0f;

  for (int i = 0; i < segments; ++i) {
    f[i] = 2.0f / 3.0f * m_control_points[i + 1].second.get().get_component<mge::TransformComponent>().get_position() +
           1.0f / 3.0f * m_control_points[i + 2].second.get().get_component<mge::TransformComponent>().get_position();
    g[i] = 1.0f / 3.0f * m_control_points[i + 1].second.get().get_component<mge::TransformComponent>().get_position() +
           2.0f / 3.0f * m_control_points[i + 2].second.get().get_component<mge::TransformComponent>().get_position();
  }
  e[0] = (gFirst + f[0]) / 2.0f;
  e[segments] = (g[segments - 1] + fLast) / 2.0f;
  for (std::size_t i = 1; i < segments; ++i) {
    e[i] = (g[i - 1] + f[i]) / 2.0f;
  }

  for (int i = 0; i < segments; ++i) {
    m_bernstein_points[3 * i].second.get().patch<mge::TransformComponent>(
        [&e, i](auto& transform) { transform.set_position(e[i]); });
    m_bernstein_points[3 * i + 1].second.get().patch<mge::TransformComponent>(
        [&f, i](auto& transform) { transform.set_position(f[i]); });
    m_bernstein_points[3 * i + 2].second.get().patch<mge::TransformComponent>(
        [&g, i](auto& transform) { transform.set_position(g[i]); });
  }
  m_bernstein_points[3 * segments].second.get().patch<mge::TransformComponent>(
      [&e, segments](auto& transform) { transform.set_position(e[segments]); });
}

void BezierCurveC2Component::update_by_control_point(mge::Entity& entity) {
  if (m_block_updates) return;
  m_block_updates = true;
  update_bernstein_points();
  m_block_updates = false;
  BezierCurveComponent::update_renderable(entity);
}

void BezierCurveC2Component::update_by_bernstein_point(mge::Entity& entity) {
  if (m_block_updates) return;
  m_block_updates = true;
  update_control_points(entity);
  m_block_updates = false;
  BezierCurveComponent::update_renderable(entity);
}