#include "ui_layer.hh"
#include "../components/bezier_component.hh"
#include "../components/selectible_component.hh"
#include "../events/events.hh"
#include "../input_state.hh"

RotationAxis::RotationAxis(RotationAxis::Type type) : m_type(type) {}

std::string RotationAxis::get_name(RotationAxis::Type type) {
  switch (type) {
    case RotationAxis::Type::X:
      return "X";
    case RotationAxis::Type::Y:
      return "Y";
    case RotationAxis::Type::Z:
      return "Z";
  }
  return "invalid";
}

glm::vec3 RotationAxis::get_value() const {
  switch (m_type) {
    case RotationAxis::Type::X:
      return {1.0f, 0.0f, 0.0f};
    case RotationAxis::Type::Y:
      return {0.0f, 1.0f, 0.0f};
    case RotationAxis::Type::Z:
      return {0.0f, 0.0f, 1.0f};
  }
  return glm::vec3{std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                   std::numeric_limits<float>::quiet_NaN()};
}

RotationAxis::Type RotationAxis::get_type() const { return m_type; }

void RotationAxis::set_type(RotationAxis::Type type) { m_type = type; }

ToolManager::ToolManager(ToolManager::Type current_type, ToolManager::Type previous_type)
    : m_current_tool(current_type), m_previous_tool(previous_type) {}

std::string ToolManager::get_name(ToolManager::Type type) {
  switch (type) {
    case ToolManager::Type::Select:
      return "Select";
    case ToolManager::Type::Delete:
      return "Delete";
    case ToolManager::Type::Move:
      return "Move";
    case ToolManager::Type::Scale:
      return "Scale";
    case ToolManager::Type::Rotate:
      return "Rotate";
    case ToolManager::Type::AddBezierPoint:
      return "AddBezierPoint";
    case ToolManager::Type::RemoveBezierPoint:
      return "RemoveBezierPoint";
  }

  return "invalid";
}

ToolManager::Type ToolManager::get_type() const { return m_current_tool; }

void ToolManager::set_type(Type type) { m_current_tool = type; }

void ToolManager::set_type_and_update_previous(Type type) {
  m_previous_tool = m_current_tool;
  m_current_tool = type;
}

void ToolManager::restore_type() { m_current_tool = m_previous_tool; }

SelectionManager::SelectionManager() : m_displayed_entity(std::nullopt), m_selected_count(0) {}

void SelectionManager::select(mge::EntityId id, bool is_parent) {
  if (m_entities.at(id).is_selected) return;
  m_entities.at(id).is_selected = true;
  m_entities.at(id).is_parent = is_parent;
  m_selected_count++;
  if (is_parent) m_parent_count++;
  m_selected_entities.push_back(id);
  mge::QueryEntityByIdEvent query_event(id);
  SendEngineEvent(query_event);
  query_event.entity.value().get().propagate([this](auto& entity) { select(entity.get_id(), false); });
  SelectionUpdateEvent selection_event(id, true, is_parent);
  SendEvent(selection_event);
  if (is_parent) {
    if (m_parent_count == 1) {
      m_displayed_entity = query_event.entity;
    } else {
      m_displayed_entity = std::nullopt;
    }
  }
}

void SelectionManager::unselect(mge::EntityId id) {
  if (!m_entities.at(id).is_selected) return;
  m_entities.at(id).is_selected = false;
  bool is_parent = m_entities.at(id).is_parent;
  m_entities.at(id).is_parent = true;
  m_selected_count--;
  if (is_parent) m_parent_count--;
  mge::QueryEntityByIdEvent query_event(id);
  SendEngineEvent(query_event);
  query_event.entity.value().get().propagate([this](auto& entity) { unselect(entity.get_id()); });
  SelectionUpdateEvent selection_event(id, false, is_parent);
  SendEvent(selection_event);
}

void SelectionManager::unselect_all() {
  for (auto& [id, data] : m_entities) {
    data.is_selected = false;
    data.is_parent = true;
  }
  m_selected_count = 0;
  m_parent_count = 0;
  m_displayed_entity = std::nullopt;
  m_selected_entities.clear();
  UnselectAllEntitiesEvent event;
  SendEvent(event);
}

