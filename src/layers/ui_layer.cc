#include "ui_layer.hh"
#include "../events/events.hh"
#include "../input_state.hh"
#include "cad_layer.hh"

UILayer::UILayer() {}

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

      if (m_displayed_entity.has_value()) {
        ImGui::Separator();
        ImGui::Text("parameters");
        show_entity_parameters_panel(m_displayed_entity->get());
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
  mge::Event::try_handler<NewEntityEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_new_entity));
  mge::Event::try_handler<RemoveEntityEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_removed_entity));
  mge::Event::try_handler<SelectEntityByTagEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_select_entity_by_tag));
  mge::Event::try_handler<UnSelectAllEntitiesEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_unselect_all_entities));
}

void UILayer::show_tag_panel(const mge::Entity& entity) {
  auto& component = entity.get_component<mge::TagComponent>();
  std::string tag = component.get_tag();
  if (ImGui::InputText("##tag", &tag)) {
    RenameEvent event(component.get_tag(), tag);
    m_send_event(event);
  }
}

void UILayer::show_transform_panel(const mge::Entity& entity) {
  auto& component = entity.get_component<mge::TransformComponent>();
  // position
  ImGui::Text("position");
  glm::vec3 position = component.get_position();
  if (ImGui::InputFloat3("##position", reinterpret_cast<float*>(&position),
                         "%.2f")) {
    MoveByUIEvent event(position);
    m_send_event(event);
  }

  // rotation
  glm::vec3 rotation = mge::eulerAngles(component.get_rotation());
  auto pi = glm::pi<float>();
  bool update_rotation = false;
  ImGui::Text("rotation");
  if (ImGui::SliderFloat("x", &rotation.x, -pi, pi, "%.2f")) {
    RotateByUIEvent event(
        mge::angleAxis(rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)));
    m_send_event(event);
  }
  if (ImGui::SliderFloat("y", &rotation.y, -pi, pi, "%.2f")) {
    RotateByUIEvent event(
        mge::angleAxis(rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_send_event(event);
  }
  if (ImGui::SliderFloat("z", &rotation.z, -pi, pi, "%.2f")) {
    RotateByUIEvent event(
        mge::angleAxis(rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)));
    m_send_event(event);
  }

  // scale
  ImGui::Text("scale");
  glm::vec3 scale = component.get_scale();
  if (ImGui::InputFloat3("##scale", reinterpret_cast<float*>(&scale), "%.2f")) {
    ScaleByUIEvent event(scale);
    m_send_event(event);
  }
}

void UILayer::show_limited_transform_panel(const mge::Entity& entity) {
  auto& component = entity.get_component<mge::TransformComponent>();
  // position
  ImGui::Text("position");
  glm::vec3 position = component.get_position();
  if (ImGui::InputFloat3("##position", reinterpret_cast<float*>(&position),
                         "%.2f")) {
    MoveByUIEvent event(position);
    m_send_event(event);
  }
}

