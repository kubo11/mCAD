#include "cad_layer.hh"

#include "../geometry/bezier_component.hh"
#include "../geometry/cursor_component.hh"
#include "../geometry/cursor_vertex.hh"
#include "../geometry/mass_center_component.hh"
#include "../geometry/point_component.hh"
#include "../geometry/selectible_component.hh"
#include "../geometry/torus_component.hh"

const std::string CadLayer::s_cursor_tag = "cursor";
const std::string CadLayer::s_mass_center_tag = "mass_center";

CadLayer::CadLayer(mge::Scene& scene, const glm::ivec2& window_size)
    : m_scene(scene),
      m_cursor(m_scene.create_entity(CadLayer::s_cursor_tag)),
      m_mass_center(m_scene.create_entity(CadLayer::s_mass_center_tag)),
      m_selected(),
      m_window_size(window_size) {}

CadLayer::~CadLayer() {
  m_scene.disconnect_all<mge::TransformComponent>();
  m_scene.disconnect_all<TorusComponent>();
  m_scene.disconnect_all<PointComponent>();
  m_scene.disconnect_all<CursorComponent>();
  m_scene.disconnect_all<MassCenterComponent>();
  m_scene.disconnect_all<SelectibleComponent>();
  m_scene.disconnect_all<BezierComponent>();
}

void CadLayer::configure() {
  m_scene.on_update<mge::TransformComponent, &CadLayer::on_transform_update>(
      *this);

  m_scene.on_construct<TorusComponent, &TorusComponent::on_construct>();
  m_scene.on_update<TorusComponent, &TorusComponent::on_update>();

  m_scene.on_construct<PointComponent, &PointComponent::on_construct>();

  m_scene.on_construct<BezierComponent, &BezierComponent::on_construct>();
  m_scene.on_update<BezierComponent, &BezierComponent::on_update>();

  m_scene.on_construct<CursorComponent, &CursorComponent::on_construct>();

  m_scene
      .on_construct<MassCenterComponent, &MassCenterComponent::on_construct>();

  m_scene.on_construct<SelectibleComponent, &CadLayer::on_selectible_construct>(
      *this);
  m_scene.on_update<SelectibleComponent,
                    &SelectibleComponent::on_update<GeometryVertex>>();
  m_scene.on_update<SelectibleComponent, &CadLayer::on_selectible_update>(
      *this);
  m_scene.on_destroy<SelectibleComponent, &CadLayer::on_selectible_destroy>(
      *this);

  m_cursor.add_component<CursorComponent>();
  m_mass_center.add_component<MassCenterComponent>();
}

void CadLayer::update() {
  auto& camera = m_scene.get_current_camera();
  m_scene.draw<GeometryVertex>([&camera, this](
                                   std::shared_ptr<mge::Shader> shader,
                                   mge::Entity& entity) {
    shader->set_uniform("projection_view", camera.get_projection_view_matrix());
    if (entity.has_component<PointComponent>()) {
      entity.patch<mge::TransformComponent>([](auto& transform) {
        transform.set_scale(glm::vec3(PointComponent::s_scale));
      });
    }
    if (entity.has_component<BezierComponent>()) {
      shader->set_uniform("window_size", m_window_size);
      glPatchParameteri(GL_PATCH_VERTICES, 4);
    }
    if (entity.has_component<mge::TransformComponent>()) {
      auto& transform = entity.get_component<mge::TransformComponent>();
      shader->set_uniform("model", transform.get_model_mat());
    }
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
  mge::Event::try_handler<AddBezierEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_add_bezier_event));

  mge::Event::try_handler<DeleteEntityByPositionEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_delete_entity_by_position));
  mge::Event::try_handler<DeleteEntityByTagEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_delete_entity_by_tag));

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

  mge::Event::try_handler<AddControlPointByPositionEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_add_control_point_by_position));
  mge::Event::try_handler<AddControlPointByTagEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_add_control_point_by_tag));
  mge::Event::try_handler<RemoveControlPointByPositionEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_remove_control_point_by_position));
  mge::Event::try_handler<RemoveControlPointByTagEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_remove_control_point_by_tag));

  mge::Event::try_handler<RenameEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_rename_event));
  mge::Event::try_handler<TorusUpdatedEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_torus_updated));
  mge::Event::try_handler<mge::RenderModeUpdatedEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_render_mode_updated));

  mge::Event::try_handler<mge::RenderModeUpdatedEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_render_mode_updated));

  mge::Event::try_handler<ShowBerensteinPolygonEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_show_berenstein_polygon));
  mge::Event::try_handler<QueryEntityByTagEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_query_entity_by_tag));

  mge::Event::try_handler<mge::WindowFramebufferResizedEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_window_framebuffer_resized));
}

