#ifndef MCAD_CAD_LAYER_HH
#define MCAD_CAD_LAYER_HH

#include "mge.hh"

#include "../events/events.hh"
#include "../vertices/cursor_vertex.hh"
#include "../vertices/geometry_vertex.hh"
#include "../vertices/point_vertex.hh"

class CadLayer : public mge::Layer {
 public:
  CadLayer(mge::Scene& scene, const glm::ivec2& window_size);
  ~CadLayer();

  virtual void configure() override;
  virtual void update() override;

  mge::Entity& get_mass_center() { return m_mass_center; }

 private:
  mge::Scene& m_scene;
  mge::Entity& m_cursor;
  mge::Entity& m_mass_center;
  glm::vec2 m_window_size;
  std::vector<mge::EntityId> m_to_be_destroyed;
  std::unique_ptr<mge::RenderPipeline<GeometryVertex>> m_geometry_wireframe_pipeline = nullptr;
  std::unique_ptr<mge::RenderPipeline<GeometryVertex>> m_geometry_solid_pipeline = nullptr;
  std::unique_ptr<mge::InstancedRenderPipeline<GeometryVertex, PointInstancedVertex>> m_point_pipeline = nullptr;
  std::unique_ptr<mge::RenderPipeline<GeometryVertex>> m_bezier_pipeline = nullptr;
  std::unique_ptr<mge::RenderPipeline<GeometryVertex>> m_bezier_polygon_pipeline = nullptr;
  std::unique_ptr<mge::RenderPipeline<GeometryVertex>> m_cursor_pipeline = nullptr;

  mge::OptionalEntity get_closest_selectible_entity(glm::vec2 screen_space_position) const;
  glm::vec3 unproject_point(glm::vec2 screen_space_position) const;

  void update_mass_center();
  void update_bezier(mge::Entity& entity);
  void update_point_instance_data(mge::Entity& entity);
  void update_parent_bezier(mge::Entity& entity);

  // Camera events
  bool on_camera_angle_changed(mge::CameraAngleChangedEvent& event);
  bool on_camera_position_changed(mge::CameraPositionChangedEvent& event);
  bool on_camera_zoom(mge::CameraZoomEvent& event);
  // Window events
  bool on_window_framebuffer_resized(mge::WindowFramebufferResizedEvent& event);
  // Entity events
  bool on_delete_entity_by_id(mge::DeleteEntityEvent& event);
  bool on_query_entity_by_id(mge::QueryEntityByIdEvent& event);
  bool on_query_entity_by_position(mge::QueryEntityByPositionEvent& event);
  bool on_query_entity_by_tag(mge::QueryEntityByTagEvent& event);
  // Point events
  bool on_add_point(AddPointEvent& event);
  // Torus events
  bool on_add_torus(AddTorusEvent& event);
  bool on_torus_radius_updated(TorusRadiusUpdatedEvent& event);
  bool on_torus_grid_density_updated(TorusGridDensityUpdatedEvent& event);
  // Bezier Curve C0 events
  bool on_add_bezier_curve_c0(AddBezierCurveC0Event& event);
  bool on_add_bezier_curve_c0_point(BezierCurveC0AddPointEvent& event);
  bool on_delete_bezier_curve_c0_point(BezierCurveC0DeletePointEvent& event);
  bool on_update_bezier_curve_c0_polygon_state(BezierCurveC0UpdatePolygonStateEvent& event);
  // Bezier Curve C2 events
  bool on_add_bezier_curve_c2(AddBezierCurveC2Event& event);
  bool on_add_bezier_curve_c2_point(BezierCurveC2AddPointEvent& event);
  bool on_delete_bezier_curve_c2_point(BezierCurveC2DeletePointEvent& event);
  bool on_update_bezier_curve_c2_polygon_state(BezierCurveC2UpdatePolygonStateEvent& event);
  bool on_update_bezier_curve_c2_base(BezierCurveC2UpdateBaseEvent& event);
  bool on_create_bernstein_point(CreateBernsteinPointEvent& event);
  // Cursor events
  bool on_cursor_move(CursorMoveEvent& event);
  // Transform events
  bool on_translate_to_cursor(TranslateToCursorEvent& event);
  bool on_relative_scale(RelativeScaleEvent& event);
  bool on_relative_rotate(RelativeRotateEvent& event);
  bool on_translate(TranslateEvent& event);
  bool on_scale(ScaleEvent& event);
  bool on_rotate(RotateEvent& event);
  // Tag events
  bool on_tag_updated(mge::TagUpdateEvent& event);
  // Render mode events
  bool on_render_mode_updated(mge::RenderModeUpdatedEvent& event);
  // Seleciton events
  bool on_selection_updated(SelectionUpdateEvent& event);
  bool on_unselect_all_entities(UnselectAllEntitiesEvent& event);
};

#endif  // MCAD_CAD_LAYER_HH