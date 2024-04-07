#include "cad_layer.hh"

#include "../geometry/cursor_component.hh"
#include "../geometry/cursor_vertex.hh"
#include "../geometry/mass_center_component.hh"
#include "../geometry/point_component.hh"
#include "../geometry/selectible_component.hh"
#include "../geometry/torus_component.hh"

const std::string CadLayer::s_cursor_tag = "cursor";
const std::string CadLayer::s_mass_center_tag = "mass_center";

CadLayer::CadLayer(mge::Scene& scene)
    : m_scene(scene),
      m_cursor(m_scene.create_entity(CadLayer::s_cursor_tag)),
      m_mass_center(m_scene.create_entity(CadLayer::s_mass_center_tag)) {}

void CadLayer::configure() {
  m_scene.on_construct<TorusComponent, &TorusComponent::on_construct>();
  m_scene.on_update<TorusComponent, &TorusComponent::on_update>();
  m_scene.on_construct<PointComponent, &PointComponent::on_construct>();
  m_scene.on_update<SelectibleComponent,
                    &SelectibleComponent::on_update<GeometryVertex>>();
  m_scene.on_construct<CursorComponent, &CursorComponent::on_construct>();
  m_scene.on_construct<SelectedComponent, &SelectedComponent::on_construct>();
  m_scene.on_destroy<SelectedComponent, &SelectedComponent::on_destroy>();
  m_scene.on_construct<SelectedChildComponent,
                       &SelectedChildComponent::on_construct>();
  m_scene.on_destroy<SelectedChildComponent,
                     &SelectedChildComponent::on_destroy>();
  m_scene
      .on_construct<MassCenterComponent, &MassCenterComponent::on_construct>();
  m_scene.on_construct<SelectedComponent, &CadLayer::on_seleciton_changed>(
      *this);
  m_scene.on_construct<SelectedChildComponent, &CadLayer::on_seleciton_changed>(
      *this);
  m_scene.on_update<mge::TransformComponent, &CadLayer::on_transform_changed>(
      *this);

  m_cursor.add_component<CursorComponent>();
  m_mass_center.add_component<MassCenterComponent>();
}

void CadLayer::update() {
  auto& camera = m_scene.get_current_camera();
  m_scene.draw<GeometryVertex>([&camera](std::shared_ptr<mge::Shader> shader,
                                         mge::Entity& entity) {
    shader->set_uniform("projection_view", camera.get_projection_view_matrix());
    if (entity.has_component<PointComponent>()) {
      entity.patch<mge::TransformComponent>([](auto& transform) {
        transform.set_scale(glm::vec3(PointComponent::s_scale));
      });
    }
    auto& transform = entity.get_component<mge::TransformComponent>();
    shader->set_uniform("model", transform.get_model_mat());
    auto& renderable =
        entity.get_component<mge::RenderableComponent<GeometryVertex>>();
    shader->set_uniform("color", renderable.get_color());
  });
  m_scene.draw<CursorVertex>([&camera](std::shared_ptr<mge::Shader> shader,
                                       mge::Entity& entity) {
    auto& transform = entity.get_component<mge::TransformComponent>();
    shader->set_uniform("projection_view", camera.get_projection_view_matrix());
    shader->set_uniform("model", transform.get_model_mat());
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

  mge::Event::try_handler<MoveByCursorEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_move_by_cursor_event));
  mge::Event::try_handler<ScaleByCursorEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_scale_by_cursor_event));
  mge::Event::try_handler<RotateByCursorEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_rotate_by_cursor_event));
  mge::Event::try_handler<MoveByUIEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_move_by_ui_event));
  mge::Event::try_handler<ScaleByUIEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_scale_by_ui_event));
  mge::Event::try_handler<RotateByUIEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_rotate_by_ui_event));

  mge::Event::try_handler<RenameEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_rename_event));
  mge::Event::try_handler<TorusUpdatedEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_torus_updated));
  mge::Event::try_handler<mge::RenderModeUpdatedEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_render_mode_updated));
}

