#ifndef MCAD_CAD_LAYER_HH
#define MCAD_CAD_LAYER_HH

#include "mge.hh"

#include "../events/events.hh"
#include "../geometry/geometry_vertex.hh"

class CadLayer : public mge::Layer {
 public:
  CadLayer(mge::Scene& scene);
  ~CadLayer();

  virtual void configure() override;
  virtual void update() override;
  virtual void handle_event(mge::Event& event, float dt) override;

  mge::OptionalEntity get_closest_selectible_entity(glm::vec2 position);

 private:
  static const std::string s_cursor_tag;
  static const std::string s_mass_center_tag;
  mge::Scene& m_scene;
  mge::Entity& m_cursor;
  mge::Entity& m_mass_center;
  mge::EntityVector m_selected;

  glm::vec3 unproject_point(glm::vec2 pos) const;

  void on_selectible_construct(entt::registry&, entt::entity entity);
  void on_selectible_update(entt::registry&, entt::entity entity);
  void on_selectible_destroy(entt::registry&, entt::entity entity);
  void on_transform_update(entt::registry&, entt::entity entity);

  bool on_camera_angle_modified(mge::CameraAngleEvent& event);
  bool on_camera_position_modified(mge::CameraPositionEvent& event);
  bool on_camera_zoom(mge::CameraZoomEvent& event);
  bool on_select_entity_by_tag(SelectEntityByTagEvent& event);
  bool on_select_entity_by_position(SelectEntityByPositionEvent& event);
  bool on_unselect_entity_by_tag(UnSelectEntityByTagEvent& event);
  bool on_unselect_all_entities(UnSelectAllEntitiesEvent& event);
  bool on_add_point_event(AddPointEvent& event);
  bool on_add_torus_event(AddTorusEvent& event);
  bool on_add_bezier_event(AddBezierEvent& event);
  bool on_delete_position_event(DeletePositionEvent& event);
  bool on_cursor_move_event(CursorMoveEvent& event);
  bool on_move_by_cursor_event(MoveByCursorEvent& event);
  bool on_scale_by_cursor_event(ScaleByCursorEvent& event);
  bool on_rotate_by_cursor_event(RotateByCursorEvent& event);
  bool on_move_by_ui_event(MoveByUIEvent& event);
  bool on_scale_by_ui_event(ScaleByUIEvent& event);
  bool on_rotate_by_ui_event(RotateByUIEvent& event);
  bool on_rename_event(RenameEvent& event);
  bool on_torus_updated(TorusUpdatedEvent& event);
  bool on_render_mode_updated(mge::RenderModeUpdatedEvent& event);
};

#endif  // MCAD_CAD_LAYER_HH