bool CadLayer::on_select_entity_by_tag(SelectEntityByTagEvent& event) {
  if (m_scene.get_entity(event.get_tag())
          .get_component<SelectibleComponent>()
          .is_selected()) {
    return true;
  }

  add_selected(m_scene.get_entity(event.get_tag()));
  return true;
}

bool CadLayer::on_select_entity_by_position(
    SelectEntityByPositionEvent& event) {
  auto entity = get_closest_selectible_entity(event.get_position());
  if (entity.has_value()) {
    if (entity.value()
            .get()
            .get_component<SelectibleComponent>()
            .is_selected()) {
      return true;
    }

    add_selected(entity.value().get());
    SelectEntityByTagEvent select_event(
        entity->get().get_component<mge::TagComponent>());
    m_send_event(select_event);
  } else {
    remove_all_selected();
    UnSelectAllEntitiesEvent unselect_event;
    m_send_event(unselect_event);
  }
  return true;
}

bool CadLayer::on_unselect_entity_by_tag(UnSelectEntityByTagEvent& event) {
  if (!m_scene.get_entity(event.get_tag())
           .get_component<SelectibleComponent>()
           .is_selected()) {
    return true;
  }

  remove_selected(m_scene.get_entity(event.get_tag()));
  return true;
}

bool CadLayer::on_unselect_all_entities(UnSelectAllEntitiesEvent& event) {
  remove_all_selected();
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
  auto& entity = m_scene.create_entity(PointComponent::get_new_name());
  entity.template add_component<PointComponent>();
  entity.template patch<mge::TransformComponent>([this](auto& transform) {
    transform.set_position(
        m_cursor.get_component<mge::TransformComponent>().get_position());
  });
  entity.template add_component<SelectibleComponent>();
  return true;
}

bool CadLayer::on_add_torus_event(AddTorusEvent& event) {
  auto& entity = m_scene.create_entity(TorusComponent::get_new_name());
  entity.template add_component<TorusComponent>();
  entity.template patch<mge::TransformComponent>([this](auto& transform) {
    transform.set_position(
        m_cursor.get_component<mge::TransformComponent>().get_position());
  });
  entity.template add_component<SelectibleComponent>();
  return true;
}

bool CadLayer::on_add_bezier_event(AddBezierEvent& event) {
  for (auto& selected : m_selected) {
    if (!selected.get().has_component<PointComponent>()) {
      return true;
    }
  }

  auto& entity = m_scene.create_entity(BezierComponent::get_new_name());
  auto& polygon_entity = m_scene.create_entity(
      "polygon_" + entity.get_component<mge::TagComponent>().get_tag());
  entity.template add_component<BezierComponent>(m_selected, polygon_entity);
  entity.template add_component<SelectibleComponent>();
  return true;
}

