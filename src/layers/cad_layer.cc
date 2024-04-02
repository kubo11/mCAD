#include "cad_layer.hh"

#include "../events/add_event.hh"
#include "../events/select_event.hh"
#include "../geometry/cursor_vertex.hh"
#include "../geometry/geometry_vertex.hh"
#include "../geometry/non_selectible_component.hh"
#include "../geometry/point_component.hh"
#include "../geometry/torus_component.hh"
#include "ui_layer.hh"

const glm::vec3 EntitySelectionManager::s_selected_color = {1.0f, 0.5f, 0.0f};
const glm::vec3 EntitySelectionManager::s_regular_color = {0.0f, 0.0f, 0.0f};
const std::string CadLayer::s_cursor_tag = "cursor";
const std::string CadLayer::s_mass_center_tag = "mass_center";

EntitySelectionManager::EntitySelectionManager(mge::Entity& mass_center)
    : m_selected_entities{}, m_send_event{}, m_mass_center(mass_center) {
  m_mass_center.add_component<mge::TransformComponent>(
      glm::vec3(0.0f, 0.0f, 0.0f));
  m_mass_center.add_component<PointComponent>();
  m_mass_center.add_component<NonSelectibleComponent>();
  auto& renderable =
      m_mass_center.add_component<mge::RenderableComponent<GeometryVertex>>(
          PointComponent::get_shader(),
          std::move(PointComponent::get_vertex_array()));
  renderable.set_render_mode(mge::RenderMode::SURFACE);
  renderable.set_color({1.0f, 0.0f, 0.0f});
  renderable.disable();
}

void EntitySelectionManager::add_entity(mge::Entity& entity) {
  if (m_selected_entities.contains(entity)) {
    return;
  }
  auto& tag = entity.get_component<mge::TagComponent>();
  entity.get_component<mge::RenderableComponent<GeometryVertex>>().set_color(
      s_selected_color);
  m_selected_entities.emplace(entity);
  update_mass_center();
  SelectEntityByTagEvent event(tag);
  m_send_event(event);
}

void EntitySelectionManager::remove_entity(mge::Entity& entity) {
  if (!m_selected_entities.contains(entity)) {
    return;
  }
  auto& tag = entity.get_component<mge::TagComponent>();
  entity.get_component<mge::RenderableComponent<GeometryVertex>>().set_color(
      s_regular_color);
  m_selected_entities.erase(entity);
  update_mass_center();
  UnSelectEntityByTagEvent event(tag);
  m_send_event(event);
}

void EntitySelectionManager::remove_all() {
  if (m_selected_entities.empty()) {
    return;
  }
  for (auto& entity : m_selected_entities) {
    entity.get()
        .get_component<mge::RenderableComponent<GeometryVertex>>()
        .set_color(s_regular_color);
  }
  m_selected_entities.clear();
  update_mass_center();
  UnSelectAllEntitiesEvent event;
  m_send_event(event);
}

void EntitySelectionManager::foreach (std::function<void(mge::Entity&)> func) {
  for (auto& entity : m_selected_entities) {
    func(entity.get());
  }
  update_mass_center();
}

void EntitySelectionManager::update_mass_center() {
  if (m_selected_entities.size() < 2) {
    m_mass_center.get_component<mge::RenderableComponent<GeometryVertex>>()
        .disable();
    if (m_selected_entities.empty()) {
      return;
    }
  } else {
    m_mass_center.get_component<mge::RenderableComponent<GeometryVertex>>()
        .enable();
  }

  glm::vec3 center = {0.0f, 0.0f, 0.0f};
  float count = 0.0f;
  for (auto& entity : m_selected_entities) {
    if (entity.get().has_component<mge::TransformComponent>()) {
      auto& transform_component =
          entity.get().get_component<mge::TransformComponent>();
      center += transform_component.get_position();
      count++;
    }
  }
  center /= count;

  m_mass_center.get_component<mge::TransformComponent>().set_position(center);
}

const glm::vec3& EntitySelectionManager::get_mass_center_position() const {
  return m_mass_center.get_component<mge::TransformComponent>().get_position();
}

CadLayer::CadLayer(mge::Scene& scene)
    : m_scene(scene),
      m_cursor(m_scene.create_entity(CadLayer::s_cursor_tag)),
      m_selection_manager(m_scene.create_entity(CadLayer::s_mass_center_tag)) {}