bool CadLayer::on_select_entity_by_tag(SelectEntityByTagEvent& event) {
  m_scene.get_entity(event.get_tag()).add_component<SelectedComponent>();
  return true;
}

bool CadLayer::on_select_entity_by_position(
    SelectEntityByPositionEvent& event) {
  auto entity = get_closest_selectible_entity(event.get_position());
  if (entity.has_value()) {
    entity.value().get().add_component<SelectedComponent>();
  } else {
    m_scene.clear<SelectedComponent>();
  }
  return true;
}

bool CadLayer::on_unselect_entity_by_tag(UnSelectEntityByTagEvent& event) {
  m_scene.get_entity(event.get_tag()).remove_component<SelectedComponent>();
  return true;
}

bool CadLayer::on_unselect_all_entities(UnSelectAllEntitiesEvent& event) {
  m_scene.clear<SelectedComponent>();
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
  m_cursor.patch<mge::TransformComponent>(
      [&event](auto& transform) { transform.scale(event.get_zoom()); });
  PointComponent::s_scale *= event.get_zoom();
  return true;
}

bool CadLayer::on_add_point_event(AddPointEvent& event) {
  auto tag = PointComponent::get_new_name();
  auto& entity = m_scene.create_entity(tag);
  entity.add_component<PointComponent>();
  entity.add_component<SelectibleComponent>();
  entity.patch<mge::TransformComponent>([this](auto& transform) {
    transform.set_position(
        m_cursor.get_component<mge::TransformComponent>().get_position());
  });
  return true;
}

bool CadLayer::on_add_torus_event(AddTorusEvent& event) {
  auto tag = TorusComponent::get_new_name();
  auto& entity = m_scene.create_entity(tag);
  entity.add_component<TorusComponent>();
  entity.add_component<SelectibleComponent>();
  entity.patch<mge::TransformComponent>([this](auto& transform) {
    transform.set_position(
        m_cursor.get_component<mge::TransformComponent>().get_position());
  });
  return true;
}

bool CadLayer::on_delete_position_event(DeletePositionEvent& event) {
  auto entity = get_closest_selectible_entity(event.get_position());
  if (entity.has_value()) {
    auto& tag = entity.value().get().get_component<mge::TagComponent>();
    m_scene.destroy_entity(tag);
  }
  return true;
}

bool CadLayer::on_cursor_move_event(CursorMoveEvent& event) {
  m_cursor.patch<mge::TransformComponent>([this, &event](auto& transform) {
    transform.set_position(unproject_point(event.get_position()));
  });
  return true;
}

bool CadLayer::on_rename_event(RenameEvent& event) {
  bool status = m_scene.rename_entity(event.get_old_tag(), event.get_new_tag());
  event.set_status(status);
  return true;
}

bool CadLayer::on_move_by_cursor_event(MoveByCursorEvent& event) {
  auto size =
      m_scene.size<>(entt::get_t<SelectedComponent, mge::TransformComponent>(),
                     entt::exclude_t<>()) +
      m_scene.size<>(
          entt::get_t<SelectedChildComponent, mge::TransformComponent>(),
          entt::exclude_t<>());
  if (size == 0) {
    return true;
  }

  glm::vec3 center =
      m_mass_center.get_component<mge::TransformComponent>().get_position();
  glm::vec3 offset =
      m_cursor.get_component<mge::TransformComponent>().get_position() - center;
  m_scene.foreach<>(
      entt::get_t<SelectedComponent, mge::TransformComponent>(),
      entt::exclude_t<>(), [&offset](mge::Entity& entity) {
        entity.template patch<mge::TransformComponent>(
            [&offset](auto& transform) { transform.translate(offset); });
      });
  m_scene.foreach<>(
      entt::get_t<SelectedChildComponent, mge::TransformComponent>(),
      entt::exclude_t<>(), [&offset](mge::Entity& entity) {
        entity.template patch<mge::TransformComponent>(
            [&offset](auto& transform) { transform.translate(offset); });
      });
  return true;
}