mge::OptionalEntity SelectionManager::get_displayed_entity() const { return m_displayed_entity; }

bool SelectionManager::is_selected(mge::EntityId id) const { return m_entities.at(id).is_selected; }

const std::string& SelectionManager::get_tag(mge::EntityId id) const { return m_entities.at(id).tag; }

unsigned int SelectionManager::get_selected_count() const { return m_selected_count; }

bool SelectionManager::contains(mge::EntityId id) const { return m_entities.contains(id); }

bool SelectionManager::is_dirty() const { return m_selected_entities.size() != m_selected_count; }

std::vector<mge::EntityId> SelectionManager::get_all_entities_ids() {
  auto kv = std::views::keys(m_entities);
  return {kv.begin(), kv.end()};
}

std::vector<mge::EntityId> SelectionManager::get_selected_ids() { return m_selected_entities; }

bool SelectionManager::add_entity(mge::EntityId id, const std::string& tag) {
  if (contains(id)) return false;

  m_entities.emplace(id, EntityMapNode{.tag = tag, .is_selected = false});

  return true;
}

bool SelectionManager::remove_entity(mge::EntityId id) {
  if (!contains(id)) return false;

  unselect(id);
  m_entities.erase(id);

  return true;
}

bool SelectionManager::rename_entity(mge::EntityId id, const std::string& tag) {
  if (!contains(id)) return false;

  auto node = m_entities.extract(id);
  node.mapped().tag = tag;
  m_entities.insert(std::move(node));

  return true;
}

void SelectionManager::validate_selected() {
  mge::vector_remove_if<mge::EntityId>(m_selected_entities, [&entities = m_entities](auto& id) {
    return !entities.contains(id) || !entities.at(id).is_selected;
  });
  if (m_parent_count == 1) {
    for (auto id : m_selected_entities) {
      if (m_entities.at(id).is_parent) {
        mge::QueryEntityByIdEvent query_event(m_selected_entities.front());
        SendEngineEvent(query_event);
        m_displayed_entity = query_event.entity;
        break;
      }
    }
  } else {
    m_displayed_entity = std::nullopt;
  }
}

UILayer::UILayer(mge::Entity& mass_center)
    : m_tool_manager(), m_disable_tools_combo(false), m_rotation_axis(), m_selection_manager() {}

void UILayer::configure() {
  mge::AddEventListener(mge::EntityEvents::Added, UILayer::on_added_entity, this);
  mge::AddEventListener(mge::EntityEvents::Deleted, UILayer::on_deleted_entity, this);
  mge::AddEventListener(mge::MouseEvents::MouseButtonUpdated, UILayer::on_mouse_button_pressed, this);
  mge::AddEventListener(mge::MouseEvents::MouseMoved, UILayer::on_mouse_moved, this);
  mge::AddEventListener(mge::MouseEvents::MouseScroll, UILayer::on_mouse_scroll, this);
}

