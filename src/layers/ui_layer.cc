#include "ui_layer.hh"

#include <nfd.hpp>
#include <nfd_glfw3.h>

#include "../components/bezier_curve_c0_component.hh"
#include "../components/bezier_curve_c2_component.hh"
#include "../components/bezier_curve_c2_interp_component.hh"
#include "../components/bezier_surface_c0_component.hh"
#include "../components/bezier_surface_c2_component.hh"
#include "../components/selectible_component.hh"
#include "../components/mass_center_component.hh"
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
  if (m_virtual_entities.contains(id)) {
    select_virtual(id, is_parent);
    return;
  }
  if (m_entities.at(id).is_selected) {
    if (m_entities.at(id).is_parent && !is_parent) {
      m_entities.at(id).is_parent = false;
      m_parent_count--;
      mge::vector_remove(m_selected_entities, id);
      DegradeSelectionEvent event(id);
      SendEvent(event);
    }
    return;
  }
  m_entities.at(id).is_selected = true;
  m_entities.at(id).is_parent = is_parent;
  m_selected_count++;
  if (is_parent) {
    m_parent_count++;
    m_selected_entities.push_back(id);
  }
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

void SelectionManager::select_virtual(mge::EntityId id, bool is_parent) {
  if (m_virtual_entities.at(id).is_selected) return;
  m_virtual_entities.at(id).is_selected = true;
  m_virtual_entities.at(id).is_parent = is_parent;
  m_selected_count++;
  if (is_parent) {
    m_parent_count++;
    m_selected_entities.push_back(id);
  }
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
  if (m_virtual_entities.contains(id)) {
    unselect_virtual(id);
    return;
  }
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

void SelectionManager::unselect_virtual(mge::EntityId id) {
  if (!m_virtual_entities.at(id).is_selected) return;
  m_virtual_entities.at(id).is_selected = false;
  bool is_parent = m_virtual_entities.at(id).is_parent;
  m_virtual_entities.at(id).is_parent = true;
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
  for (auto& [id, data] : m_virtual_entities) {
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

bool SelectionManager::is_selected(mge::EntityId id) const {
  return m_entities.contains(id) && m_entities.at(id).is_selected ||
         m_virtual_entities.contains(id) && m_virtual_entities.at(id).is_selected;
}

bool SelectionManager::is_parent(mge::EntityId id) const {
  return m_entities.contains(id) && m_entities.at(id).is_parent ||
         m_virtual_entities.contains(id) && m_virtual_entities.at(id).is_parent;
}

const std::string& SelectionManager::get_tag(mge::EntityId id) const { return m_entities.at(id).tag; }

unsigned int SelectionManager::get_selected_count() const { return m_selected_count; }

bool SelectionManager::contains(mge::EntityId id) const {
  return m_entities.contains(id) || m_virtual_entities.contains(id);
}

bool SelectionManager::is_dirty() const { return m_selected_entities.size() != m_parent_count; }

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

bool SelectionManager::add_virtual_entity(mge::EntityId id) {
  if (contains(id)) return false;

  m_virtual_entities.emplace(id, false);

  return true;
}

bool SelectionManager::remove_entity(mge::EntityId id) {
  if (!contains(id)) return false;

  unselect(id);
  m_entities.erase(id);

  return true;
}

bool SelectionManager::remove_virtual_entity(mge::EntityId id) {
  if (!contains(id)) return false;

  unselect(id);
  m_virtual_entities.erase(id);

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
  mge::vector_remove_if<mge::EntityId>(
      m_selected_entities, [&entities = m_entities, &virtual_entities = m_virtual_entities](auto& id) {
        return (!entities.contains(id) || !entities.at(id).is_selected) &&
               (!virtual_entities.contains(id) || !virtual_entities.at(id).is_selected);
      });
  if (m_parent_count == 1) {
    mge::QueryEntityByIdEvent query_event(m_selected_entities.front());
    SendEngineEvent(query_event);
    m_displayed_entity = query_event.entity;

  } else {
    m_displayed_entity = std::nullopt;
  }
}

UILayer::UILayer(mge::Scene& scene) : m_tool_manager(), m_disable_tools_combo(false), m_rotation_axis(), m_selection_manager(), m_scene(scene) {}

void UILayer::configure() {
  mge::AddEventListener(mge::EntityEvents::Added, UILayer::on_added_entity, this);
  mge::AddEventListener(mge::EntityEvents::Deleted, UILayer::on_deleted_entity, this);
  mge::AddEventListener(mge::MouseEvents::MouseButtonUpdated, UILayer::on_mouse_button_pressed, this);
  mge::AddEventListener(mge::MouseEvents::MouseMoved, UILayer::on_mouse_moved, this);
  mge::AddEventListener(mge::MouseEvents::MouseScroll, UILayer::on_mouse_scroll, this);
  AddEventListener(UIEvents::SelectionUpdate, UILayer::on_ui_selection_updated, this);
  AddEventListener(SerializationEvents::AnnounceDeserializedPoints, UILayer::on_announce_points_deserialization, this);
}

void UILayer::update() {
  if (m_selection_manager.is_dirty()) m_selection_manager.validate_selected();

  mge::UIManager::start_frame();

  auto size = ImGui::GetMainViewport()->Size;

  ImGuizmo::SetOrthographic(false);
  ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
  ImGuizmo::SetRect(0, 0, size.x, size.y);
  if (m_selection_manager.get_selected_count() > 0 &&  m_tool_manager.get_type() == ToolManager::Type::Select) {
    glm::mat4 model;
    m_scene.foreach<>(entt::get<MassCenterComponent>, entt::exclude<>, [&](mge::Entity& entity) {
      model = glm::translate(glm::mat4(1.0f), entity.get_component<MassCenterComponent>().get_position());
    });
    if (ImGuizmo::Manipulate(glm::value_ptr(m_scene.get_current_camera().get_view_matrix()), glm::value_ptr(m_scene.get_current_camera().get_projection_matrix()), 
                              m_gizmo_operation, m_gizmo_mode, glm::value_ptr(model))) {
      if (m_gizmo_operation == ImGuizmo::OPERATION::TRANSLATE) {
        glm::vec3 translation = glm::vec3{model[3][0], model[3][1], model[3][2]};
        RelativeTranslateEvent event{m_selection_manager.get_selected_ids(), translation};
        SendEvent(event);
      }
      else if (m_gizmo_operation == ImGuizmo::OPERATION::ROTATE) {
        auto quat = glm::quat_cast(model);
        RelativeRotateEvent event(m_selection_manager.get_selected_ids(), quat);
        SendEvent(event);
      }
      else if (m_gizmo_operation == ImGuizmo::OPERATION::SCALE) {
        glm::vec3 scaling = glm::vec3{model[0][0], model[1][1], model[2][2]};
        RelativeScaleEvent event(m_selection_manager.get_selected_ids(), scaling);
        SendEvent(event);
      }
    }
  }


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
      show_anaglyph_panel();
      show_serialization_panel();
    }
    ImGui::EndTabBar();
  }

  ImGui::End();
  ImGui::Render();
  mge::UIManager::end_frame();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UILayer::define_create_bezier_curve_dialog() {
  static int type = 0;

  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("AddBezierCurve", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Add Bezier Curve");
    ImGui::RadioButton("C0", &type, 0);
    ImGui::SameLine();
    ImGui::RadioButton("C2", &type, 1);
    ImGui::SameLine();
    ImGui::RadioButton("C2 Interp", &type, 2);
    if (ImGui::Button("Create", ImVec2(120, 0))) {
      if (type == 0) {
        AddBezierCurveC0Event event(m_selection_manager.get_selected_ids());
        SendEvent(event);
      } else if (type == 1) {
        AddBezierCurveC2Event event(m_selection_manager.get_selected_ids());
        SendEvent(event);
      } else if (type == 2) {
        AddBezierCurveC2InterpEvent event(m_selection_manager.get_selected_ids());
        SendEvent(event);
      }
      ImGui::CloseCurrentPopup();
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void UILayer::define_create_bezier_surface_dialog() {
  static int type = 0;
  static int wrapping = 0;
  static int patch_count[2] = {1u, 1u};
  static float size[2] = {1.0f, 1.0f};

  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("AddBezierSurface", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Add Bezier Surface");
    ImGui::RadioButton("C0", &type, 0);
    ImGui::SameLine();
    ImGui::RadioButton("C2", &type, 1);

    ImGui::RadioButton("none", &wrapping, 0);
    ImGui::SameLine();
    ImGui::RadioButton("u", &wrapping, 1);
    ImGui::SameLine();
    ImGui::RadioButton("v", &wrapping, 2);

    // add patch and size input
    ImGui::Text("patch count u/v");
    ImGui::InputInt2("##bezier_patch", reinterpret_cast<int*>(&patch_count));
    if (wrapping == 0) {
      ImGui::Text("patch size u/v");
    } else {
      ImGui::Text("patch size height/radius");
    }
    ImGui::InputFloat2("##bezier_size", reinterpret_cast<float*>(&size), "%.2f");

    if (ImGui::Button("Create", ImVec2(120, 0))) {
      if (!(wrapping == 1 && patch_count[0] < 3) && !(wrapping == 2 && patch_count[1] < 3)) {
        if (type == 0) {
          if (wrapping == 1) {
            AddBezierSurfaceC0Event event(patch_count[0], patch_count[1], size[1], size[0], BezierSurfaceWrapping::u);
            SendEvent(event);
          } else {
            AddBezierSurfaceC0Event event(patch_count[0], patch_count[1], size[0], size[1],
                                          wrapping == 0 ? BezierSurfaceWrapping::none : BezierSurfaceWrapping::v);
            SendEvent(event);
          }
        } else if (type == 1) {
          if (wrapping == 1) {
            AddBezierSurfaceC2Event event(patch_count[0], patch_count[1], size[1], size[0], BezierSurfaceWrapping::u);
            SendEvent(event);
          } else {
            AddBezierSurfaceC2Event event(patch_count[0], patch_count[1], size[0], size[1],
                                          wrapping == 0 ? BezierSurfaceWrapping::none : BezierSurfaceWrapping::v);
            SendEvent(event);
          }
        }
        ImGui::CloseCurrentPopup();
      }
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
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
    RotateEvent event(entity.get_id(), glm::quat(rotation));
    SendEvent(event);
  }
  if (ImGui::SliderFloat("y", &rotation.y, -pi, pi, "%.2f")) {
    RotateEvent event(entity.get_id(), glm::quat(rotation));
    SendEvent(event);
  }
  if (ImGui::SliderFloat("z", &rotation.z, -pi, pi, "%.2f")) {
    RotateEvent event(entity.get_id(), glm::quat(rotation));
    SendEvent(event);
  }

  // scale
  ImGui::Text("scale");
  glm::vec3 scale = component.get_scale();
  if (ImGui::InputFloat3("##scale", reinterpret_cast<float*>(&scale), "%.2f")) {
    if (glm::abs(scale.x) < glm::epsilon<float>() || glm::abs(scale.y) < glm::epsilon<float>() ||
        glm::abs(scale.z) < glm::epsilon<float>())
      return;
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
  if (component.get_registered_render_modes().size() < 2) return;
  ImGui::Text("Render mode");
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

void UILayer::show_bezier_c0_curve_panel(const mge::Entity& entity) {
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
  auto& component = entity.get_component<BezierCurveC0Component>();
  std::array<std::string, 2> options = {"yes", "no"};
  static int selected = 1;
  if (ImGui::BeginCombo("##berenstein_polygon", options[selected].c_str())) {
    for (int n = 0; n < options.size(); n++) {
      const bool is_selected = component.get_polygon_status() && !n || !component.get_polygon_status() && n;
      if (ImGui::Selectable(options[n].c_str(), is_selected)) {
        selected = n;
        BezierCurveC0UpdatePolygonStateEvent polygon_event(entity.get_id(), !n);
        SendEvent(polygon_event);
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

void UILayer::show_bezier_c2_curve_panel(const mge::Entity& entity) {
  if (entity.get_component<BezierCurveC2Component>().get_base() == BezierCurveBase::BSpline) {
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
  }

  ImGui::Text("Show polygon");
  auto& component = entity.get_component<BezierCurveC2Component>();
  std::array<std::string, 2> polygon_options = {"yes", "no"};
  static int polygon_selected = 1;
  if (ImGui::BeginCombo("##polygon", polygon_options[polygon_selected].c_str())) {
    for (int n = 0; n < polygon_options.size(); n++) {
      const bool is_selected = component.get_polygon_status() && !n || !component.get_polygon_status() && n;
      if (ImGui::Selectable(polygon_options[n].c_str(), is_selected)) {
        polygon_selected = n;
        BezierCurveC2UpdatePolygonStateEvent polygon_event(entity.get_id(), !n);
        SendEvent(polygon_event);
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  ImGui::Text("Base");
  std::unordered_map<BezierCurveBase, std::string> base_options = {{BezierCurveBase::Bernstein, "Bernstein"},
                                                                   {BezierCurveBase::BSpline, "BSpline"}};
  static BezierCurveBase base_selected = BezierCurveBase::Bernstein;
  if (ImGui::BeginCombo("##berenstein_points", base_options[base_selected].c_str())) {
    for (auto& [base, name] : base_options) {
      const bool is_selected = component.get_base() == base;
      if (ImGui::Selectable(name.c_str(), is_selected)) {
        base_selected = base;
        BezierCurveC2UpdateBaseEvent bernstein_event(entity.get_id(), base);
        SendEvent(bernstein_event);
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

void UILayer::show_bezier_c2_curve_interp_panel(const mge::Entity& entity) {
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
  auto& component = entity.get_component<BezierCurveC2InterpComponent>();
  std::array<std::string, 2> options = {"yes", "no"};
  static int selected = 1;
  if (ImGui::BeginCombo("##berenstein_polygon", options[selected].c_str())) {
    for (int n = 0; n < options.size(); n++) {
      const bool is_selected = component.get_polygon_status() && !n || !component.get_polygon_status() && n;
      if (ImGui::Selectable(options[n].c_str(), is_selected)) {
        selected = n;
        BezierCurveC2InterpUpdatePolygonStateEvent polygon_event(entity.get_id(), !n);
        SendEvent(polygon_event);
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

void UILayer::show_bezier_c0_surface_panel(const mge::Entity& entity) {
  static int line_count = entity.get_component<BezierSurfaceC0Component>().get_line_count();
  ImGui::Text("Line count");
  if (ImGui::InputInt("##line", &line_count, 1, 1)) {
    line_count = std::clamp(line_count, 1, 128);
    BezierSurfaceC0UpdateLineCountEvent event(entity.get_id(), line_count);
    SendEvent(event);
  }

  ImGui::Text("Show grid");
  auto& component = entity.get_component<BezierSurfaceC0Component>();
  std::array<std::string, 2> options = {"yes", "no"};
  static int selected = 1;
  if (ImGui::BeginCombo("##gridc0", options[selected].c_str())) {
    for (int n = 0; n < options.size(); n++) {
      const bool is_selected = component.get_grid_status() && !n || !component.get_grid_status() && n;
      if (ImGui::Selectable(options[n].c_str(), is_selected)) {
        selected = n;
        BezierSurfaceC0UpdateGridStateEvent polygon_event(entity.get_id(), !n);
        SendEvent(polygon_event);
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

void UILayer::show_bezier_c2_surface_panel(const mge::Entity& entity) {
  int line_count = entity.get_component<BezierSurfaceC2Component>().get_line_count();
  ImGui::Text("Line count");
  if (ImGui::InputInt("##line", &line_count, 1, 1)) {
    line_count = std::clamp(line_count, 1, 128);
    BezierSurfaceC2UpdateLineCountEvent event(entity.get_id(), line_count);
    SendEvent(event);
  }

  ImGui::Text("Show grid");
  auto& component = entity.get_component<BezierSurfaceC2Component>();
  std::array<std::string, 2> options = {"yes", "no"};
  static int selected = 1;
  if (ImGui::BeginCombo("##gridc2", options[selected].c_str())) {
    for (int n = 0; n < options.size(); n++) {
      const bool is_selected = component.get_grid_status() && !n || !component.get_grid_status() && n;
      if (ImGui::Selectable(options[n].c_str(), is_selected)) {
        selected = n;
        BezierSurfaceC2UpdateGridStateEvent polygon_event(entity.get_id(), !n);
        SendEvent(polygon_event);
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

void UILayer::show_tools_panel() {
  using Tool = ToolManager::Type;
  static std::vector<Tool> displayable_tools = {Tool::Select, Tool::Delete};
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

  static int gizmo_operation = 0;
  ImGui::Text("Gizmo operation");
  ImGui::RadioButton("translate", &gizmo_operation, 0);
  ImGui::SameLine();
  ImGui::RadioButton("rotate", &gizmo_operation, 1);
  ImGui::SameLine();
  ImGui::RadioButton("scale", &gizmo_operation, 2);
  if (gizmo_operation == 0) {
    m_gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
  }
  else if (gizmo_operation == 1) {
    m_gizmo_operation = ImGuizmo::OPERATION::ROTATE;
  }
  else if (gizmo_operation == 2) {
    m_gizmo_operation = ImGuizmo::OPERATION::SCALE;
  }

  if (ImGui::Button("translate to cursor")) {
    TranslateToCursorEvent event(m_selection_manager.get_selected_ids());
    SendEvent(event);
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
  if (ImGui::Button("curve") && m_selection_manager.get_selected_count() > 1) {
    ImGui::OpenPopup("AddBezierCurve");
  }
  define_create_bezier_curve_dialog();
  ImGui::SameLine();
  if (ImGui::Button("surface")) {
    ImGui::OpenPopup("AddBezierSurface");
  }
  define_create_bezier_surface_dialog();
}

void UILayer::show_entities_list_panel() {
  ImGui::BeginChild("EntitiesList", ImVec2(ImGui::GetContentRegionAvail().x, 150), ImGuiWindowFlags_None,
                    ImGuiWindowFlags_HorizontalScrollbar);
  for (auto id : m_selection_manager.get_all_entities_ids()) {
    if (ImGui::Selectable(m_selection_manager.get_tag(id).c_str(), m_selection_manager.is_selected(id))) {
      if (m_selection_manager.get_displayed_entity().has_value() &&
          (m_selection_manager.get_displayed_entity()->get().has_component<BezierCurveC0Component>())) {
        auto bezier_id = m_selection_manager.get_displayed_entity().value().get().get_id();
        if (m_selection_manager.is_selected(id) && m_tool_manager.get_type() == ToolManager::Type::RemoveBezierPoint) {
          m_selection_manager.unselect(id);
          BezierCurveC0DeletePointEvent event(bezier_id, id);
          SendEvent(event);
          continue;
        } else if (!m_selection_manager.is_selected(id) &&
                   m_tool_manager.get_type() == ToolManager::Type::AddBezierPoint) {
          m_selection_manager.select(id, false);
          BezierCurveC0AddPointEvent event(bezier_id, id);
          SendEvent(event);
          continue;
        }
      } else if (m_selection_manager.get_displayed_entity().has_value() &&
                 m_selection_manager.get_displayed_entity()->get().has_component<BezierCurveC2Component>()) {
        auto bezier_id = m_selection_manager.get_displayed_entity().value().get().get_id();
        if (m_selection_manager.is_selected(id) && m_tool_manager.get_type() == ToolManager::Type::RemoveBezierPoint) {
          m_selection_manager.unselect(id);
          BezierCurveC2DeletePointEvent event(bezier_id, id);
          SendEvent(event);
          continue;
        } else if (!m_selection_manager.is_selected(id) &&
                   m_tool_manager.get_type() == ToolManager::Type::AddBezierPoint) {
          m_selection_manager.select(id, false);
          BezierCurveC2AddPointEvent event(bezier_id, id);
          SendEvent(event);
          continue;
        }
      } else if (m_selection_manager.get_displayed_entity().has_value() &&
                 m_selection_manager.get_displayed_entity()->get().has_component<BezierCurveC2InterpComponent>()) {
        auto bezier_id = m_selection_manager.get_displayed_entity().value().get().get_id();
        if (m_selection_manager.is_selected(id) && m_tool_manager.get_type() == ToolManager::Type::RemoveBezierPoint) {
          m_selection_manager.unselect(id);
          BezierCurveC2InterpDeletePointEvent event(bezier_id, id);
          SendEvent(event);
          continue;
        } else if (!m_selection_manager.is_selected(id) &&
                   m_tool_manager.get_type() == ToolManager::Type::AddBezierPoint) {
          m_selection_manager.select(id, false);
          BezierCurveC2InterpAddPointEvent event(bezier_id, id);
          SendEvent(event);
          continue;
        }
      }

      if (!ImGui::GetIO().KeyCtrl) {
        m_selection_manager.unselect_all();
      }
      if (m_selection_manager.is_selected(id)) {
        if (m_selection_manager.is_parent(id)) m_selection_manager.unselect(id);
      } else {
        m_selection_manager.select(id, true);
      }
    }
  }
  ImGui::EndChild();
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

  if (entity.has_component<BezierCurveC0Component>()) {
    show_bezier_c0_curve_panel(entity);
  }

  if (entity.has_component<BezierCurveC2Component>()) {
    show_bezier_c2_curve_panel(entity);
  }

  if (entity.has_component<BezierCurveC2InterpComponent>()) {
    show_bezier_c2_curve_interp_panel(entity);
  }

  if (entity.has_component<BezierSurfaceC0Component>()) {
    show_bezier_c0_surface_panel(entity);
  }

  if (entity.has_component<BezierSurfaceC2Component>()) {
    show_bezier_c2_surface_panel(entity);
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

void UILayer::show_anaglyph_panel() {
  ImGui::Separator();
  ImGui::Text("Anaglyphs");
  // combo on/off
  std::array<std::string, 2> options = {"on", "off"};
  static int selected = 1;
  if (ImGui::BeginCombo("##anaglyph_state", options[selected].c_str())) {
    for (int n = 0; n < options.size(); n++) {
      const bool is_selected = n == selected;
      if (ImGui::Selectable(options[n].c_str(), is_selected)) {
        selected = n;
        AnaglyphUpdateStateEvent event(!n);
        SendEvent(event);
      }

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  // eye dist
  static float eye_distance = 0.06f;
  ImGui::Text("Eye distance");
  if (ImGui::InputFloat("##eye_distance", &eye_distance, 0.1f, 0.1f, "%.2f")) {
    AnaglyphUpdateEyeDistanceEvent event(eye_distance);
    SendEvent(event);
  }
  // proj plane dist
  static float projective_plane_distance = 1.0f;
  ImGui::Text("Projective plane distance");
  if (ImGui::InputFloat("##projective_plane_distance", &projective_plane_distance, 0.1f, 0.1f, "%.2f")) {
    AnaglyphUpdateProjectivePlaneDistanceEvent event(projective_plane_distance);
    SendEvent(event);
  }
  // screen dist
  static float screen_distance = 1.0f;
  ImGui::Text("Screen distance");
  if (ImGui::InputFloat("##screen_distance", &screen_distance, 0.1f, 0.1f, "%.2f")) {
    AnaglyphUpdateScreenDistanceEvent event(screen_distance);
    SendEvent(event);
  }
}

void UILayer::show_serialization_panel() {
  ImGui::Separator();

  if (ImGui::Button("Save")) {
    nfdu8char_t *save_path;
    nfdu8filteritem_t filters[1] = { { "Scene JSON", "json" } };
    auto result = NFD::SaveDialog(save_path, filters, 1, fs::current_path().c_str());
    if (result == NFD_OKAY) {
      SerializeSceneEvent event{fs::path(save_path)};
      SendEvent(event);
      NFD::FreePath(save_path);
    } else if (result == NFD_ERROR) {
      MGE_ERROR("Could not load scene");
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Load")) {
    nfdu8char_t *load_path;
    nfdu8filteritem_t filters[1] = { { "Scene JSON", "json" } };
    auto result = NFD::OpenDialog(load_path, filters, 1, fs::current_path().c_str());
    if (result == NFD_OKAY) {
      DeserializeSceneEvent event{fs::path(load_path)};
      SendEvent(event);
      NFD::FreePath(load_path);
    } else if (result == NFD_ERROR) {
      MGE_ERROR("Could not load scene");
    }
  }
}

bool UILayer::on_added_entity(mge::AddedEntityEvent& event) {
  mge::QueryEntityByIdEvent query_event(event.id);
  SendEngineEvent(query_event);
  MGE_ASSERT(query_event.entity.has_value(), "Queried entity doesn't exist: {}", static_cast<int>(event.id));
  auto& entity = query_event.entity.value().get();
  if (!entity.has_component<SelectibleComponent>()) return false;
  if (entity.has_component<mge::TagComponent>())
    m_selection_manager.add_entity(entity.get_id(), entity.get_component<mge::TagComponent>().get_tag());
  else {
    m_selection_manager.add_virtual_entity(entity.get_id());
    return true;
  }
  if (m_selection_manager.get_displayed_entity().has_value() &&
      m_selection_manager.get_displayed_entity()->get().has_component<BezierCurveC0Component>() &&
      entity.has_component<PointComponent>()) {
    m_selection_manager.select(entity.get_id(), false);
    BezierCurveC0AddPointEvent add_event(m_selection_manager.get_displayed_entity()->get().get_id(), entity.get_id());
    SendEvent(add_event);
  } else if (m_selection_manager.get_displayed_entity().has_value() &&
             m_selection_manager.get_displayed_entity()->get().has_component<BezierCurveC2Component>() &&
             entity.has_component<PointComponent>()) {
    m_selection_manager.select(entity.get_id(), false);
    BezierCurveC2AddPointEvent add_event(m_selection_manager.get_displayed_entity()->get().get_id(), entity.get_id());
    SendEvent(add_event);
  } else if (m_selection_manager.get_displayed_entity().has_value() &&
             m_selection_manager.get_displayed_entity()->get().has_component<BezierCurveC2InterpComponent>() &&
             entity.has_component<PointComponent>()) {
    m_selection_manager.select(entity.get_id(), false);
    BezierCurveC2InterpAddPointEvent add_event(m_selection_manager.get_displayed_entity()->get().get_id(),
                                               entity.get_id());
    SendEvent(add_event);
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
    return true;
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
            if (m_selection_manager.is_parent(id)) m_selection_manager.unselect(id);
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

bool UILayer::on_ui_selection_updated(UISelectionUpdateEvent& event) {
  if (event.state) {
    m_selection_manager.select(event.id, event.is_parent);
  } else {
    m_selection_manager.unselect(event.id);
  }
  return true;
}

bool UILayer::on_announce_points_deserialization(AnnounceDeserializedPointsEvent& event) {
  for (auto& point_data : event.data) {
    m_selection_manager.add_entity(point_data.first, point_data.second);
  }
  return true;
}