#include "bezier_component_c2.hh"
#include "selectible_component.hh"

#include "point_component.hh"
#include "selectible_component.hh"

unsigned int BezierComponentC2::s_new_id = 1;
fs::path BezierComponentC2::s_poly_shader_path =
    fs::current_path() / "src" / "shaders" / "bezier_poly";
fs::path BezierComponentC2::s_bezier_shader_path =
    fs::current_path() / "src" / "shaders" / "bezier";

BezierComponentC2::BezierComponentC2(const mge::EntityVector& control_points,
                                     mge::Entity& polygon, Base base,
                                     mge::Entity& entity, mge::Scene& scene)
    : m_control_points(),
      m_polygon(polygon),
      m_base(base),
      m_entity(entity),
      m_scene(scene) {
  m_control_points.insert({base, control_points});
  m_control_points.insert({static_cast<Base>(1 - static_cast<int>(base)), {}});
}

void BezierComponentC2::on_construct(mge::Entity& entity) {
  auto vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(this->generate_geometry()),
      GeometryVertex::get_vertex_attributes());
  entity.add_or_replace_component<mge::RenderableComponent<GeometryVertex>>(
      mge::ShaderSystem::acquire(s_bezier_shader_path),
      std::move(vertex_array));
  entity.patch<mge::RenderableComponent<GeometryVertex>>([](auto& renderable) {
    renderable.set_render_mode(mge::RenderMode::PATCHES);
  });
  vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      generate_polygon_geometry(), GeometryVertex::get_vertex_attributes());
  m_polygon
      .add_or_replace_component<mge::RenderableComponent<GeometryVertex>>(
          mge::ShaderSystem::acquire(s_poly_shader_path),
          std::move(vertex_array))
      .disable();
  entity.add_owned_child(m_polygon);
  for (auto& control_point : m_control_points.at(m_base)) {
    entity.add_child(control_point.get());
  }
  if (m_base == Base::DeBoor) {
    for (int i = 0; i < m_control_points.at(Base::Berenstein).size() - 3; ++i) {
      auto& control_point =
          m_scene.create_entity(PointComponent::get_new_name());
      control_point.add_component<PointComponent>();
      control_point.add_component<SelectibleComponent>();
      entity.add_owned_child(control_point);
      m_control_points.at(Base::DeBoor).push_back(control_point);
    }
  } else {
    for (int i = 0; i < m_control_points.at(Base::DeBoor).size() + 3; ++i) {
      auto& control_point =
          m_scene.create_entity(PointComponent::get_new_name());
      control_point.add_component<PointComponent>();
      control_point.add_component<SelectibleComponent>();
      entity.add_owned_child(control_point);
      m_control_points.at(Base::Berenstein).push_back(control_point);
    }
  }
}

void BezierComponentC2::on_update(mge::Entity& entity) {
  if (m_base == Base::Berenstein) {
    for (int i = 0; i < m_control_points.at(Base::DeBoor).size(); ++i) {
      m_control_points.at(Base::DeBoor)[i].get().patch<mge::TransformComponent>(
          [this, i](auto& transform) {
            transform.set_position(
                (m_control_points.at(Base::DeBoor)[i]
                     .get()
                     .get_component<mge::TransformComponent>()
                     .get_position() +
                 m_control_points.at(Base::DeBoor)[i + 1]
                     .get()
                     .get_component<mge::TransformComponent>()
                     .get_position() +
                 m_control_points.at(Base::DeBoor)[i + 2]
                     .get()
                     .get_component<mge::TransformComponent>()
                     .get_position() +
                 m_control_points.at(Base::DeBoor)[i + 3]
                     .get()
                     .get_component<mge::TransformComponent>()
                     .get_position()) /
                4.0f);
          });
    }
  } else {
    std::size_t segments = m_control_points.at(Base::DeBoor).size() - 3;
    std::vector<glm::vec3> e(segments + 1);
    std::vector<glm::vec3> f(segments);
    std::vector<glm::vec3> g(segments);
    glm::vec3 fLast =
        (m_control_points
             .at(Base::DeBoor)[m_control_points.at(Base::DeBoor).size() - 2]
             .get()
             .get_component<mge::TransformComponent>()
             .get_position() +
         m_control_points
             .at(Base::DeBoor)[m_control_points.at(Base::DeBoor).size() - 1]
             .get()
             .get_component<mge::TransformComponent>()
             .get_position()) /
        2.0f;
    glm::vec3 gFirst = (m_control_points.at(Base::DeBoor)[0]
                            .get()
                            .get_component<mge::TransformComponent>()
                            .get_position() +
                        m_control_points.at(Base::DeBoor)[1]
                            .get()
                            .get_component<mge::TransformComponent>()
                            .get_position()) /
                       2.0f;

    for (std::size_t i = 0; i < segments; ++i) {
      f[i] = 2.0f / 3.0f *
                 m_control_points.at(Base::DeBoor)[i + 1]
                     .get()
                     .get_component<mge::TransformComponent>()
                     .get_position() +
             1.0f / 3.0f *
                 m_control_points.at(Base::DeBoor)[i + 2]
                     .get()
                     .get_component<mge::TransformComponent>()
                     .get_position();
      g[i] = 1.0f / 3.0f *
                 m_control_points.at(Base::DeBoor)[i + 1]
                     .get()
                     .get_component<mge::TransformComponent>()
                     .get_position() +
             2.0f / 3.0f *
                 m_control_points.at(Base::DeBoor)[i + 2]
                     .get()
                     .get_component<mge::TransformComponent>()
                     .get_position();
    }
    e[0] = (gFirst + f[0]) / 2.0f;
    e[segments] = (g[segments - 1] + fLast) / 2.0f;
    for (std::size_t i = 1; i < segments; ++i) {
      e[i] = (g[i - 1] + f[i]) / 2.0f;
    }

    for (std::size_t i = 0; i < segments; ++i) {
      m_control_points.at(Base::Berenstein)[3 * i]
          .get()
          .patch<mge::TransformComponent>(
              [&e, &i](auto& transform) { transform.set_position(e[i]); });
      m_control_points.at(Base::Berenstein)[3 * i + 1]
          .get()
          .patch<mge::TransformComponent>(
              [&f, &i](auto& transform) { transform.set_position(f[i]); });
      m_control_points.at(Base::Berenstein)[3 * i + 2]
          .get()
          .patch<mge::TransformComponent>(
              [&g, &i](auto& transform) { transform.set_position(g[i]); });
    }
    m_control_points.at(Base::Berenstein)[3 * segments]
        .get()
        .patch<mge::TransformComponent>([&e, &segments](auto& transform) {
          transform.set_position(e[segments]);
        });
  }
  entity.patch<mge::RenderableComponent<GeometryVertex>>(
      [this](auto& renderbale) {
        renderbale.get_vertex_array().update_vertices(generate_geometry());
      });
  m_polygon.patch<mge::RenderableComponent<GeometryVertex>>(
      [this](auto& renderbale) {
        renderbale.get_vertex_array().update_vertices(
            generate_polygon_geometry());
      });
}

