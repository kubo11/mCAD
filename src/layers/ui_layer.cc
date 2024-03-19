#include "ui_layer.hh"
#include "../events/input_state_event.hh"
#include "../input_state.hh"
#include "cad_layer.hh"

UILayer::UILayer(mge::Entity& cursor)
    : m_entities{}, m_selected{}, m_cursor(cursor) {}

void UILayer::configure() {}

void UILayer::update() {
  mge::UIManager::start_frame();

  //   bool dwnd = true;
  //   ImGui::ShowDemoWindow(&dwnd);

  auto size = ImGui::GetMainViewport()->Size;

  ImGui::SetNextWindowSize({std::min(size.x * 0.25f, 250.0f), size.y});
  ImGui::SetNextWindowPos({std::max(size.x * 0.75f, size.x - 250.f), 0});
  ImGui::Begin("ToolsParams", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove);

  if (ImGui::BeginTabBar("MainTabBar")) {
    if (ImGui::BeginTabItem("Main")) {
      ImGui::Text("tools");
      show_tools_panel();
      ImGui::Separator();

      ImGui::Text("objects");
      show_entities_list_panel();

      if (m_selected.size() == 1) {
        ImGui::Separator();
        ImGui::Text("parameters");
        show_entity_parameters_panel(m_selected[0]);
      }

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Other")) {
      ImGuiIO& io = ImGui::GetIO();
      ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate,
                  io.Framerate);
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }

  ImGui::End();
  ImGui::Render();
  mge::UIManager::end_frame();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UILayer::handle_event(mge::Event& event, float dt) {
  mge::Event::try_handler<SelectEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_selected));
  mge::Event::try_handler<AnnounceNewEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_announce_new));
  mge::Event::try_handler<DrawMassCenterEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_draw_mass_center_event));

  mge::Event::try_handler<DeleteTagEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_delete_tag));
  mge::Event::try_handler<MoveEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_move_event));
  mge::Event::try_handler<RotateEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_rotate_event));
  mge::Event::try_handler<ScaleEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_scale_event));
}

void UILayer::show_tag_panel(mge::TagComponent& component) {
  std::string tag = component.get_tag();
  if (ImGui::InputText("##tag", &tag)) {
    auto node = m_entities.extract(component.get_tag());
    node.key() = tag;
    m_entities.insert(std::move(node));
    component.set_tag(tag);
  }
}

void UILayer::show_transform_panel(mge::TransformComponent& component) {
  // position
  ImGui::Text("position");
  glm::vec3 position = component.get_position();
  if (ImGui::InputFloat3("##position", reinterpret_cast<float*>(&position),
                         "%.2f")) {
    component.set_position(position);
  }

  // rotation
  glm::vec3 rotation = mge::eulerAngles(component.get_rotation());
  auto pi = glm::pi<float>();
  bool update_rotation = false;
  ImGui::Text("rotation");
  if (ImGui::SliderFloat("x", &rotation.x, -pi, pi, "%.2f")) {
    component.set_rotation(mge::quat(rotation));
  }
  if (ImGui::SliderFloat("y", &rotation.y, -pi, pi, "%.2f")) {
    component.set_rotation(mge::quat(rotation));
  }
  if (ImGui::SliderFloat("z", &rotation.z, -pi, pi, "%.2f")) {
    component.set_rotation(mge::quat(rotation));
  }

  // scale
  ImGui::Text("scale");
  glm::vec3 scale = component.get_scale();
  if (ImGui::InputFloat3("##scale", reinterpret_cast<float*>(&scale), "%.2f")) {
    component.set_scale(scale);
  }
}

