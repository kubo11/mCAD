#ifndef MCAD_UI_LAYER_HH
#define MCAD_UI_LAYER_HH

#include "mge.hh"

#include "../components/point_component.hh"
#include "../components/selectible_component.hh"
#include "../components/torus_component.hh"
#include "../events/events.hh"

class SelectionManager {
 public:
  struct EntityMapNode {
    std::string tag;
    bool is_selected;
    bool is_parent;
  };

  struct VirtualEntityMapNode {
    bool is_selected;
    bool is_parent;
  };

  SelectionManager();

  void select(mge::EntityId id, bool is_parent);
  void select_virtual(mge::EntityId id, bool is_parent);
  void unselect(mge::EntityId id);
  void unselect_virtual(mge::EntityId id);
  void unselect_all();

  mge::OptionalEntity get_displayed_entity() const;
  bool is_selected(mge::EntityId id) const;
  bool is_parent(mge::EntityId id) const;
  const std::string& get_tag(mge::EntityId id) const;
  unsigned int get_selected_count() const;
  unsigned int get_parent_count() const;
  bool contains(mge::EntityId id) const;
  bool is_dirty() const;

  std::vector<mge::EntityId> get_all_entities_ids();
  std::vector<mge::EntityId> get_selected_ids();
  std::vector<mge::EntityId> get_selected_parents_ids();

  bool add_entity(mge::EntityId id, const std::string& tag);
  bool add_virtual_entity(mge::EntityId id);
  bool remove_entity(mge::EntityId id);
  bool remove_virtual_entity(mge::EntityId id);
  bool rename_entity(mge::EntityId id, const std::string& tag);

  void validate_selected();

 private:
  std::map<mge::EntityId, EntityMapNode> m_entities;
  std::map<mge::EntityId, VirtualEntityMapNode> m_virtual_entities;
  std::vector<mge::EntityId> m_selected_entities;
  mge::OptionalEntity m_displayed_entity;
  unsigned int m_selected_count;
  unsigned int m_parent_count;
};

class ToolManager {
 public:
  enum class Type { Select, Delete, AddBezierPoint, RemoveBezierPoint };

  ToolManager(Type current_type = Type::Select, Type previous_type = Type::Select);

  static std::string get_name(Type type);

  Type get_type() const;
  void set_type(Type type);
  void set_type_and_update_previous(Type type);
  void restore_type();

 private:
  Type m_current_tool;
  Type m_previous_tool;
};

class RotationAxis {
 public:
  enum class Type { X, Y, Z };

  RotationAxis(Type type = Type::X);

  static std::string get_name(Type type);
  glm::vec3 get_value() const;
  Type get_type() const;

  void set_type(Type type);

 private:
  Type m_type;
};

class UILayer : public mge::Layer {
 public:
  UILayer(mge::Scene& scene);
  ~UILayer() = default;

  virtual void configure() override;
  virtual void update() override;

 private:
  ToolManager m_tool_manager;
  bool m_disable_tools_combo;
  RotationAxis m_rotation_axis;
  SelectionManager m_selection_manager;
  mge::Scene& m_scene;
  ImGuizmo::OPERATION m_gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
  ImGuizmo::MODE m_gizmo_mode = ImGuizmo::MODE::WORLD;
  bool m_show_gregory_creator = false;
  std::vector<std::vector<mge::EntityId>> m_holes = {};

  void define_create_bezier_curve_dialog();
  void define_create_bezier_surface_dialog();
  void show_create_gregory_patch_window();

  void show_tag_panel(const mge::Entity& entity);
  void show_transform_panel(const mge::Entity& entity);
  void show_limited_transform_panel(const mge::Entity& entity);
  void show_renderable_component(const mge::Entity& entity);
  void show_torus_panel(const mge::Entity& entity);
  void show_bezier_c0_curve_panel(const mge::Entity& entity);
  void show_bezier_c2_curve_panel(const mge::Entity& entity);
  void show_bezier_c2_curve_interp_panel(const mge::Entity& entity);
  void show_bezier_c0_surface_panel(const mge::Entity& entity);
  void show_bezier_c2_surface_panel(const mge::Entity& entity);
  void show_gregory_patch_panel(const mge::Entity& entity);
  void show_tools_panel();
  void show_entities_list_panel();
  void show_entity_parameters_panel(const mge::Entity& entity);
  void show_anaglyph_panel();
  void show_serialization_panel();

  bool on_added_entity(mge::AddedEntityEvent& event);
  bool on_deleted_entity(mge::DeletedEntityEvent& event);

  bool on_mouse_moved(mge::MouseMovedEvent& event);
  bool on_mouse_button_pressed(mge::MouseButtonUpdatedEvent& event);
  bool on_mouse_scroll(mge::MouseScrollEvent& event);

  bool on_ui_selection_updated(UISelectionUpdateEvent& event);

  bool on_announce_points_deserialization(AnnounceDeserializedPointsEvent& event);

  void send_camera_move_events(mge::MouseMovedEvent& event);
  void send_camera_zoom_event(mge::MouseScrollEvent& event);
};

#endif  // MCAD_UI_LAYER_HH