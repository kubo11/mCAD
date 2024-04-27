#include "ui_layer.hh"
#include "../events/events.hh"
#include "../geometry/bezier_component.hh"
#include "../input_state.hh"
#include "cad_layer.hh"

UILayer::UILayer()
    : m_ui_mode(UIMode::NONE), m_input_state(InputState::SELECT) {}

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
  mge::Event::try_handler<SelectEntityByPositionEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_select_entity_by_position));
  mge::Event::try_handler<UnSelectAllEntitiesEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_unselect_all_entities));
  mge::Event::try_handler<UnSelectEntityByTagEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_unselect_entity_by_tag));
  mge::Event::try_handler<UpdateDisplayedEntityEvent>(
      event, BIND_EVENT_HANDLER(UILayer::on_update_displayed_entity));
}

void UILayer::show_tag_panel(const mge::Entity& entity) {
  auto& component = entity.get_component<mge::TagComponent>();
  std::string tag = component.get_tag();
  std::string old_tag = tag;
  if (ImGui::InputText("##tag", &tag)) {
    RenameEvent event(component.get_tag(), tag);
    m_send_event(event);
    if (event.get_status()) {
      auto node = m_entities.extract(old_tag);
      node.key() = tag;
      m_entities.insert(std::move(node));
    }
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

void UILayer::show_bezier_panel(const mge::Entity& entity) {
  static InputState prev_input_state = InputState::SELECT;
  ImGui::Text("Control points");
  auto add_button_func = [this]() {
    if (ImGui::Button("add")) {
      if (m_ui_mode == UIMode::NONE) {
        prev_input_state = m_input_state;
      }
      if (m_ui_mode != UIMode::ADD_BEZIER_POINT) {
        m_ui_mode = UIMode::ADD_BEZIER_POINT;
        m_input_state = InputState::SELECT;
      } else {
        m_ui_mode = UIMode::NONE;
        m_input_state = prev_input_state;
      }
      InputStateChangedEvent event(m_input_state);
      m_send_event(event);
    }
  };
  if (m_ui_mode == UIMode::ADD_BEZIER_POINT) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 1.0, 0.0, 1.0));
    add_button_func();
    ImGui::PopStyleColor();
  } else {
    add_button_func();
  }

  auto remove_button_func = [this]() {
    ImGui::SameLine();
    if (ImGui::Button("remove")) {
      if (m_ui_mode == UIMode::NONE) {
        prev_input_state = m_input_state;
      }
      if (m_ui_mode != UIMode::REMOVE_BEZIER_POINT) {
        m_ui_mode = UIMode::REMOVE_BEZIER_POINT;
        m_input_state = InputState::SELECT;
      } else {
        m_ui_mode = UIMode::NONE;
        m_input_state = prev_input_state;
      }
      InputStateChangedEvent event(m_input_state);
      m_send_event(event);
    }
  };
  if (m_ui_mode == UIMode::REMOVE_BEZIER_POINT) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 1.0, 0.0, 1.0));
    remove_button_func();
    ImGui::PopStyleColor();
  } else {
    remove_button_func();
  }

  ImGui::Text("Show berenstein polygon");
  auto& component = entity.get_component<BezierComponent>();
  std::array<std::string, 2> options = {"yes", "no"};
  static int selected = 1;
  if (ImGui::BeginCombo("##berenstein_polygon", options[selected].c_str())) {
    for (int n = 0; n < options.size(); n++) {
      const bool is_selected = component.get_bezier_polygon_status() && !n ||
                               !component.get_bezier_polygon_status() && n;
      if (ImGui::Selectable(options[n].c_str(), is_selected)) {
        selected = n;
        ShowBerensteinPolygonEvent event(
            entity.get_component<mge::TagComponent>(), !n);
        m_send_event(event);
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

void UILayer::show_tools_panel() {
  std::string selected_value = to_string(m_input_state);
  if (ImGui::BeginCombo("##modes", selected_value.c_str())) {
    for (int n = 0; n < INPUT_STATE_SIZE; n++) {
      const bool is_selected = (static_cast<int>(m_input_state) == (1 << n));
      if (ImGui::Selectable(to_string(static_cast<InputState>(1 << n)).c_str(),
                            is_selected)) {
        m_input_state = static_cast<InputState>(1 << n);
        InputStateChangedEvent event(m_input_state);
        m_send_event(event);
        m_ui_mode = UIMode::NONE;
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
  ImGui::SameLine();
  if (ImGui::Button("torus")) {
    AddTorusEvent torus_event;
    m_send_event(torus_event);
  }
  ImGui::SameLine();
  if (ImGui::Button("bezier")) {
    AddBezierEvent bezier_event;
    m_send_event(bezier_event);
  }
}

void UILayer::show_entities_list_panel() {
  for (auto& [tag, selected] : m_entities) {
    if (ImGui::Selectable(tag.c_str(), selected)) {
      if (m_displayed_entity.has_value() &&
          m_displayed_entity->get().has_component<BezierComponent>()) {
        auto& bezier_tag =
            m_displayed_entity->get().get_component<mge::TagComponent>();
        if (selected) {
          RemoveControlPointByTagEvent event(bezier_tag, tag);
          m_send_event(event);
        } else {
          AddControlPointByTagEvent event(bezier_tag, tag);
          m_send_event(event);
        }
      } else {
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

  if (entity.has_component<BezierComponent>()) {
    show_bezier_panel(entity);
  }

  if (entity.has_component<mge::RenderableComponent<GeometryVertex>>() &&
      entity.has_component<TorusComponent>()) {
    show_renderable_component(entity);
  }

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0, 0.0, 0.0, 1.0));
  if (ImGui::Button("Remove", ImVec2(ImGui::GetWindowSize().x, 0.0f))) {
    DeleteEntityByTagEvent event(entity.get_component<mge::TagComponent>());
    m_send_event(event);
  }
  ImGui::PopStyleColor();
}

bool UILayer::on_new_entity(NewEntityEvent& event) {
  m_entities.emplace(event.get_tag(), false);
  if (m_displayed_entity.has_value() &&
      m_displayed_entity->get().has_component<BezierComponent>()) {
    AddControlPointByTagEvent add_event(
        m_displayed_entity->get().get_component<mge::TagComponent>(),
        event.get_tag());
    m_send_event(add_event);
  }
  return true;
}

bool UILayer::on_removed_entity(RemoveEntityEvent& event) {
  m_entities.erase(event.get_tag());
  return true;
}

bool UILayer::on_select_entity_by_tag(SelectEntityByTagEvent& event) {
  m_entities.at(event.get_tag()) = true;
  return false;
}

bool UILayer::on_unselect_entity_by_tag(UnSelectEntityByTagEvent event) {
  m_entities.at(event.get_tag()) = false;
  return false;
}

bool UILayer::on_select_entity_by_position(SelectEntityByPositionEvent& event) {
  switch (m_ui_mode) {
    case UIMode::NONE:
      return false;
    case UIMode::ADD_BEZIER_POINT: {
      AddControlPointByPositionEvent add_event(
          m_displayed_entity.value().get().get_component<mge::TagComponent>(),
          event.get_position());
      m_send_event(add_event);
    }
      return true;
    case UIMode::REMOVE_BEZIER_POINT: {
      RemoveControlPointByPositionEvent remove_event(
          m_displayed_entity.value().get().get_component<mge::TagComponent>(),
          event.get_position());
      m_send_event(remove_event);
    }
      return true;
    default:
      return false;
  }
}

bool UILayer::on_unselect_all_entities(UnSelectAllEntitiesEvent event) {
  for (auto& [_, selected] : m_entities) {
    selected = false;
  }
  m_displayed_entity = std::nullopt;
  return false;
}

bool UILayer::on_update_displayed_entity(UpdateDisplayedEntityEvent& event) {
  m_displayed_entity = event.get_entity();
  return true;
}