void UILayer::update() {
  if (m_selection_manager.is_dirty()) m_selection_manager.validate_selected();

  mge::UIManager::start_frame();

  //   bool dwnd = true;
  //   ImGui::ShowDemoWindow(&dwnd);

  auto size = ImGui::GetMainViewport()->Size;

  ImGui::SetNextWindowSize({std::min(size.x * 0.25f, 250.0f), size.y});
  ImGui::SetNextWindowPos({std::max(size.x * 0.75f, size.x - 250.f), 0});
  ImGui::Begin("ToolsParams", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

  if (ImGui::BeginTabBar("MainTabBar")) {
    if (ImGui::BeginTabItem("Main")) {
      ImGui::Text("tools");
      show_tools_panel();
      ImGui::Separator();

      ImGui::Text("objects");
      show_entities_list_panel();

      if (m_selection_manager.get_displayed_entity().has_value()) {
        ImGui::Separator();
        ImGui::Text("parameters");
        show_entity_parameters_panel(m_selection_manager.get_displayed_entity()->get());
      }

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Other")) {
      ImGuiIO& io = ImGui::GetIO();
      ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }

  ImGui::End();
  ImGui::Render();
  mge::UIManager::end_frame();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UILayer::show_tag_panel(const mge::Entity& entity) {
  auto& component = entity.get_component<mge::TagComponent>();
  std::string tag = component.get_tag();
  if (ImGui::InputText("##tag", &tag)) {
    mge::TagUpdateEvent event(entity.get_id(), tag);
    SendEngineEvent(event);
    if (event.is_handled()) {
      m_selection_manager.rename_entity(entity.get_id(), tag);
    }
  }
}

void UILayer::show_transform_panel(const mge::Entity& entity) {
  auto& component = entity.get_component<mge::TransformComponent>();
  // position
  ImGui::Text("position");
  glm::vec3 position = component.get_position();
  if (ImGui::InputFloat3("##position", reinterpret_cast<float*>(&position), "%.2f")) {
    TranslateEvent event(entity.get_id(), position);
    SendEvent(event);
  }

  // rotation
  glm::vec3 rotation = glm::eulerAngles(component.get_rotation());
  auto pi = glm::pi<float>();
  bool update_rotation = false;
  ImGui::Text("rotation");
  if (ImGui::SliderFloat("x", &rotation.x, -pi, pi, "%.2f")) {
    RotateEvent event(entity.get_id(), glm::angleAxis(rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)));
    SendEvent(event);
  }
  if (ImGui::SliderFloat("y", &rotation.y, -pi, pi, "%.2f")) {
    RotateEvent event(entity.get_id(), glm::angleAxis(rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)));
    SendEvent(event);
  }
  if (ImGui::SliderFloat("z", &rotation.z, -pi, pi, "%.2f")) {
    RotateEvent event(entity.get_id(), glm::angleAxis(rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)));
    SendEvent(event);
  }

  // scale
  ImGui::Text("scale");
  glm::vec3 scale = component.get_scale();
  if (ImGui::InputFloat3("##scale", reinterpret_cast<float*>(&scale), "%.2f")) {
    ScaleEvent event(entity.get_id(), scale);
    SendEvent(event);
  }
}

void UILayer::show_limited_transform_panel(const mge::Entity& entity) {
  auto& component = entity.get_component<mge::TransformComponent>();
  // position
  ImGui::Text("position");
  glm::vec3 position = component.get_position();
  if (ImGui::InputFloat3("##position", reinterpret_cast<float*>(&position), "%.2f")) {
    TranslateEvent event(entity.get_id(), position);
    SendEvent(event);
  }
}

