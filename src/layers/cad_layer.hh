#ifndef MCAD_CAD_LAYER_HH
#define MCAD_CAD_LAYER_HH

#include "mge.hh"

#include "../events/events.hh"
#include "../geometry/geometry_vertex.hh"

class EntitySelectionManager {
 public:
  EntitySelectionManager(mge::Entity& mass_center);
  ~EntitySelectionManager() = default;

  inline int count() const { return m_selected_entities.size(); }
  inline bool empty() const { return m_selected_entities.empty(); }
  inline std::optional<std::reference_wrapper<mge::Entity>>
  get_single_selected_entity() const {
    if (m_selected_entities.size() == 1) {
      return *m_selected_entities.begin();
    }

    return std::nullopt;
  }
  void add_entity(mge::Entity& entity);
  void remove_entity(mge::Entity& entity);
  void remove_all();
  void foreach (std::function<void(mge::Entity&)> func);
  const glm::vec3& get_mass_center_position() const;
  void set_event_handler(std::function<void(mge::Event&)> send_event) {
    m_send_event = send_event;
  }

 private:
  static const glm::vec3 s_selected_color;
  static const glm::vec3 s_regular_color;
  std::set<std::reference_wrapper<mge::Entity>> m_selected_entities;
  std::function<void(mge::Event&)> m_send_event;
  mge::Entity& m_mass_center;

  void update_mass_center();
};

class CadLayer : public mge::Layer {
 public:
  CadLayer(mge::Scene& scene);
  ~CadLayer() {}

  virtual void configure() override;
  virtual void update() override;
  virtual void handle_event(mge::Event& event, float dt) override;

  inline mge::Entity& get_cursor() const { return m_cursor; }
  std::optional<std::reference_wrapper<mge::Entity>> get_closest_entity(
      glm::vec2 position);

 private:
  static const std::string s_cursor_tag;
  static const std::string s_mass_center_tag;
  glm::u8vec3 m_background_color = {100, 100, 100};
  mge::Scene& m_scene;
  mge::Entity& m_cursor;
  EntitySelectionManager m_selection_manager;

  glm::vec3 unproject_point(glm::vec2 pos) const;

  bool on_camera_angle_modified(mge::CameraAngleEvent& event);
  bool on_camera_position_modified(mge::CameraPositionEvent& event);
  bool on_camera_zoom(mge::CameraZoomEvent& event);
  bool on_select_entity_by_tag(SelectEntityByTagEvent& event);
  bool on_select_entity_by_position(SelectEntityByPositionEvent& event);
  bool on_unselect_entity_by_tag(UnSelectEntityByTagEvent& event);
  bool on_unselect_all_entities(UnSelectAllEntitiesEvent& event);
  bool on_query_selected_entity(QuerySelectedEntityEvent& event);
  bool on_add_point_event(AddPointEvent& event);
  bool on_add_torus_event(AddTorusEvent& event);
  bool on_delete_position_event(DeletePositionEvent& event);
  bool on_cursor_move_event(CursorMoveEvent& event);
  bool on_scale_event(ScaleEvent& event);
  bool on_rotate_event(RotateEvent& event);
  bool on_move_event(MoveEvent& event);
  bool on_rename_event(RenameEvent& event);
  bool on_torus_update_geometry(TorusUpdateGeometryEvent& event);
  bool on_torus_update_topology(TorusUpdateTopologyEvent& event);
};

#endif  // MCAD_CAD_LAYER_HH