void CadLayer::configure() {
  m_selection_manager.set_event_handler(m_send_event);
  m_cursor.add_component<mge::TransformComponent>();
  m_cursor.add_component<NonSelectibleComponent>();
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
  m_scene.draw<GeometryVertex>([&camera](mge::Shader& shader,
                                         mge::Entity& entity) {
    shader.set_uniform("projection_view", camera.get_projection_view_matrix());
    auto& transform = entity.get_component<mge::TransformComponent>();
    if (entity.has_component<PointComponent>()) {
      transform.set_scale(glm::vec3(PointComponent::s_scale));
    }
    shader.set_uniform("model", transform.get_model_mat());
    auto& renderable =
        entity.get_component<mge::RenderableComponent<GeometryVertex>>();
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
  mge::Event::try_handler<mge::CameraAngleEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_camera_angle_modified));
  mge::Event::try_handler<mge::CameraPositionEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_camera_position_modified));
  mge::Event::try_handler<mge::CameraZoomEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_camera_zoom));

  mge::Event::try_handler<SelectEntityByTagEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_select_entity_by_tag));
  mge::Event::try_handler<SelectEntityByPositionEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_select_entity_by_position));
  mge::Event::try_handler<QuerySelectedEntityEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_query_selected_entity));
  mge::Event::try_handler<UnSelectEntityByTagEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_unselect_entity_by_tag));
  mge::Event::try_handler<UnSelectAllEntitiesEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_unselect_all_entities));

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

  mge::Event::try_handler<RenameEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_rename_event));

  mge::Event::try_handler<TorusUpdateGeometryEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_torus_update_geometry));
  mge::Event::try_handler<TorusUpdateTopologyEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_torus_update_topology));
}

bool CadLayer::on_select_entity_by_tag(SelectEntityByTagEvent& event) {
  m_selection_manager.add_entity(m_scene.get_entity(event.get_tag()));
  return true;
}

bool CadLayer::on_select_entity_by_position(
    SelectEntityByPositionEvent& event) {
  auto entity = get_closest_entity(event.get_position());
  if (entity.has_value()) {
    m_selection_manager.add_entity(entity.value().get());
  } else {
    m_selection_manager.remove_all();
  }

  return true;
}

bool CadLayer::on_query_selected_entity(QuerySelectedEntityEvent& event) {
  event.set_entity(m_selection_manager.get_single_selected_entity());
  return true;
}

bool CadLayer::on_unselect_entity_by_tag(UnSelectEntityByTagEvent& event) {
  m_selection_manager.remove_entity(m_scene.get_entity(event.get_tag()));
  return true;
}

bool CadLayer::on_unselect_all_entities(UnSelectAllEntitiesEvent& event) {
  m_selection_manager.remove_all();
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
  auto& entity = m_scene.create_entity(tag);
  entity.add_component<PointComponent>();
  entity.add_component<mge::TransformComponent>(
      m_cursor.get_component<mge::TransformComponent>().get_position());
  auto& shader = PointComponent::get_shader();
  auto vertex_array = PointComponent::get_vertex_array();
  entity
      .add_component<mge::RenderableComponent<GeometryVertex>>(
          shader, std::move(vertex_array))
      .set_render_mode(mge::RenderMode::SURFACE);

  AnnounceNewEntityEvent announce_event(tag);
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

  AnnounceNewEntityEvent announce_event(tag);
  m_send_event(announce_event);

  return true;
}

