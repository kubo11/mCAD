#ifndef MCAD_CAD_LAYER_HH
#define MCAD_CAD_LAYER_HH

#include "mge.hh"

#include "../events/events.hh"
#include "../vertices/bezier_curve_c2_interp_vertex.hh"
#include "../vertices/cursor_vertex.hh"
#include "../vertices/geometry_vertex.hh"
#include "../vertices/point_vertex.hh"

class CadLayer : public mge::Layer {
 public:
  CadLayer(mge::Scene& scene, const glm::ivec2& window_size);
  ~CadLayer();

  PREVENT_COPY(CadLayer);

  virtual void configure() override;
  virtual void update() override;

  mge::Entity& create_point(const glm::vec3& pos);
  mge::Entity& create_torus(const glm::vec3& pos, float inner_radius, float outer_radius, unsigned int inner_density,
                            unsigned int outer_density);
  mge::Entity& create_bezier_curve_c0(const std::vector<mge::EntityId>& points);
  mge::Entity& create_bezier_curve_c2(const std::vector<mge::EntityId>& points);
  mge::Entity& create_bezier_curve_c2_interp(const std::vector<mge::EntityId>& points);
  mge::Entity& create_bezier_surface_c0(const glm::vec3& pos, BezierSurfaceWrapping wrapping, unsigned int patches_u,
                                        unsigned int patches_v, float size_u, float size_v);
  mge::Entity& create_bezier_surface_c0(const std::vector<std::vector<mge::EntityId>>& points,
                                        BezierSurfaceWrapping wrapping, unsigned int patches_u, unsigned int patches_v,
                                        unsigned int line_count);
  mge::Entity& create_bezier_surface_c2(const glm::vec3& pos, BezierSurfaceWrapping wrapping, unsigned int patches_u,
                                        unsigned int patches_v, float size_u, float size_v);
  mge::Entity& create_bezier_surface_c2(const std::vector<std::vector<mge::EntityId>>& points,
                                        BezierSurfaceWrapping wrapping, unsigned int patches_u, unsigned int patches_v,
                                        unsigned int line_count);

  void create_cursor();
  void create_mass_center();
  void create_vertical_line();

 private:
  bool m_do_anaglyphs;
  mge::Scene& m_scene;
  std::reference_wrapper<mge::Entity> m_cursor;
  std::reference_wrapper<mge::Entity> m_vertical_line;
  std::reference_wrapper<mge::Entity> m_mass_center;
  glm::vec2 m_window_size;
  std::vector<mge::EntityId> m_to_be_destroyed;
  std::unique_ptr<mge::RenderPipeline<GeometryVertex>> m_geometry_wireframe_pipeline = nullptr;
  std::unique_ptr<mge::RenderPipeline<GeometryVertex>> m_geometry_solid_pipeline = nullptr;
  std::unique_ptr<mge::InstancedRenderPipeline<GeometryVertex, PointInstancedVertex>> m_point_pipeline = nullptr;
  std::unique_ptr<mge::RenderPipeline<GeometryVertex>> m_bezier_pipeline = nullptr;
  std::unique_ptr<mge::RenderPipeline<BezierCurveC2InterpVertex>> m_bezier_c2_interp_pipeline = nullptr;
  std::unique_ptr<mge::RenderPipeline<GeometryVertex>> m_bezier_surface_pipeline = nullptr;
  std::unique_ptr<mge::RenderPipeline<GeometryVertex>> m_bezier_polygon_pipeline = nullptr;
  std::unique_ptr<mge::RenderPipeline<GeometryVertex>> m_bezier_grid_pipeline = nullptr;
  std::unique_ptr<mge::RenderPipeline<GeometryVertex>> m_cursor_pipeline = nullptr;
  std::unique_ptr<mge::RenderPipeline<GeometryVertex>> m_vertical_line_pipeline = nullptr;

  mge::OptionalEntity get_closest_selectible_entity(glm::vec2 screen_space_position) const;
  glm::vec3 unproject_point(glm::vec2 screen_space_position) const;

  void update_mass_center();
  void update_bezier(mge::Entity& entity);
  void update_point_instance_data(mge::Entity& entity);
  void update_parent_bezier(mge::Entity& entity);

  void relative_translate(mge::Entity& entity, const glm::vec3& center, const glm::vec3& destination, std::list<mge::EntityId>& visited);
  void relative_scale(mge::Entity& entity, const glm::vec3& center, const glm::vec3& scaling_factor);
  void relative_rotate(mge::Entity& entity, const glm::vec3& center, const glm::quat& q);

  // Anaglyph events
  bool on_anaglyph_update_state(AnaglyphUpdateStateEvent& event);
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
  bool on_collapse_points(CollapsePointsEvent& event);
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
  // Bezier Curve C2 Interp events
  bool on_add_bezier_curve_c2_interp(AddBezierCurveC2InterpEvent& event);
  bool on_add_bezier_curve_c2_interp_point(BezierCurveC2InterpAddPointEvent& event);
  bool on_delete_bezier_curve_c2_interp_point(BezierCurveC2InterpDeletePointEvent& event);
  bool on_update_bezier_curve_c2_interp_polygon_state(BezierCurveC2InterpUpdatePolygonStateEvent& event);
  // Bezier Surface C0 events
  bool on_add_bezier_surface_c0(AddBezierSurfaceC0Event& event);
  bool on_update_bezier_surface_c0_grid_state(BezierSurfaceC0UpdateGridStateEvent& event);
  bool on_update_bezier_surface_c0_line_count(BezierSurfaceC0UpdateLineCountEvent& event);
  // Bezier Surface C2 events
  bool on_add_bezier_surface_c2(AddBezierSurfaceC2Event& event);
  bool on_update_bezier_surface_c2_grid_state(BezierSurfaceC2UpdateGridStateEvent& event);
  bool on_update_bezier_surface_c2_line_count(BezierSurfaceC2UpdateLineCountEvent& event);
  // Gregory Patch events
  bool on_find_hole(FindHoleEvent& event);
  bool on_add_gregory_patch(AddGregoryPatchEvent& event);
  // Cursor events
  bool on_cursor_move(CursorMoveEvent& event);
  // Transform events
  bool on_translate_to_cursor(TranslateToCursorEvent& event);
  bool on_relative_translate(RelativeTranslateEvent& event);
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
  bool on_degrade_selection(DegradeSelectionEvent& event);
};

#endif  // MCAD_CAD_LAYER_HH