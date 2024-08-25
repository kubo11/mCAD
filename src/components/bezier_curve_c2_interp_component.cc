#include "bezier_curve_c2_interp_component.hh"

unsigned int BezierCurveC2InterpComponent::s_new_id = 1;

BezierCurveC2InterpComponent::BezierCurveC2InterpComponent(const mge::EntityVector& points, mge::Entity& self,
                                                           mge::Entity& polygon)
    : BezierCurveComponent(BezierCurveBase::BSpline, points, self, polygon) {
  for (auto& data : m_control_points) {
    data.first = data.second.get().register_on_update<mge::TransformComponent, BezierCurveC2InterpComponent>(
        &BezierCurveC2InterpComponent::update_curve, this);
  }
}

std::vector<BezierCurveC2InterpVertex> BezierCurveC2InterpComponent::generate_geometry() const {
  std::vector<BezierCurveC2InterpVertex> segments;
  if (m_control_points.size() >= 3) {
    std::size_t n = m_control_points.size() - 1;
    std::vector<float> dt(n);
    std::vector<glm::vec3> a(n);
    std::vector<glm::vec3> b(n);
    std::vector<glm::vec3> c(n);
    std::vector<glm::vec3> d(n);
    std::vector<float> alpha(n);
    std::vector<float> beta(n);
    std::vector<float> betap(n);
    std::vector<glm::vec3> R(n);
    std::vector<glm::vec3> Rp(n);

    for (std::size_t i = 0; i < n; ++i) {
      dt[i] = glm::length(m_control_points[i + 1].second.get().get_component<mge::TransformComponent>().get_position() -
                          m_control_points[i].second.get().get_component<mge::TransformComponent>().get_position());
      a[i] = m_control_points[i].second.get().get_component<mge::TransformComponent>().get_position();
    }

    for (std::size_t i = 2; i < n; ++i) {
      alpha[i] = dt[i - 1] / (dt[i - 1] + dt[i]);
    }

    for (std::size_t i = 1; i < n - 1; ++i) {
      beta[i] = dt[i] / (dt[i - 1] + dt[i]);
    }

    for (std::size_t i = 1; i < n; ++i) {
      R[i] = 3.0f *
             ((m_control_points[i + 1].second.get().get_component<mge::TransformComponent>().get_position() -
               m_control_points[i].second.get().get_component<mge::TransformComponent>().get_position()) /
                  dt[i] -
              (m_control_points[i].second.get().get_component<mge::TransformComponent>().get_position() -
               m_control_points[i - 1].second.get().get_component<mge::TransformComponent>().get_position()) /
                  dt[i - 1]) /
             (dt[i - 1] + dt[i]);
    }

    betap[1] = beta[1] / 2;
    for (std::size_t i = 2; i < n - 1; ++i) {
      betap[i] = beta[i] / (2 - alpha[i] * betap[i - 1]);
    }

    Rp[1] = R[1] / 2.0f;
    for (std::size_t i = 2; i < n; ++i) {
      Rp[i] = (R[i] - alpha[i] * Rp[i - 1]) / (2 - alpha[i] * betap[i - 1]);
    }

    c[n - 1] = Rp[n - 1];
    for (std::size_t i = n - 2; i >= 1; --i) {
      c[i] = Rp[i] - betap[i] * c[i + 1];
    }
    c[0] = {0, 0, 0};

    for (std::size_t i = 0; i < n - 1; ++i) {
      d[i] = 2.0f * (c[i + 1] - c[i]) / (6 * dt[i]);
    }
    d[n - 1] = -2.0f * c[n - 1] / (6 * dt[n - 1]);

    for (std::size_t i = 0; i < n - 1; ++i) {
      b[i] = (a[i + 1] - a[i]) / dt[i] - (c[i] + d[i] * dt[i]) * dt[i];
    }
    b[n - 1] = (m_control_points[n].second.get().get_component<mge::TransformComponent>().get_position() - a[n - 1]) /
                   dt[n - 1] -
               (c[n - 1] + d[n - 1] * dt[n - 1]) * dt[n - 1];

    for (std::size_t i = 0; i < n; ++i) {
      segments.push_back({a[i], b[i], c[i], d[i],
                          m_control_points[i + 1].second.get().get_component<mge::TransformComponent>().get_position(),
                          dt[i]});
    }
  }
  return segments;
}

std::vector<GeometryVertex> BezierCurveC2InterpComponent::generate_polygon_geometry() const {
  std::vector<GeometryVertex> points(m_control_points.size());
  for (int i = 0; i < m_control_points.size(); ++i) {
    points[i] = {m_control_points[i].second.get().get_component<mge::TransformComponent>().get_position()};
  }
  return points;
}

void BezierCurveC2InterpComponent::set_base(BezierCurveBase base) {}

void BezierCurveC2InterpComponent::update_curve(mge::Entity& entity) {
  update_renderables(generate_geometry(), generate_polygon_geometry());
}