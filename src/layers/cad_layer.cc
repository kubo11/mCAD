#include "cad_layer.hh"

#include "../events/add_event.hh"
#include "../events/select_event.hh"
#include "../geometry/cursor_vertex.hh"
#include "../geometry/geometry_vertex.hh"
#include "../geometry/torus_component.hh"
#include "ui_layer.hh"

CadLayer::CadLayer(mge::Scene& scene)
    : m_scene(scene), m_cursor(m_scene.create_entity("cursor")) {}

void CadLayer::configure() {
  m_cursor.add_component<mge::TransformComponent>();
  m_cursor.add_component<mge::RenderableComponent<CursorVertex>>(
      mge::ShaderSystem::acquire(fs::current_path() / "src" / "shaders" /
                                 "cursor"),
      std::move(std::make_unique<mge::VertexArray<CursorVertex>>(
          std::vector<CursorVertex>{{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
                                    {{0.3f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                    {{0.0f, 0.3f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                    {{0.0f, 0.0f, 0.3f}, {0.0f, 0.0f, 1.0f}}},
          CursorVertex::get_vertex_attributes(),
          std::vector<unsigned int>{0, 1, 0, 2, 0, 3})));
}

void CadLayer::update() {
  auto& camera = m_scene.get_current_camera();
  DrawMassCenterEvent mass_center_event;
  m_send_event(mass_center_event);
  if (m_scene.contains("mass_center")) {
    if (mass_center_event.get_mass_center()) {
      auto& mass_center = m_scene.get_entity("mass_center");
      auto& transform = mass_center.get_component<mge::TransformComponent>();
      transform.set_position(mass_center_event.get_mass_center().value());
    } else {
      m_scene.destroy_entity("mass_center");
    }
  } else {
    if (mass_center_event.get_mass_center()) {
      auto& point = m_scene.create_entity("mass_center");
      point
          .add_component<mge::TransformComponent>(
              m_cursor.get_component<mge::TransformComponent>().get_position())
          .set_scale({0.05f, 0.05f, 0.05f});
      auto& point_component = point.add_component<PointComponent>(1);
      auto& shader = mge::ShaderSystem::acquire(
          fs::current_path() / "src" / "shaders" / "solid" / "surface");
      auto vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
          point_component.generate_geometry(),
          GeometryVertex::get_vertex_attributes(),
          point_component.generate_topology());
      auto& renderable =
          point.add_component<mge::RenderableComponent<GeometryVertex>>(
              shader, std::move(vertex_array));
      renderable.set_render_mode(mge::RenderMode::SURFACE);
      renderable.set_color({1.0f, 0.0f, 0.0f});
    }
  }

  m_scene.draw<GeometryVertex>([&camera](mge::Shader& shader,
                                         mge::Entity& entity) {
    auto& transform = entity.get_component<mge::TransformComponent>();
    auto& renderable =
        entity.get_component<mge::RenderableComponent<GeometryVertex>>();
    if (entity.has_component<PointComponent>()) {
      transform.set_scale(glm::vec3(PointComponent::s_scale,
                                    PointComponent::s_scale,
                                    PointComponent::s_scale));
    }
    shader.set_uniform("projection_view", camera.get_projection_view_matrix());
    shader.set_uniform("model", transform.get_model_mat());
    shader.set_uniform("color", renderable.get_color());
  });
  m_scene.draw<CursorVertex>([&camera](mge::Shader& shader,
                                       mge::Entity& entity) {
    auto& transform = entity.get_component<mge::TransformComponent>();
    shader.set_uniform("projection_view", camera.get_projection_view_matrix());
    shader.set_uniform("model", transform.get_model_mat());
  });
}

void CadLayer::handle_event(mge::Event& event, float dt) {
  mge::Event::try_handler<QuerySelectedTagEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_query_selected_tag_event));

  mge::Event::try_handler<QuerySelectedCursorEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_query_selected_cursor_event));

  mge::Event::try_handler<AddPointEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_add_point_event));

  mge::Event::try_handler<AddTorusEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_add_torus_event));

  mge::Event::try_handler<DeletePositionEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_delete_position_event));

  mge::Event::try_handler<CursorMoveEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_cursor_move_event));

  mge::Event::try_handler<ScaleEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_scale_event));

  mge::Event::try_handler<MoveEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_move_event));

  mge::Event::try_handler<RotateEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_rotate_event));

  mge::Event::try_handler<mge::CameraAngleEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_camera_angle_modified));

  mge::Event::try_handler<mge::CameraPositionEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_camera_position_modified));

  mge::Event::try_handler<mge::CameraZoomEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_camera_zoom));
}

bool CadLayer::on_query_selected_tag_event(QuerySelectedTagEvent& event) {
  SelectEvent selected_event(m_scene.get_entity(event.get_tag()));
  m_send_event(selected_event);
  return true;
}

bool CadLayer::on_query_selected_cursor_event(QuerySelectedCursorEvent& event) {
  SelectEvent selected_event(
      m_scene.get_closest_entity_ss(event.get_position()));
  m_send_event(selected_event);

  return true;
}

bool CadLayer::on_camera_angle_modified(mge::CameraAngleEvent& event) {
  m_scene.get_current_camera().add_azimuth(event.get_azimuth());
  m_scene.get_current_camera().add_elevation(event.get_elevation());
  return true;
}

bool CadLayer::on_camera_position_modified(mge::CameraPositionEvent& event) {
  m_scene.get_current_camera().move(event.get_pos());
  return true;
}