bool CadLayer::on_scale_by_cursor_event(ScaleByCursorEvent& event) {
  auto size =
      m_scene.size<>(entt::get_t<SelectedComponent, mge::TransformComponent>(),
                     entt::exclude_t<>()) +
      m_scene.size<>(
          entt::get_t<SelectedChildComponent, mge::TransformComponent>(),
          entt::exclude_t<>());
  if (size == 0) {
    return true;
  }

  glm::vec3 center =
      m_mass_center.get_component<mge::TransformComponent>().get_position();
  float scale_factor = glm::distance(center, unproject_point(event.get_end())) /
                       glm::distance(center, unproject_point(event.get_beg()));
  m_scene.foreach<>(entt::get_t<SelectedComponent, mge::TransformComponent>(),
                    entt::exclude_t<>(), [&scale_factor](mge::Entity& entity) {
                      entity.template patch<mge::TransformComponent>(
                          [&scale_factor](auto& transform) {
                            transform.scale(scale_factor);
                          });
                    });
  m_scene.foreach<>(
      entt::get_t<SelectedChildComponent, mge::TransformComponent>(),
      entt::exclude_t<>(), [&scale_factor](mge::Entity& entity) {
        entity.template patch<mge::TransformComponent>(
            [&scale_factor](auto& transform) {
              transform.scale(scale_factor);
            });
      });
  return true;
}

bool CadLayer::on_rotate_by_cursor_event(RotateByCursorEvent& event) {
  auto size =
      m_scene.size<>(entt::get_t<SelectedComponent, mge::TransformComponent>(),
                     entt::exclude_t<>()) +
      m_scene.size<>(
          entt::get_t<SelectedChildComponent, mge::TransformComponent>(),
          entt::exclude_t<>());
  if (size == 0) {
    return true;
  }

  glm::vec3 center =
      m_mass_center.get_component<mge::TransformComponent>().get_position();
  glm::vec2 center_ss =
      m_scene.get_current_camera().get_projection_view_matrix() *
      glm::vec4(center, 1.0f);
  float angle =
      glm::asin(
          glm::cross(
              glm::vec3(glm::normalize(event.get_beg() - center_ss), 0.0f),
              glm::vec3(glm::normalize(event.get_end() - center_ss), 0.0f)))
          .z;
  m_scene.foreach<>(
      entt::get_t<SelectedComponent, mge::TransformComponent>(),
      entt::exclude_t<>(), [&angle, &center, &event](mge::Entity& entity) {
        entity.template patch<mge::TransformComponent>(
            [&angle, &center, &event](auto& transform) {
              transform.rotate(angle, event.get_axis());
              transform.set_position(center +
                                     glm::angleAxis(angle, event.get_axis()) *
                                         (transform.get_position() - center));
            });
      });
  m_scene.foreach<>(
      entt::get_t<SelectedChildComponent, mge::TransformComponent>(),
      entt::exclude_t<>(), [&angle, &center, &event](mge::Entity& entity) {
        entity.template patch<mge::TransformComponent>(
            [&angle, &center, &event](auto& transform) {
              transform.rotate(angle, event.get_axis());
              transform.set_position(center +
                                     glm::angleAxis(angle, event.get_axis()) *
                                         (transform.get_position() - center));
            });
      });
  return true;
}

bool CadLayer::on_move_by_ui_event(MoveByUIEvent& event) {
  m_scene.foreach<>(entt::get_t<SelectedComponent, mge::TransformComponent>(),
                    entt::exclude_t<>(), [&event](mge::Entity& entity) {
                      entity.template patch<mge::TransformComponent>(
                          [&event](auto& transform) {
                            transform.set_position(event.get_offset());
                          });
                    });
  return true;
}

bool CadLayer::on_scale_by_ui_event(ScaleByUIEvent& event) {
  m_scene.foreach<>(entt::get_t<SelectedComponent, mge::TransformComponent>(),
                    entt::exclude_t<>(), [&event](mge::Entity& entity) {
                      entity.template patch<mge::TransformComponent>(
                          [&event](auto& transform) {
                            transform.set_scale(event.get_scale());
                          });
                    });
  return true;
}

