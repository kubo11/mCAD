#include "ui_layer.hh"
#include "../events/events.hh"
#include "../input_state.hh"
#include "cad_layer.hh"

UILayer::UILayer(mge::Entity& cursor) : m_entities{} {}

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

      QuerySelectedEntityEvent event;
      m_send_event(event);
      if (event.get_entity().has_value()) {
        ImGui::Separator();
        ImGui::Text("parameters");
        show_entity_parameters_panel(event.get_entity().value());
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
  mge::Event::try_handler<AnnounceNewEntityEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_announce_new_entity));
  mge::Event::try_handler<SelectEntityByTagEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_select_entity_by_tag));
  mge::Event::try_handler<DeleteEntityByTagEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_delete_entity_by_tag));
  mge::Event::try_handler<UnSelectEntityByTagEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_unselect_entity_by_tag));
  mge::Event::try_handler<UnSelectAllEntitiesEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_unselect_all_entities));
}

void UILayer::show_tag_panel(mge::TagComponent& component) {
  std::string tag = component.get_tag();
  if (ImGui::InputText("##tag", &tag)) {
    RenameEvent event(component.get_tag(), tag);
    m_send_event(event);
    if (event.get_status()) {
      auto node = m_entities.extract(component.get_tag());
      node.key() = tag;
      m_entities.insert(std::move(node));
      component.set_tag(tag);
    }
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

void UILayer::show_limited_transform_panel(mge::TransformComponent& component) {
  // position
  ImGui::Text("position");
  glm::vec3 position = component.get_position();
  if (ImGui::InputFloat3("##position", reinterpret_cast<float*>(&position),
                         "%.2f")) {
    component.set_position(position);
  }
}

bool UILayer::show_renderable_component(
    mge::RenderableComponent<GeometryVertex>& component) {
  bool update = false;
  std::string selected_value = to_string(component.get_render_mode());
  if (ImGui::BeginCombo("##render_mode", selected_value.c_str())) {
    for (int n = 0; n < RENDER_MODE_SIZE; n++) {
      const bool is_selected =
          (static_cast<int>(component.get_render_mode()) == 1 << n);
      if (ImGui::Selectable(
              to_string(static_cast<mge::RenderMode>(1 << n)).c_str(),
              is_selected)) {
        update = true;
        component.set_render_mode(static_cast<mge::RenderMode>(1 << n));
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  return update;
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
  static int selected_idx = 0;
  std::string selected_value =
      to_string(static_cast<InputState>(1 << selected_idx));
  if (ImGui::BeginCombo("##modes", selected_value.c_str())) {
    for (int n = 0; n < INPUT_STATE_SIZE; n++) {
      const bool is_selected = (selected_idx == n);
      if (ImGui::Selectable(to_string(static_cast<InputState>(1 << n)).c_str(),
                            is_selected)) {
        selected_idx = n;
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
        UnSelectAllEntitiesEvent event;
        m_send_event(event);
        for (auto& [_, selection] : m_entities) {
          selection = false;
        }
      }
      m_entities[tag] ^= 1;
      if (m_entities[tag]) {
        SelectEntityByTagEvent event(tag);
        m_send_event(event);
      } else {
        UnSelectEntityByTagEvent event(tag);
        m_send_event(event);
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
    if (entity.has_component<PointComponent>()) {
      show_limited_transform_panel(transform_component);
    } else {
      show_transform_panel(transform_component);
    }
  }

  if (entity.has_component<TorusComponent>()) {
    auto& torus_component = entity.get_component<TorusComponent>();
    auto& tag_component = entity.get_component<mge::TagComponent>();
    auto p = show_torus_panel(torus_component);
    if (p.first || p.second) {
      TorusUpdateGeometryEvent geometry_event(tag_component.get_tag());
      m_send_event(geometry_event);
    }
    if (p.second) {
      TorusUpdateTopologyEvent topology_event(tag_component.get_tag());
      m_send_event(topology_event);
    }
  } else if (entity.has_component<PointComponent>()) {
    auto& point_component = entity.get_component<PointComponent>();
    show_point_panel(point_component);
  }

  if (entity.has_component<mge::RenderableComponent<GeometryVertex>>()) {
    auto& renderable_component =
        entity.get_component<mge::RenderableComponent<GeometryVertex>>();
    auto& tag_component = entity.get_component<mge::TagComponent>();
    auto p = show_renderable_component(renderable_component);
    if (p) {
      TorusUpdateTopologyEvent topology_event(tag_component.get_tag());
      m_send_event(topology_event);
    }
  }
}

bool UILayer::on_announce_new_entity(AnnounceNewEntityEvent& event) {
  m_entities.emplace(event.get_tag(), false);
  return true;
}

bool UILayer::on_select_entity_by_tag(SelectEntityByTagEvent& event) {
  m_entities.at(event.get_tag()) = true;
  return false;
}

bool UILayer::on_delete_entity_by_tag(DeleteEntityByTagEvent& event) {
  m_entities.erase(event.get_tag());
  return true;
}

bool UILayer::on_unselect_entity_by_tag(UnSelectEntityByTagEvent& event) {
  m_entities.at(event.get_tag()) = false;
  return false;
}

bool UILayer::on_unselect_all_entities(UnSelectAllEntitiesEvent& event) {
  for (auto& [_, selected] : m_entities) {
    selected = false;
  }
  return false;
}