bool CadLayer::on_camera_zoom(mge::CameraZoomEvent& event) {
  m_scene.get_current_camera().zoom(event.get_zoom());
  m_cursor.get_component<mge::TransformComponent>().scale(event.get_zoom());
  PointComponent::s_scale *= event.get_zoom();
  return true;
}

bool CadLayer::on_add_point_event(AddPointEvent& event) {
  auto tag = PointComponent::get_new_name();
  auto& point = m_scene.create_entity(tag);
  point
      .add_component<mge::TransformComponent>(
          m_cursor.get_component<mge::TransformComponent>().get_position())
      .set_scale({0.05f, 0.05f, 0.05f});
  auto& point_component = point.add_component<PointComponent>(1);
  auto& shader = mge::ShaderSystem::acquire(fs::current_path() / "src" /
                                            "shaders" / "solid" / "surface");
  auto vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      point_component.generate_geometry(),
      GeometryVertex::get_vertex_attributes(),
      point_component.generate_topology());
  point
      .add_component<mge::RenderableComponent<GeometryVertex>>(
          shader, std::move(vertex_array))
      .set_render_mode(mge::RenderMode::SURFACE);

  AnnounceNewEvent announce_event(tag);
  m_send_event(announce_event);

  return true;
}

bool CadLayer::on_add_torus_event(AddTorusEvent& event) {
  auto tag = TorusComponent::get_new_name();
  auto& torus = m_scene.create_entity(tag);
  torus.add_component<mge::TransformComponent>(
      m_cursor.get_component<mge::TransformComponent>().get_position());
  auto& torus_component = torus.add_component<TorusComponent>();
  auto& shader = mge::ShaderSystem::acquire(fs::current_path() / "src" /
                                            "shaders" / "solid" / "wireframe");
  auto vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      torus_component.generate_geometry(),
      GeometryVertex::get_vertex_attributes(),
      torus_component.generate_topology<mge::RenderMode::WIREFRAME>());
  torus.add_component<mge::RenderableComponent<GeometryVertex>>(
      shader, std::move(vertex_array));

  AnnounceNewEvent announce_event(tag);
  m_send_event(announce_event);

  return true;
}

bool CadLayer::on_delete_position_event(DeletePositionEvent& event) {
  auto entity = m_scene.get_closest_entity_ss(event.get_position());
  if (entity.has_value()) {
    auto& tag_component =
        entity.value().get().get_component<mge::TagComponent>();
    DeleteTagEvent delete_event(tag_component);
    m_send_event(delete_event);
    m_scene.destroy_entity(tag_component);
  }

  return true;
}

bool CadLayer::on_cursor_move_event(CursorMoveEvent& event) {
  auto& transform_component = m_cursor.get_component<mge::TransformComponent>();
  transform_component.set_position(unproject_point(event.get_position()));
  return true;
}

bool CadLayer::on_scale_event(ScaleEvent& event) {
  if (event.get_selected().empty()) {
    return true;
  }

  glm::vec3 center = {0.0f, 0.0f, 0.0f};
  for (auto& entity : event.get_selected()) {
    auto& transform_component =
        entity.get().get_component<mge::TransformComponent>();
    center += transform_component.get_position();
  }
  center /= static_cast<float>(event.get_selected().size());
  float scale_factor = glm::distance(center, unproject_point(event.get_end())) /
                       glm::distance(center, unproject_point(event.get_beg()));

  for (auto& entity : event.get_selected()) {
    entity.get().get_component<mge::TransformComponent>().scale(scale_factor);
  }

  return true;
}

bool CadLayer::on_rotate_event(RotateEvent& event) {
  if (event.get_selected().empty()) {
    return true;
  }

  glm::vec3 center = {0.0f, 0.0f, 0.0f};
  for (auto& entity : event.get_selected()) {
    auto& transform_component =
        entity.get().get_component<mge::TransformComponent>();
    center += transform_component.get_position();
  }
  center /= static_cast<float>(event.get_selected().size());
  glm::vec2 center_screen_space =
      m_scene.get_current_camera().get_projection_view_matrix() *
      glm::vec4(center, 1.0f);

  float angle =
      glm::asin(
          glm::cross(
              glm::vec3(glm::normalize(event.get_beg() - center_screen_space),
                        0.0f),
              glm::vec3(glm::normalize(event.get_end() - center_screen_space),
                        0.0f)))
          .z;

  for (auto& entity : event.get_selected()) {
    auto& transform_component =
        entity.get().get_component<mge::TransformComponent>();
    transform_component.rotate(angle, event.get_axis());
    transform_component.set_position(
        center + glm::angleAxis(angle, event.get_axis()) *
                     (transform_component.get_position() - center));
  }

  return true;
}

bool CadLayer::on_move_event(MoveEvent& event) {
  if (event.get_selected().empty()) {
    return true;
  }

  glm::vec3 center = {0.0f, 0.0f, 0.0f};
  for (auto& entity : event.get_selected()) {
    auto& transform_component =
        entity.get().get_component<mge::TransformComponent>();
    center += transform_component.get_position();
  }
  center /= static_cast<float>(event.get_selected().size());
  glm::vec3 offset =
      m_cursor.get_component<mge::TransformComponent>().get_position() - center;

  for (auto& entity : event.get_selected()) {
    entity.get().get_component<mge::TransformComponent>().translate(offset);
  }

  return true;
}

glm::vec3 CadLayer::unproject_point(glm::vec2 pos) const {
  glm::vec4 unprojected_point =
      glm::inverse(m_scene.get_current_camera().get_projection_view_matrix()) *
      glm::vec4(pos, 0.0f, 1.0f);
  return glm::vec3(unprojected_point) / unprojected_point.w;
}