void UILayer::show_renderable_component(const mge::Entity& entity) {
  auto& component = entity.get_component<mge::RenderableComponent<GeometryVertex>>();
  std::string selected_value = to_string(component.get_render_mode());
  if (ImGui::BeginCombo("##render_mode", selected_value.c_str())) {
    auto render_modes = component.get_registered_render_modes();
    for (auto render_mode : render_modes) {
      const bool is_selected = component.get_render_mode() == render_mode;
      if (ImGui::Selectable(to_string(render_mode).c_str(), is_selected)) {
        mge::RenderModeUpdatedEvent event(entity.get_id(), render_mode);
        SendEngineEvent(event);
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
    TorusRadiusUpdatedEvent event(entity.get_id(), inner_radius, component.get_outer_radius());
    SendEvent(event);
  }

  ImGui::Text("outer radius");
  float outer_radius = component.get_outer_radius();
  if (ImGui::InputFloat("##outer", &outer_radius, 0.1f, 1.0f, "%.2f")) {
    outer_radius = std::max(outer_radius, component.get_inner_radius());
    TorusRadiusUpdatedEvent event(entity.get_id(), component.get_inner_radius(), outer_radius);
    SendEvent(event);
  }

  ImGui::Text("horizontal density");
  int horizontal_density = component.get_horizontal_density();
  if (ImGui::InputInt("##horizontal", &horizontal_density, 1, 1)) {
    horizontal_density = std::clamp(horizontal_density, 3, 128);
    TorusGridDensityUpdatedEvent event(entity.get_id(), horizontal_density, component.get_vertical_density());
    SendEvent(event);
  }

  ImGui::Text("vertical density");
  int vertical_density = component.get_vertical_density();
  if (ImGui::InputInt("##vertical", &vertical_density, 1, 1)) {
    vertical_density = std::clamp(vertical_density, 3, 128);
    TorusGridDensityUpdatedEvent event(entity.get_id(), component.get_horizontal_density(), vertical_density);
    SendEvent(event);
  }
}

void UILayer::show_bezier_panel(const mge::Entity& entity) {
  ImGui::Text("Control points");
  auto add_button_func = [this]() {
    if (ImGui::Button("add")) {
      if (m_tool_manager.get_type() == ToolManager::Type::AddBezierPoint) {
        m_tool_manager.restore_type();
        m_disable_tools_combo = false;
      } else {
        if (m_tool_manager.get_type() != ToolManager::Type::RemoveBezierPoint) {
          m_tool_manager.set_type_and_update_previous(ToolManager::Type::AddBezierPoint);
        } else {
          m_tool_manager.set_type(ToolManager::Type::AddBezierPoint);
        }
        m_disable_tools_combo = true;
      }
    }
  };
  if (m_tool_manager.get_type() == ToolManager::Type::AddBezierPoint) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 1.0, 0.0, 1.0));
    add_button_func();
    ImGui::PopStyleColor();
  } else {
    add_button_func();
  }

  auto remove_button_func = [this]() {
    ImGui::SameLine();
    if (ImGui::Button("remove")) {
      if (m_tool_manager.get_type() == ToolManager::Type::RemoveBezierPoint) {
        m_tool_manager.restore_type();
        m_disable_tools_combo = false;
      } else {
        if (m_tool_manager.get_type() != ToolManager::Type::AddBezierPoint) {
          m_tool_manager.set_type_and_update_previous(ToolManager::Type::RemoveBezierPoint);
        } else {
          m_tool_manager.set_type(ToolManager::Type::RemoveBezierPoint);
        }
        m_disable_tools_combo = true;
      }
    }
  };
  if (m_tool_manager.get_type() == ToolManager::Type::RemoveBezierPoint) {
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
      const bool is_selected =
          component.get_bezier_polygon_status() && !n || !component.get_bezier_polygon_status() && n;
      if (ImGui::Selectable(options[n].c_str(), is_selected)) {
        selected = n;
        BezierUpdateBerensteinPolygonStateEvent polygon_event(entity.get_id(), !n);
        SendEvent(polygon_event);
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

void UILayer::show_tools_panel() {
  using Tool = ToolManager::Type;
  static std::vector<Tool> displayable_tools = {Tool::Select, Tool::Delete, Tool::Move, Tool::Scale, Tool::Rotate};
  std::string selected_value = ToolManager::get_name(m_tool_manager.get_type());
  ImGui::BeginDisabled(m_disable_tools_combo);
  if (ImGui::BeginCombo("##tools", selected_value.c_str())) {
    for (auto tool : displayable_tools) {
      bool is_selected = m_tool_manager.get_type() == tool;
      if (ImGui::Selectable(ToolManager::get_name(tool).c_str(), is_selected)) {
        m_tool_manager.set_type_and_update_previous(tool);
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  ImGui::EndDisabled();

  if (m_tool_manager.get_type() == ToolManager::Type::Rotate) {
    for (auto& axis_type : std::vector{RotationAxis::Type::X, RotationAxis::Type::Y, RotationAxis::Type::Z}) {
      if (m_rotation_axis.get_type() == axis_type) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 1.0, 0.0, 1.0));
        if (ImGui::Button(RotationAxis::get_name(axis_type).c_str())) {
          m_rotation_axis.set_type(axis_type);
        }
        ImGui::PopStyleColor();
      } else {
        if (ImGui::Button(RotationAxis::get_name(axis_type).c_str())) {
          m_rotation_axis.set_type(axis_type);
        }
      }
      ImGui::SameLine();
    }
    ImGui::Text(" ");
  }

  ImGui::Text("add object:");
  if (ImGui::Button("point")) {
    AddPointEvent point_event;
    SendEvent(point_event);
  }
  ImGui::SameLine();
  if (ImGui::Button("torus")) {
    AddTorusEvent torus_event(4.0f, 6.0f, 8, 8);
    SendEvent(torus_event);
  }
  ImGui::SameLine();
  if (ImGui::Button("bezier") && m_selection_manager.get_selected_count() > 1) {
    AddBezierEvent bezier_event(m_selection_manager.get_selected_ids());
    SendEvent(bezier_event);
  }
}

void UILayer::show_entities_list_panel() {
  for (auto id : m_selection_manager.get_all_entities_ids()) {
    if (ImGui::Selectable(m_selection_manager.get_tag(id).c_str(), m_selection_manager.is_selected(id))) {
      if (m_selection_manager.get_displayed_entity().has_value() &&
          m_selection_manager.get_displayed_entity()->get().has_component<BezierComponent>()) {
        auto bezier_id = m_selection_manager.get_displayed_entity().value().get().get_id();
        if (m_selection_manager.is_selected(id) && m_tool_manager.get_type() == ToolManager::Type::RemoveBezierPoint) {
          m_selection_manager.unselect(id);
          BezierDeleteControlPointEvent event(bezier_id, id);
          SendEvent(event);
        } else if (!m_selection_manager.is_selected(id) &&
                   m_tool_manager.get_type() == ToolManager::Type::AddBezierPoint) {
          m_selection_manager.select(id, false);
          BezierAddControlPointEvent event(bezier_id, id);
          SendEvent(event);
        }
      } else {
        if (!ImGui::GetIO().KeyCtrl) {
          m_selection_manager.unselect_all();
        }
        if (m_selection_manager.is_selected(id)) {
          m_selection_manager.unselect(id);
        } else {
          m_selection_manager.select(id, true);
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

  if (entity.has_component<mge::RenderableComponent<GeometryVertex>>()) {
    show_renderable_component(entity);
  }

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0, 0.0, 0.0, 1.0));
  if (ImGui::Button("Remove", ImVec2(ImGui::GetWindowSize().x, 0.0f))) {
    mge::DeleteEntityEvent event(entity.get_id());
    SendEngineEvent(event);
    if (event.is_handled()) {
      m_selection_manager.remove_entity(event.id);
    }
  }
  ImGui::PopStyleColor();
}

bool UILayer::on_added_entity(mge::AddedEntityEvent& event) {
  mge::QueryEntityByIdEvent query_event(event.id);
  SendEngineEvent(query_event);
  MGE_ASSERT(query_event.entity.has_value(), "Queried entity doesn't exist: {}", static_cast<int>(event.id));
  auto& entity = query_event.entity.value().get();
  if (!entity.has_component<mge::TagComponent>() || !entity.has_component<SelectibleComponent>()) return false;
  m_selection_manager.add_entity(entity.get_id(), entity.get_component<mge::TagComponent>().get_tag());
  if (m_selection_manager.get_displayed_entity().has_value() &&
      m_selection_manager.get_displayed_entity()->get().has_component<BezierComponent>()) {
    BezierAddControlPointEvent add_event(m_selection_manager.get_displayed_entity()->get().get_id(), entity.get_id());
    SendEvent(add_event);
    m_selection_manager.select(entity.get_id(), false);
  }
  return true;
}

bool UILayer::on_deleted_entity(mge::DeletedEntityEvent& event) { return m_selection_manager.remove_entity(event.id); }

bool UILayer::on_mouse_moved(mge::MouseMovedEvent& event) {
  auto& window = event.source_window;
  if (window.is_key_pressed(mge::KeyboardKey::LeftShift)) {
    send_camera_move_events(event);
    return true;
  }

  if (window.is_key_pressed(mge::KeyboardKey::LeftControl)) {
    CursorMoveEvent cursor_event(event.end_position);
    SendEvent(cursor_event);
    if (m_tool_manager.get_type() != ToolManager::Type::Move) return true;
  }

  switch (m_tool_manager.get_type()) {
    case ToolManager::Type::Scale:
      if (window.is_mouse_pressed(mge::MouseButton::Left)) {
        RelativeScaleEvent scale_event(m_selection_manager.get_selected_ids(), event.start_position,
                                       event.end_position);
        SendEvent(scale_event);
      }
      break;
    case ToolManager::Type::Rotate:
      if (window.is_mouse_pressed(mge::MouseButton::Left)) {
        RelativeRotateEvent rotate_event(m_selection_manager.get_selected_ids(), event.start_position,
                                         event.end_position, m_rotation_axis.get_value());
        SendEvent(rotate_event);
      }
      break;
    case ToolManager::Type::Move:
      if (window.is_mouse_pressed(mge::MouseButton::Left)) {
        TranslateToCursorEvent translate_event(m_selection_manager.get_selected_ids());
        SendEvent(translate_event);
      }
      break;
    default:
      break;
  }
  return true;
}

bool UILayer::on_mouse_button_pressed(mge::MouseButtonUpdatedEvent& event) {
  if (event.state != mge::InputState::Press) return false;
  if (event.mods & mge::InputModifierKey::Shift) {
    return true;
  }

  switch (m_tool_manager.get_type()) {
    case ToolManager::Type::Select:
      if (event.button == mge::MouseButton::Left) {
        mge::QueryEntityByPositionEvent query_event(event.position);
        SendEngineEvent(query_event);
        if (query_event.entity.has_value()) {
          auto id = query_event.entity.value().get().get_id();
          if (m_selection_manager.is_selected(id)) {
            m_selection_manager.unselect(id);
          } else {
            m_selection_manager.select(id, true);
          }
        } else {
          m_selection_manager.unselect_all();
        }
      }
      break;
    case ToolManager::Type::Delete:
      if (event.button == mge::MouseButton::Left) {
        mge::QueryEntityByPositionEvent query_event(event.position);
        SendEngineEvent(query_event);
        if (query_event.entity.has_value()) {
          auto id = query_event.entity.value().get().get_id();
          mge::DeleteEntityEvent delete_event(id);
          SendEngineEvent(delete_event);
          if (delete_event.is_handled()) m_selection_manager.remove_entity(id);
        }
      }
      break;
    default:
      return false;
  }
  return true;
}

bool UILayer::on_mouse_scroll(mge::MouseScrollEvent& event) {
  send_camera_zoom_event(event);

  return true;
}

void UILayer::send_camera_move_events(mge::MouseMovedEvent& event) {
  auto& window = event.source_window;
  if (window.is_mouse_pressed(mge::MouseButton::Left)) {
    float sensitivity = 1.0f;
    mge::CameraAngleChangedEvent cam_event(sensitivity * event.get_offset().x, sensitivity * event.get_offset().y);
    SendEngineEvent(cam_event);
    return;
  }

  if (window.is_mouse_pressed(mge::MouseButton::Right)) {
    float sensitivity = 0.5f;
    mge::CameraPositionChangedEvent cam_event(
        glm::vec2(-sensitivity * event.get_offset().x, -sensitivity * event.get_offset().y));
    SendEngineEvent(cam_event);
    return;
  }

  if (window.is_mouse_pressed(mge::MouseButton::Middle)) {
    float sensitivity = 1.005f;
    mge::CameraZoomEvent cam_event(std::pow(sensitivity, event.get_offset().x));
    SendEngineEvent(cam_event);
    return;
  }
}

void UILayer::send_camera_zoom_event(mge::MouseScrollEvent& event) {
  float sensitivity = 1.1f;
  mge::CameraZoomEvent cam_event(std::pow(sensitivity, -event.y_offset));
  SendEngineEvent(cam_event);
}