bool CadLayer::on_delete_position_event(DeletePositionEvent& event) {
  auto entity = get_closest_entity(event.get_position());
  if (entity.has_value()) {
    m_selection_manager.remove_entity(entity.value());
    auto& tag_component =
        entity.value().get().get_component<mge::TagComponent>();
    DeleteEntityByTagEvent delete_event(tag_component);
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

bool CadLayer::on_rename_event(RenameEvent& event) {
  bool status = m_scene.rename_entity(event.get_old_tag(), event.get_new_tag());
  event.set_status(status);
  return true;
}

bool CadLayer::on_scale_event(ScaleEvent& event) {
  if (m_selection_manager.empty()) {
    return true;
  }

  glm::vec3 center = m_selection_manager.get_mass_center_position();
  float scale_factor = glm::distance(center, unproject_point(event.get_end())) /
                       glm::distance(center, unproject_point(event.get_beg()));
  m_selection_manager.foreach ([&scale_factor](mge::Entity& entity) {
    entity.get_component<mge::TransformComponent>().scale(scale_factor);
  });

  return true;
}

bool CadLayer::on_rotate_event(RotateEvent& event) {
  if (m_selection_manager.empty()) {
    return true;
  }

  glm::vec3 center = m_selection_manager.get_mass_center_position();
  glm::vec2 center_ss =
      m_scene.get_current_camera().get_projection_view_matrix() *
      glm::vec4(center, 1.0f);
  float angle =
      glm::asin(
          glm::cross(
              glm::vec3(glm::normalize(event.get_beg() - center_ss), 0.0f),
              glm::vec3(glm::normalize(event.get_end() - center_ss), 0.0f)))
          .z;
  m_selection_manager.foreach ([&angle, &center, &event](mge::Entity& entity) {
    auto& transform_component = entity.get_component<mge::TransformComponent>();
    transform_component.rotate(angle, event.get_axis());
    transform_component.set_position(
        center + glm::angleAxis(angle, event.get_axis()) *
                     (transform_component.get_position() - center));
  });

  return true;
}

bool CadLayer::on_move_event(MoveEvent& event) {
  if (m_selection_manager.empty()) {
    return true;
  }

  glm::vec3 center = m_selection_manager.get_mass_center_position();
  glm::vec3 offset =
      m_cursor.get_component<mge::TransformComponent>().get_position() - center;
  m_selection_manager.foreach ([&offset](mge::Entity& entity) {
    entity.get_component<mge::TransformComponent>().translate(offset);
  });

  return true;
}

bool CadLayer::on_torus_update_geometry(TorusUpdateGeometryEvent& event) {
  auto& entity = m_scene.get_entity(event.get_tag());
  auto& torus = entity.get_component<TorusComponent>();
  auto& renderable =
      entity.get_component<mge::RenderableComponent<GeometryVertex>>();
  renderable.get_vertex_array().update_vertices(
      std::move(torus.generate_geometry()));
  return true;
}

bool CadLayer::on_torus_update_topology(TorusUpdateTopologyEvent& event) {
  auto& entity = m_scene.get_entity(event.get_tag());
  auto& torus = entity.get_component<TorusComponent>();
  auto& renderable =
      entity.get_component<mge::RenderableComponent<GeometryVertex>>();
  if (renderable.get_render_mode() == mge::RenderMode::SURFACE) {
    renderable.get_vertex_array().update_indices(
        std::move(torus.generate_topology<mge::RenderMode::SURFACE>()));
  } else if (renderable.get_render_mode() == mge::RenderMode::WIREFRAME) {
    renderable.get_vertex_array().update_indices(
        std::move(torus.generate_topology<mge::RenderMode::WIREFRAME>()));
  }
  return true;
}

glm::vec3 CadLayer::unproject_point(glm::vec2 pos) const {
  glm::vec4 unprojected_point =
      glm::inverse(m_scene.get_current_camera().get_projection_view_matrix()) *
      glm::vec4(pos, 0.0f, 1.0f);
  return glm::vec3(unprojected_point) / unprojected_point.w;
}

std::optional<std::reference_wrapper<mge::Entity>> CadLayer::get_closest_entity(
    glm::vec2 position) {
  float min_dist = 10e7;
  std::string closest_tag = "";
  auto& camera = m_scene.get_current_camera();
  m_scene.foreach<>(entt::get<mge::TransformComponent, mge::TagComponent>,
                    entt::exclude<NonSelectibleComponent>,
                    [&](auto entity, auto& transform, auto& tag) {
                      auto dist = glm::distance(
                          position,
                          glm::vec2(camera.get_projection_view_matrix() *
                                    glm::vec4(transform.get_position(), 1.0f)));
                      if (dist < min_dist && dist < 0.03f) {
                        min_dist = dist;
                        closest_tag = tag.get_tag();
                      }
                    });

  if (closest_tag.size()) {
    return m_scene.get_entity(closest_tag);
  }

  return std::nullopt;
}