void BezierComponentC2::set_polygon_status(bool status) {
  m_polygon.patch<mge::RenderableComponent<GeometryVertex>>(
      [&status](auto& renderable) {
        if (status) {
          renderable.enable();
        } else {
          renderable.disable();
        }
      });
}

void BezierComponentC2::set_base(BezierComponentC2::Base base) {
  m_base = base;
  for (auto& point : m_control_points.at(base)) {
    point.get().patch<mge::RenderableComponent<GeometryVertex>>(
        [](auto& renderable) { renderable.enable(); });
  }
  for (auto& point :
       m_control_points.at(static_cast<Base>(1 - static_cast<int>(base)))) {
    point.get().patch<mge::RenderableComponent<GeometryVertex>>(
        [](auto& renderable) { renderable.disable(); });
  }
}

std::vector<GeometryVertex> BezierComponentC2::generate_geometry() {
  std::vector<GeometryVertex> points;
  if (m_control_points.empty()) {
    return points;
  }
  int size = m_control_points.size() <= 4
                 ? 4
                 : (m_control_points.at(Base::Berenstein).size() + 1) / 3 * 4;
  points.reserve(size);
  for (int i = 0; i < m_control_points.size(); ++i) {
    if (i > 0 && i % 3 == 0) {
      points.emplace_back(m_control_points.at(Base::Berenstein)[i]
                              .get()
                              .get_component<mge::TransformComponent>()
                              .get_position());
    }
    points.emplace_back(m_control_points.at(Base::Berenstein)[i]
                            .get()
                            .get_component<mge::TransformComponent>()
                            .get_position());
  }
  for (int i = points.size(); i < size; ++i) {
    points.emplace_back(glm::vec3{std::numeric_limits<float>::quiet_NaN(),
                                  std::numeric_limits<float>::quiet_NaN(),
                                  std::numeric_limits<float>::quiet_NaN()});
  }

  return points;
}

std::vector<GeometryVertex> BezierComponentC2::generate_polygon_geometry() {
  std::vector<GeometryVertex> points(m_control_points.at(m_base).size());
  for (int i = 0; i < m_control_points.at(m_base).size(); ++i) {
    points[i] = {m_control_points.at(m_base)[i]
                     .get()
                     .get_component<mge::TransformComponent>()
                     .get_position()};
  }
  return points;
}

void BezierComponentC2::add_control_point(mge::Entity& control_point) {
  auto& second_control_point =
      m_scene.create_entity(PointComponent::get_new_name());
  second_control_point.add_component<PointComponent>();
  second_control_point.add_component<SelectibleComponent>();
  m_control_points.at(m_base).emplace_back(control_point);
  m_control_points.at(static_cast<Base>(1 - static_cast<int>(m_base)))
      .emplace_back(second_control_point);
}

void BezierComponentC2::remove_control_point(mge::Entity& control_point) {
  if (m_base == Base::DeBoor) {
    if (m_control_points.at(Base::Berenstein).size() == 4) {
      m_control_points.at(Base::Berenstein).clear();
    } else if (m_control_points.at(Base::Berenstein).size() > 4) {
      m_control_points.at(Base::Berenstein)
          .erase(--m_control_points.at(Base::Berenstein).end());
    }
  } else {
    if (m_control_points.at(Base::Berenstein).size() >= 4) {
      m_control_points.at(Base::DeBoor)
          .erase(--m_control_points.at(Base::DeBoor).end());
    }
  }
  m_control_points.at(m_base).erase(
      std::remove(m_control_points.at(m_base).begin(),
                  m_control_points.at(m_base).end(), control_point),
      m_control_points.at(m_base).end());
}

bool BezierComponentC2::get_polygon_status() const {
  return m_polygon.get_component<mge::RenderableComponent<GeometryVertex>>()
      .is_enabled();
}
