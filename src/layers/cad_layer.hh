#ifndef MCAD_CAD_LAYER_HH
#define MCAD_CAD_LAYER_HH

#include "mge.hh"

#include "../events/events.hh"
#include "../geometry/geometry_vertex.hh"

class CadLayer : public mge::Layer {
 public:
  CadLayer(mge::Scene& scene);
  ~CadLayer() {}

  virtual void configure() override;
  virtual void update() override;
  virtual void handle_event(mge::Event& event, float dt) override;

  inline mge::Entity& get_cursor() const { return m_cursor; }

 private:
  glm::u8vec3 m_background_color = {100, 100, 100};
  mge::Scene& m_scene;
  mge::Entity& m_cursor;

  glm::vec3 unproject_point(glm::vec2 pos) const;

  bool on_query_selected_tag_event(QuerySelectedTagEvent& event);
  bool on_query_selected_cursor_event(QuerySelectedCursorEvent& event);
  bool on_camera_angle_modified(mge::CameraAngleEvent& event);
  bool on_camera_position_modified(mge::CameraPositionEvent& event);
  bool on_camera_zoom(mge::CameraZoomEvent& event);
  bool on_add_point_event(AddPointEvent& event);
  bool on_add_torus_event(AddTorusEvent& event);
  bool on_delete_position_event(DeletePositionEvent& event);
  bool on_cursor_move_event(CursorMoveEvent& event);
  bool on_scale_event(ScaleEvent& event);
  bool on_rotate_event(RotateEvent& event);
  bool on_move_event(MoveEvent& event);
};

#endif  // MCAD_CAD_LAYER_HH