void UILayer::show_renderable_component(const mge::Entity& entity) {
  auto& component =
      entity.get_component<mge::RenderableComponent<GeometryVertex>>();
  std::string selected_value = to_string(component.get_render_mode());
  if (ImGui::BeginCombo("##render_mode", selected_value.c_str())) {
    for (int n = 0; n < RENDER_MODE_SIZE; n++) {
      const bool is_selected =
          (static_cast<int>(component.get_render_mode()) == 1 << n);
      if (ImGui::Selectable(
              to_string(static_cast<mge::RenderMode>(1 << n)).c_str(),
              is_selected)) {
        mge::RenderModeUpdatedEvent event(
            entity.get_component<mge::TagComponent>(),
            static_cast<mge::RenderMode>(1 << n));
        m_send_event(event);
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

void UILayer::show_torus_panel(const mge::Entity& entity) {
  auto& component = entity.get_component<TorusComponent>();
  ImGui::Text("inner radius");
  float inner_radius = component.get_inner_radius();
  if (ImGui::InputFloat("##inner", &inner_radius, 0.1f, 1.0f, "%.2f")) {
    inner_radius = std::clamp(inner_radius, 0.0f, component.get_outer_radius());
    TorusUpdatedEvent event(entity.get_component<mge::TagComponent>(),
                            inner_radius, component.get_outer_radius(),
                            component.get_horizontal_density(),
                            component.get_vertical_density());
    m_send_event(event);
  }

  ImGui::Text("outer radius");
  float outer_radius = component.get_outer_radius();
  if (ImGui::InputFloat("##outer", &outer_radius, 0.1f, 1.0f, "%.2f")) {
    outer_radius = std::max(outer_radius, component.get_inner_radius());
    TorusUpdatedEvent event(entity.get_component<mge::TagComponent>(),
                            component.get_inner_radius(), outer_radius,
                            component.get_horizontal_density(),
                            component.get_vertical_density());
    m_send_event(event);
  }

  ImGui::Text("horizontal density");
  int horizontal_density = component.get_horizontal_density();
  if (ImGui::InputInt("##horizontal", &horizontal_density, 1, 1)) {
    horizontal_density = std::clamp(horizontal_density, 3, 128);
    TorusUpdatedEvent event(entity.get_component<mge::TagComponent>(),
                            component.get_inner_radius(),
                            component.get_outer_radius(), horizontal_density,
                            component.get_vertical_density());
    m_send_event(event);
  }

  ImGui::Text("vertical density");
  int vertical_density = component.get_vertical_density();
  if (ImGui::InputInt("##vertical", &vertical_density, 1, 1)) {
    vertical_density = std::clamp(vertical_density, 3, 128);
    TorusUpdatedEvent event(
        entity.get_component<mge::TagComponent>(), component.get_inner_radius(),
        component.get_outer_radius(), component.get_horizontal_density(),
        vertical_density);
    m_send_event(event);
  }
}

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

  if (ImGui::Button("bezier")) {
    AddBezierEvent bezier_event;
    m_send_event(bezier_event);
  }
}

void UILayer::show_entities_list_panel() {
  for (auto& [tag, selected] : m_entities) {
    if (ImGui::Selectable(tag.c_str(), selected)) {
      if (!ImGui::GetIO().KeyCtrl) {
        for (auto& [_, selected] : m_entities) {
          selected = false;
        }
        UnSelectAllEntitiesEvent event;
        m_send_event(event);
      }
      selected = !selected;
      if (selected) {
        SelectEntityByTagEvent event(tag);
        m_send_event(event);
      } else {
        UnSelectEntityByTagEvent event(tag);
        m_send_event(event);
      }
    }
  }
}

void UILayer::show_entity_parameters_panel(const mge::Entity& entity) {
  if (entity.has_component<mge::TagComponent>()) {
    show_tag_panel(entity);
  }

  if (entity.has_component<mge::TransformComponent>()) {
    if (entity.has_component<PointComponent>()) {
      show_limited_transform_panel(entity);
    } else {
      show_transform_panel(entity);
    }
  }

  if (entity.has_component<TorusComponent>()) {
    show_torus_panel(entity);
  }

  if (entity.has_component<mge::RenderableComponent<GeometryVertex>>()) {
    show_renderable_component(entity);
  }
}

bool UILayer::on_new_entity(NewEntityEvent& event) {
  m_entities.emplace(event.get_tag(), false);
  return true;
}

bool UILayer::on_removed_entity(RemoveEntityEvent& event) {
  m_entities.erase(event.get_tag());
  return true;
}

bool UILayer::on_select_entity_by_tag(SelectEntityByTagEvent event) {
  m_entities.at(event.get_tag()) = true;
  return false;
}

bool UILayer::on_unselect_all_entities(UnSelectAllEntitiesEvent event) {
  for (auto& [_, selected] : m_entities) {
    selected = false;
  }
  m_displayed_entity = std::nullopt;
  return false;
}