bool CadLayer::on_delete_entity_by_position(
    DeleteEntityByPositionEvent& event) {
  auto entity = get_closest_selectible_entity(event.get_position());
  if (entity.has_value()) {
    if (!entity->get().get_parents().empty()) {
      return true;
    }
    remove_selected(entity.value());
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
  if (m_selected.empty()) {
    return true;
  }

  glm::vec3 center =
      m_mass_center.get_component<mge::TransformComponent>().get_position();
  glm::vec3 offset =
      m_cursor.get_component<mge::TransformComponent>().get_position() - center;
  for (auto& selected : m_selected) {
    selected.get().run_and_propagate([&offset](auto& entity) {
      if (!entity.template has_component<mge::TransformComponent>()) {
        return;
      }
      entity.template patch<mge::TransformComponent>(
          [&offset](auto& transform) { transform.translate(offset); });
    });
    if (selected.get().has_component<BezierComponent>()) {
      selected.get().patch<BezierComponent>([](auto& _) {});
    }
  }
  return true;
}

bool CadLayer::on_scale_by_cursor_event(ScaleByCursorEvent& event) {
  if (m_selected.empty()) {
    return true;
  }

  glm::vec3 center =
      m_mass_center.get_component<mge::TransformComponent>().get_position();
  float scale_factor = glm::distance(center, unproject_point(event.get_end())) /
                       glm::distance(center, unproject_point(event.get_beg()));
  for (auto& selected : m_selected) {
    selected.get().run_and_propagate([&scale_factor](auto& entity) {
      if (!entity.template has_component<mge::TransformComponent>()) {
        return;
      }
      entity.template patch<mge::TransformComponent>(
          [&scale_factor](auto& transform) { transform.scale(scale_factor); });
    });
    if (selected.get().has_component<BezierComponent>()) {
      selected.get().patch<BezierComponent>([](auto& _) {});
    }
  }
  return true;
}

bool CadLayer::on_rotate_by_cursor_event(RotateByCursorEvent& event) {
  if (m_selected.empty()) {
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
  for (auto& selected : m_selected) {
    selected.get().run_and_propagate([&angle, &center, &event](auto& entity) {
      if (!entity.template has_component<mge::TransformComponent>()) {
        return;
      }
      entity.template patch<mge::TransformComponent>(
          [&angle, &center, &event](auto& transform) {
            transform.rotate(angle, event.get_axis());
            transform.set_position(center +
                                   glm::angleAxis(angle, event.get_axis()) *
                                       (transform.get_position() - center));
          });
    });
    if (selected.get().has_component<BezierComponent>()) {
      selected.get().patch<BezierComponent>([](auto& _) {});
    }
  }
  return true;
}

bool CadLayer::on_move_by_ui_event(MoveByUIEvent& event) {
  for (auto& selected : m_selected) {
    selected.get().run_and_propagate([&event](auto& entity) {
      if (!entity.template has_component<mge::TransformComponent>()) {
        return;
      }
      entity.template patch<mge::TransformComponent>([&event](auto& transform) {
        transform.set_position(event.get_offset());
      });
    });
  }
  return true;
}

bool CadLayer::on_scale_by_ui_event(ScaleByUIEvent& event) {
  for (auto& selected : m_selected) {
    selected.get().run_and_propagate([&event](auto& entity) {
      if (!entity.template has_component<mge::TransformComponent>()) {
        return;
      }
      entity.template patch<mge::TransformComponent>([&event](auto& transform) {
        transform.set_scale(event.get_scale());
      });
    });
  }
  return true;
}

bool CadLayer::on_rotate_by_ui_event(RotateByUIEvent& event) {
  for (auto& selected : m_selected) {
    selected.get().run_and_propagate([&event](auto& entity) {
      if (!entity.template has_component<mge::TransformComponent>()) {
        return;
      }
      entity.template patch<mge::TransformComponent>([&event](auto& transform) {
        transform.set_rotation(event.get_rotation());
      });
    });
  }
  return true;
}

bool CadLayer::on_torus_updated(TorusUpdatedEvent& event) {
  auto& entity = m_scene.get_entity(event.get_tag());
  entity.patch<TorusComponent>([&event](auto& torus) {
    torus.set_inner_radius(event.get_inner_radius());
    torus.set_outer_radius(event.get_outer_radius());
    torus.set_horizontal_density(event.get_horizontal_density());
    torus.set_vertical_density(event.get_vertical_density());
  });
  return true;
}

bool CadLayer::on_render_mode_updated(mge::RenderModeUpdatedEvent& event) {
  auto& entity = m_scene.get_entity(event.get_tag());
  entity.patch<mge::RenderableComponent<GeometryVertex>>(
      [&event](auto& renderable) {
        renderable.set_render_mode(event.get_render_mode());
      });
  return true;
}

glm::vec3 CadLayer::unproject_point(glm::vec2 pos) const {
  glm::vec4 unprojected_point =
      glm::inverse(m_scene.get_current_camera().get_projection_view_matrix()) *
      glm::vec4(pos, 0.0f, 1.0f);
  return glm::vec3(unprojected_point) / unprojected_point.w;
}

mge::OptionalEntity CadLayer::get_closest_selectible_entity(
    glm::vec2 position) {
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

// FIXME: redundant calculations when multiple children are updated
void CadLayer::on_transform_update(entt::registry& registry,
                                   entt::entity entt_entity) {
  auto& entity =
      m_scene.get_entity(registry.get<mge::TagComponent>(entt_entity));
  for (auto& parent : entity.get_parents()) {
    if (!parent.get().has_component<BezierComponent>()) {
      continue;
    }

    parent.get().patch<BezierComponent>([](auto& bezier) {});
  }

  if (!entity.has_component<SelectibleComponent>() ||
      !entity.get_component<SelectibleComponent>().is_selected()) {
    return;
  }

  m_mass_center.patch<MassCenterComponent>([this](auto& mass_center) {
    mass_center.update_mass_center(m_selected, m_mass_center);
  });
}

void CadLayer::on_selectible_construct(entt::registry& registry,
                                       entt::entity entt_entity) {
  NewEntityEvent event(registry.get<mge::TagComponent>(entt_entity));
  m_send_event(event);
}

void CadLayer::on_selectible_update(entt::registry& registry,
                                    entt::entity entity) {
  bool selection = registry.get<SelectibleComponent>(entity).is_selected();
  if (selection) {
    SelectEntityByTagEvent event(registry.get<mge::TagComponent>(entity));
    m_send_event(event);
  } else {
    UnSelectEntityByTagEvent event(registry.get<mge::TagComponent>(entity));
    m_send_event(event);
  }
}

void CadLayer::on_selectible_destroy(entt::registry& registry,
                                     entt::entity entt_entity) {
  RemoveEntityEvent event(registry.get<mge::TagComponent>(entt_entity));
  m_send_event(event);
}

bool CadLayer::on_show_berenstein_polygon(ShowBerensteinPolygonEvent& event) {
  m_scene.get_entity(event.get_tag())
      .patch<BezierComponent>([&event](auto& bezier) {
        bezier.set_berenstein_polygon(event.get_status());
      });
  return true;
}

bool CadLayer::on_query_entity_by_tag(QueryEntityByTagEvent& event) {
  event.set_entity(m_scene.get_entity(event.get_tag()));
  return true;
}

bool CadLayer::on_add_control_point_by_position(
    AddControlPointByPositionEvent& event) {
  auto point = get_closest_selectible_entity(event.get_position());
  if (point.has_value() && point->get().has_component<PointComponent>()) {
    auto& bezier = m_scene.get_entity(event.get_bezier_tag());
    if (bezier.get_component<SelectibleComponent>().is_selected()) {
      point->get().patch<SelectibleComponent>(
          [](auto& selectible) { selectible.set_selection(true); });
    }
    bezier.add_child(point->get());
    bezier.patch<BezierComponent>([&point](auto& component) {
      component.add_control_point(point.value());
    });
  }
  return true;
}

bool CadLayer::on_remove_control_point_by_position(
    RemoveControlPointByPositionEvent& event) {
  auto point = get_closest_selectible_entity(event.get_position());
  if (point.has_value() && point->get().has_component<PointComponent>()) {
    auto& bezier = m_scene.get_entity(event.get_bezier_tag());
    if (bezier.get_component<SelectibleComponent>().is_selected()) {
      point->get().patch<SelectibleComponent>(
          [](auto& selectible) { selectible.set_selection(false); });
    }
    bezier.remove_child(point.value());
    bezier.patch<BezierComponent>([&point](auto& component) {
      component.remove_control_point(point.value());
    });
  }
  return true;
}

bool CadLayer::on_add_control_point_by_tag(AddControlPointByTagEvent& event) {
  auto& point = m_scene.get_entity(event.get_point_tag());
  auto& bezier = m_scene.get_entity(event.get_bezier_tag());
  if (point.has_component<PointComponent>()) {
    if (bezier.get_component<SelectibleComponent>().is_selected()) {
      point.patch<SelectibleComponent>(
          [](auto& selectible) { selectible.set_selection(true); });
    }
    bezier.add_child(point);
    bezier.patch<BezierComponent>(
        [&point](auto& component) { component.add_control_point(point); });
  }
  return true;
}

bool CadLayer::on_remove_control_point_by_tag(
    RemoveControlPointByTagEvent& event) {
  auto& point = m_scene.get_entity(event.get_point_tag());
  auto& bezier = m_scene.get_entity(event.get_bezier_tag());
  if (point.has_component<PointComponent>()) {
    if (bezier.get_component<SelectibleComponent>().is_selected()) {
      point.patch<SelectibleComponent>(
          [](auto& selectible) { selectible.set_selection(false); });
    }
    bezier.remove_child(point);
    bezier.patch<BezierComponent>(
        [&point](auto& component) { component.remove_control_point(point); });
  }
  return true;
}

bool CadLayer::on_delete_entity_by_tag(DeleteEntityByTagEvent& event) {
  if (m_scene.contains(event.get_tag())) {
    auto& entity = m_scene.get_entity(event.get_tag());
    if (!entity.get_parents().empty()) {
      return true;
    }
    remove_selected(entity);
    m_scene.destroy_entity(event.get_tag());
  }
  return true;
}

bool CadLayer::on_window_framebuffer_resized(
    mge::WindowFramebufferResizedEvent& event) {
  m_window_size = {event.get_width(), event.get_height()};
  return true;
}

void CadLayer::add_selected(mge::Entity& entity) {
  entity.run_and_propagate([](auto& entity) {
    if (entity.template has_component<SelectibleComponent>()) {
      entity.template patch<SelectibleComponent>(
          [](auto& selectible) { selectible.set_selection(true); });
    }
  });
  m_selected.emplace_back(entity);
  m_mass_center.patch<MassCenterComponent>([this](auto& mass_center) {
    mass_center.update_mass_center(m_selected, m_mass_center);
  });
  if (m_selected.size() == 1) {
    UpdateDisplayedEntityEvent event(*m_selected.begin());
    m_send_event(event);
  } else {
    UpdateDisplayedEntityEvent event(std::nullopt);
    m_send_event(event);
  }
}

void CadLayer::remove_selected(mge::Entity& entity) {
  entity.run_and_propagate([](auto& entity) {
    if (entity.template has_component<SelectibleComponent>()) {
      entity.template patch<SelectibleComponent>(
          [](auto& selectible) { selectible.set_selection(false); });
    }
  });
  m_selected.erase(std::remove(m_selected.begin(), m_selected.end(), entity),
                   m_selected.end());
  m_mass_center.patch<MassCenterComponent>([this](auto& mass_center) {
    mass_center.update_mass_center(m_selected, m_mass_center);
  });
  if (m_selected.size() == 1) {
    UpdateDisplayedEntityEvent event(*m_selected.begin());
    m_send_event(event);
  } else {
    UpdateDisplayedEntityEvent event(std::nullopt);
    m_send_event(event);
  }
}

void CadLayer::remove_all_selected() {
  for (auto& selected : m_selected) {
    selected.get().run_and_propagate([](auto& entity) {
      if (entity.template has_component<SelectibleComponent>()) {
        entity.template patch<SelectibleComponent>(
            [](auto& selectible) { selectible.set_selection(false); });
      }
    });
  }
  m_selected.clear();
  m_mass_center.patch<MassCenterComponent>([this](auto& mass_center) {
    mass_center.update_mass_center(m_selected, m_mass_center);
  });
  UpdateDisplayedEntityEvent event(std::nullopt);
  m_send_event(event);
}