bool CadLayer::on_rotate_by_ui_event(RotateByUIEvent& event) {
  m_scene.foreach<>(entt::get_t<SelectedComponent, mge::TransformComponent>(),
                    entt::exclude_t<>(), [&event](mge::Entity& entity) {
                      entity.template patch<mge::TransformComponent>(
                          [&event](auto& transform) {
                            transform.set_rotation(event.get_rotation());
                          });
                    });
  return true;
}

bool CadLayer::on_torus_updated(TorusUpdatedEvent& event) {
  m_scene.foreach<>(
      entt::get_t<SelectedComponent, TorusComponent>(), entt::exclude_t<>(),
      [&event](mge::Entity& entity) {
        entity.template patch<TorusComponent>([&event](auto& torus) {
          torus.set_inner_radius(event.get_inner_radius());
          torus.set_outer_radius(event.get_outer_radius());
          torus.set_horizontal_density(event.get_horizontal_density());
          torus.set_vertical_density(event.get_vertical_density());
        });
      });
  return true;
}

glm::vec3 CadLayer::unproject_point(glm::vec2 pos) const {
  glm::vec4 unprojected_point =
      glm::inverse(m_scene.get_current_camera().get_projection_view_matrix()) *
      glm::vec4(pos, 0.0f, 1.0f);
  return glm::vec3(unprojected_point) / unprojected_point.w;
}

std::optional<std::reference_wrapper<mge::Entity>>
CadLayer::get_closest_selectible_entity(glm::vec2 position) {
  float min_dist = 10e7;
  std::string closest_tag;
  auto& camera = m_scene.get_current_camera();
  m_scene.foreach<>(
      entt::get<mge::TransformComponent, mge::TagComponent,
                SelectibleComponent>,
      entt::exclude<>, [&](auto& entity) {
        auto& transform =
            entity.template get_component<mge::TransformComponent>();
        auto dist = glm::distance(
            position, glm::vec2(camera.get_projection_view_matrix() *
                                glm::vec4(transform.get_position(), 1.0f)));
        if (dist < min_dist && dist < 0.03f) {
          min_dist = dist;
          auto& tag = entity.template get_component<mge::TagComponent>();
          closest_tag = tag.get_tag();
        }
      });

  if (!closest_tag.empty()) {
    return m_scene.get_entity(closest_tag);
  }

  return std::nullopt;
}

void CadLayer::on_seleciton_changed(entt::registry& registry,
                                    entt::entity entity) {
  m_mass_center.patch<MassCenterComponent>([this](auto& mass_center) {
    mass_center.update_mass_center(m_scene, m_mass_center);
  });
}

bool CadLayer::on_render_mode_updated(mge::RenderModeUpdatedEvent& event) {
  m_scene.foreach<>(
      entt::get_t<SelectedComponent,
                  mge::RenderableComponent<GeometryVertex>>(),
      entt::exclude_t<>(), [&event](mge::Entity& entity) {
        entity.template patch<mge::RenderableComponent<GeometryVertex>>(
            [&event](auto& renderable) {
              renderable.set_render_mode(event.get_render_mode());
            });
      });
  m_scene.foreach<>(
      entt::get_t<SelectedChildComponent,
                  mge::RenderableComponent<GeometryVertex>>(),
      entt::exclude_t<>(), [&event](mge::Entity& entity) {
        entity.template patch<mge::RenderableComponent<GeometryVertex>>(
            [&event](auto& renderable) {
              renderable.set_render_mode(event.get_render_mode());
            });
      });
  return true;
}

void CadLayer::on_transform_changed(entt::registry& registry,
                                    entt::entity entity) {
  if (!registry.try_get<SelectedComponent>(entity) &&
      !registry.try_get<SelectedChildComponent>(entity)) {
    return;
  }

  m_mass_center.patch<MassCenterComponent>([this](auto& mass_center) {
    mass_center.update_mass_center(m_scene, m_mass_center);
  });
}