bool UILayer::show_renderable_component(
    mge::RenderableComponent<GeometryVertex>& component) {
  bool updated = false;
  const char* combo_preview_value =
      to_string(component.get_render_mode()).c_str();
  if (ImGui::BeginCombo("##render_mode", combo_preview_value)) {
    for (int n = 0; n < RENDER_MODE_SIZE; n++) {
      const bool is_selected =
          (static_cast<int>(component.get_render_mode()) == 1 << n);
      if (ImGui::Selectable(
              to_string(static_cast<mge::RenderMode>(1 << n)).c_str(),
              is_selected)) {
        component.set_render_mode(static_cast<mge::RenderMode>(1 << n));
        updated = true;
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  return updated;
}

std::pair<bool, bool> UILayer::show_torus_panel(TorusComponent& component) {
  bool regenerate_geometry = false, regenerate_topology = false;
  ImGui::Text("inner radius");
  float inner_radius = component.get_inner_radius();
  if (ImGui::InputFloat("##inner", &inner_radius, 0.1f, 1.0f, "%.2f")) {
    inner_radius = std::clamp(inner_radius, 0.0f, component.get_outer_radius());
    component.set_inner_radius(inner_radius);
    regenerate_geometry = true;
  }

  ImGui::Text("outer radius");
  float outer_radius = component.get_outer_radius();
  if (ImGui::InputFloat("##outer", &outer_radius, 0.1f, 1.0f, "%.2f")) {
    outer_radius = std::max(outer_radius, component.get_inner_radius());
    component.set_outer_radius(outer_radius);
    regenerate_geometry = true;
  }

  ImGui::Text("horizontal density");
  int horizontal_density = component.get_horizontal_density();
  if (ImGui::InputInt("##horizontal", &horizontal_density, 1, 1)) {
    horizontal_density = std::clamp(horizontal_density, 3, 128);
    component.set_horizontal_density(horizontal_density);
    regenerate_geometry = regenerate_topology = true;
  }

  ImGui::Text("vertical density");
  int vertical_density = component.get_vertical_density();
  if (ImGui::InputInt("##vertical", &vertical_density, 1, 1)) {
    vertical_density = std::clamp(vertical_density, 3, 128);
    component.set_vertical_density(vertical_density);
    regenerate_geometry = regenerate_topology = true;
  }

  return {regenerate_geometry, regenerate_topology};
}

void UILayer::show_point_panel(PointComponent& component) {}

void UILayer::show_tools_panel() {
  static int item_current_idx = 0;
  const char* combo_preview_value =
      to_string(static_cast<InputState>(1 << item_current_idx)).c_str();
  if (ImGui::BeginCombo("##modes", combo_preview_value)) {
    for (int n = 0; n < INPUT_STATE_SIZE; n++) {
      const bool is_selected = (item_current_idx == n);
      if (ImGui::Selectable(to_string(static_cast<InputState>(1 << n)).c_str(),
                            is_selected)) {
        item_current_idx = n;
        InputStateChangedEvent event(static_cast<InputState>(1 << n));
        m_send_event(event);
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  ImGui::Text("add object:");
  if (ImGui::Button("point")) {
    AddPointEvent point_event;
    m_send_event(point_event);
  }

  if (ImGui::Button("torus")) {
    AddTorusEvent torus_event;
    m_send_event(torus_event);
  }
}

void UILayer::show_entities_list_panel() {
  for (auto& [tag, is_selected] : m_entities) {
    if (ImGui::Selectable(tag.c_str(), is_selected)) {
      if (!ImGui::GetIO().KeyCtrl) {
        unselect_all();
      }
      m_entities[tag] ^= 1;
      if (m_entities[tag]) {
        QuerySelectedTagEvent event(tag);
        m_send_event(event);
      } else {
        for (auto& entity : m_selected) {
          auto& tag_component = entity.get().get_component<mge::TagComponent>();
          if (tag == tag_component.get_tag()) {
            unselect(entity.get());
          }
        }
      }
    }
  }
}

void UILayer::show_entity_parameters_panel(mge::Entity& entity) {
  if (entity.has_component<mge::TagComponent>()) {
    auto& tag_component = entity.get_component<mge::TagComponent>();
    show_tag_panel(tag_component);
  }

  if (entity.has_component<mge::TransformComponent>()) {
    auto& transform_component = entity.get_component<mge::TransformComponent>();
    show_transform_panel(transform_component);
  }

  if (entity.has_component<TorusComponent>()) {
    auto& torus_component = entity.get_component<TorusComponent>();
    auto [regenerate_geometry, regenerate_topology] =
        show_torus_panel(torus_component);

    if (entity.has_component<mge::RenderableComponent<GeometryVertex>>() &&
        regenerate_geometry) {
      auto& renderable_component =
          entity.get_component<mge::RenderableComponent<GeometryVertex>>();
      renderable_component.get_vertex_array().update_vertices(
          torus_component.generate_geometry());
      if (regenerate_topology) {
        if (renderable_component.get_render_mode() ==
            mge::RenderMode::WIREFRAME) {
          renderable_component.get_vertex_array().update_indices(
              torus_component.generate_topology<mge::RenderMode::WIREFRAME>());
        } else if (renderable_component.get_render_mode() ==
                   mge::RenderMode::SURFACE) {
          renderable_component.get_vertex_array().update_indices(
              torus_component.generate_topology<mge::RenderMode::SURFACE>());
        }
      }
    }
  }
  //   else if (entity.has_component<PointComponent>()) {
  //     auto& point_component = entity.get_component<PointComponent>();
  //     show_point_panel(point_component);
  //   }

  if (entity.has_component<mge::RenderableComponent<GeometryVertex>>()) {
    auto& renderable_component =
        entity.get_component<mge::RenderableComponent<GeometryVertex>>();
    if (show_renderable_component(renderable_component) &&
        entity.has_component<TorusComponent>()) {
      auto& torus_component = entity.get_component<TorusComponent>();
      if (renderable_component.get_render_mode() ==
          mge::RenderMode::WIREFRAME) {
        renderable_component.get_vertex_array().update_indices(
            torus_component.generate_topology<mge::RenderMode::WIREFRAME>());
      } else if (renderable_component.get_render_mode() ==
                 mge::RenderMode::SURFACE) {
        renderable_component.get_vertex_array().update_indices(
            torus_component.generate_topology<mge::RenderMode::SURFACE>());
      }
    }
  }
}

bool UILayer::on_selected(SelectEvent& event) {
  if (!event.get_selected()) {
    unselect_all();
  } else {
    select(event.get_selected().value());
  }

  return true;
}

bool UILayer::on_announce_new(AnnounceNewEvent& event) {
  m_entities.emplace(event.get_tag(), false);
  return true;
}

bool UILayer::on_delete_tag(DeleteTagEvent& event) {
  if (!m_selected.empty()) {
    m_selected.erase(std::remove_if(
        m_selected.begin(), m_selected.end(), [&event](mge::Entity& e) {
          return e.get_component<mge::TagComponent>().get_tag() ==
                 event.get_tag();
        }));
  }

  m_entities.erase(event.get_tag());
  return true;
}

bool UILayer::on_move_event(MoveEvent& event) {
  event.set_selected(m_selected);
  return false;
}

bool UILayer::on_rotate_event(RotateEvent& event) {
  event.set_selected(m_selected);
  return false;
}

bool UILayer::on_scale_event(ScaleEvent& event) {
  event.set_selected(m_selected);
  return false;
}

void UILayer::select(mge::Entity& entity) {
  auto& tag_component = entity.get_component<mge::TagComponent>();
  m_entities[tag_component] = true;
  if (entity.has_component<mge::RenderableComponent<GeometryVertex>>()) {
    entity.get_component<mge::RenderableComponent<GeometryVertex>>().set_color(
        {1.0f, 0.5f, 0.0f});
  }
  m_selected.push_back(entity);
}

void UILayer::unselect(mge::Entity& entity) {
  auto& tag_component = entity.get_component<mge::TagComponent>();
  m_entities[tag_component] = false;
  if (entity.has_component<mge::RenderableComponent<GeometryVertex>>()) {
    entity.get_component<mge::RenderableComponent<GeometryVertex>>().set_color(
        {0.0f, 0.0f, 0.0f});
  }
  auto tag = tag_component.get_tag();
  m_selected.erase(std::remove_if(
      m_selected.begin(), m_selected.end(), [&tag](mge::Entity& e) {
        return e.get_component<mge::TagComponent>().get_tag() == tag;
      }));
}

void UILayer::unselect_all() {
  for (auto& entity : m_selected) {
    auto& tag_component = entity.get().get_component<mge::TagComponent>();
    m_entities[tag_component] = false;
    if (entity.get()
            .has_component<mge::RenderableComponent<GeometryVertex>>()) {
      entity.get()
          .get_component<mge::RenderableComponent<GeometryVertex>>()
          .set_color({0.0f, 0.0f, 0.0f});
    }
  }
  m_selected.clear();
}

bool UILayer::on_draw_mass_center_event(DrawMassCenterEvent& event) {
  if (m_selected.size() < 2) {
    event.set_mass_center(std::nullopt);
    return true;
  }

  glm::vec3 center = {0.0f, 0.0f, 0.0f};
  for (auto& entity : m_selected) {
    auto& transform_component =
        entity.get().get_component<mge::TransformComponent>();
    center += transform_component.get_position();
  }
  center /= static_cast<float>(m_selected.size());
  event.set_mass_center(center);

  return true;
}