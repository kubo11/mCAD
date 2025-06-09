#include "cad_layer.hh"

#include "../components/bezier_curve_c0_component.hh"
#include "../components/bezier_curve_c2_component.hh"
#include "../components/bezier_curve_c2_interp_component.hh"
#include "../components/bezier_surface_c0_component.hh"
#include "../components/bezier_surface_c2_component.hh"
#include "../components/color_component.hh"
#include "../components/intersection_component.hh"
#include "../components/gregory_patch_component.hh"
#include "../components/mass_center_component.hh"
#include "../components/point_component.hh"
#include "../components/selectible_component.hh"
#include "../components/torus_component.hh"
#include "../geometry/gregory_patch_builder.hh"
#include "../vertices/cursor_vertex.hh"

CadLayer::CadLayer(mge::Scene& scene, const glm::ivec2& window_size)
    : m_scene(scene),
      m_cursor(m_scene.create_entity()),
      m_mass_center(m_scene.create_entity()),
      m_vertical_line(m_scene.create_entity()),
      m_window_size(window_size),
      m_do_anaglyphs(false) {}

CadLayer::~CadLayer() { m_scene.clear(); }

void CadLayer::configure() {
  // Enable listeners
  m_scene.enable_on_update_listeners<mge::TransformComponent>();
  m_scene.enable_on_update_listeners<ColorComponent>();
  m_scene.enable_on_update_listeners<BezierCurveC0Component>();
  m_scene.enable_on_update_listeners<BezierCurveC2Component>();
  m_scene.enable_on_update_listeners<BezierCurveC2InterpComponent>();
  m_scene.enable_on_update_listeners<BezierSurfaceC0Component>();
  m_scene.enable_on_update_listeners<BezierSurfaceC2Component>();
  // Pipelines
  auto base_shader_path = fs::current_path() / "src" / "shaders";
  mge::RenderPipelineBuilder pipeline_builder;
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "solid" / "surface"))
      .add_uniform_update<glm::mat4>("projection_view", [&scene = m_scene]() {
        auto& camera = scene.get_current_camera();
        return camera.get_projection_matrix() * camera.get_view_matrix();
      });
  m_geometry_solid_pipeline = std::move(pipeline_builder.build<PosUvVertex>(mge::DrawPrimitiveType::TRIANGLE));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "solid" / "wireframe"))
      .add_uniform_update<glm::mat4>("projection_view", [&scene = m_scene]() {
        auto& camera = scene.get_current_camera();
        return camera.get_projection_matrix() * camera.get_view_matrix();
      });
  m_geometry_wireframe_pipeline = std::move(pipeline_builder.build<PosUvVertex>(mge::DrawPrimitiveType::LINE));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "ui" / "cursor"))
      .add_uniform_update<glm::mat4>("projection_view", [&scene = m_scene]() {
        auto& camera = scene.get_current_camera();
        return camera.get_projection_matrix() * camera.get_view_matrix();
      });
  m_cursor_pipeline = std::move(pipeline_builder.build<GeometryVertex>(mge::DrawPrimitiveType::POINT));
  m_cursor_pipeline->dynamic_uniform_update("far_plane", m_scene.get_current_camera().get_far_plane());
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "ui" / "vertical_line"))
      .add_uniform_update<glm::mat4>("projection_view", [&scene = m_scene]() {
        auto& camera = scene.get_current_camera();
        return camera.get_projection_matrix() * camera.get_view_matrix();
      });
  m_vertical_line_pipeline = std::move(pipeline_builder.build<GeometryVertex>(mge::DrawPrimitiveType::POINT));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "bezier"))
      .add_uniform_update<glm::mat4>("projection_view",
                                     [&scene = m_scene]() {
                                       auto& camera = scene.get_current_camera();
                                       return camera.get_projection_matrix() * camera.get_view_matrix();
                                     })
      .add_uniform_update<glm::vec2>("window_size", [&window_size = m_window_size]() { return window_size; })
      .set_patch_count(4);
  m_bezier_pipeline = std::move(pipeline_builder.build<GeometryVertex>(mge::DrawPrimitiveType::PATCH));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "bezier_interp"))
      .add_uniform_update<glm::mat4>("projection_view",
                                     [&scene = m_scene]() {
                                       auto& camera = scene.get_current_camera();
                                       return camera.get_projection_matrix() * camera.get_view_matrix();
                                     })
      .add_uniform_update<glm::vec2>("window_size", [&window_size = m_window_size]() { return window_size; })
      .set_patch_count(1);
  m_bezier_c2_interp_pipeline =
      std::move(pipeline_builder.build<BezierCurveC2InterpVertex>(mge::DrawPrimitiveType::PATCH));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "bezier_surface"))
      .add_uniform_update<glm::mat4>("projection_view",
                                     [&scene = m_scene]() {
                                       auto& camera = scene.get_current_camera();
                                       return camera.get_projection_matrix() * camera.get_view_matrix();
                                     })
      .set_patch_count(16);
  m_bezier_surface_pipeline = std::move(pipeline_builder.build<GeometryVertex>(mge::DrawPrimitiveType::PATCH));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "bezier_poly"))
      .add_uniform_update<glm::mat4>("projection_view", [&scene = m_scene]() {
        auto& camera = scene.get_current_camera();
        return camera.get_projection_matrix() * camera.get_view_matrix();
      });
  m_bezier_polygon_pipeline = std::move(pipeline_builder.build<GeometryVertex>(mge::DrawPrimitiveType::LINE_STRIP));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "gregory" / "patch"))
      .add_uniform_update<glm::mat4>("projection_view",
                                     [&scene = m_scene]() {
                                       auto& camera = scene.get_current_camera();
                                       return camera.get_projection_matrix() * camera.get_view_matrix();
                                     })
      .set_patch_count(20);
  m_gregory_patch_pipeline = std::move(pipeline_builder.build<GeometryVertex>(mge::DrawPrimitiveType::PATCH));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "gregory" / "vectors"))
      .add_uniform_update<glm::mat4>("projection_view", [&scene = m_scene]() {
        auto& camera = scene.get_current_camera();
        return camera.get_projection_matrix() * camera.get_view_matrix();
      });
  m_gregory_vectors_pipeline = std::move(pipeline_builder.build<GeometryVertex>(mge::DrawPrimitiveType::LINE));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "bezier_poly"))
      .add_uniform_update<glm::mat4>("projection_view", [&scene = m_scene]() {
        auto& camera = scene.get_current_camera();
        return camera.get_projection_matrix() * camera.get_view_matrix();
      });
  m_bezier_grid_pipeline = std::move(pipeline_builder.build<GeometryVertex>(mge::DrawPrimitiveType::LINE));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "point"))
      .add_uniform_update<glm::mat4>("projection_view",
                                     [&scene = m_scene]() {
                                       auto& camera = scene.get_current_camera();
                                       return camera.get_projection_matrix() * camera.get_view_matrix();
                                     })
      .add_uniform_update<glm::vec2>("window_size", [&window_size = m_window_size]() { return window_size; });
  auto point_vertices = std::vector<GeometryVertex>{{{1.0f, 0.0f, 0.0f}},
                                                    {{0.9807852804032304f, 0.19509032201612825f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.9807852804032304f, 0.19509032201612825f, 0.0f}},
                                                    {{0.9238795325112867f, 0.3826834323650898f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.9238795325112867f, 0.3826834323650898f, 0.0f}},
                                                    {{0.8314696123025452f, 0.5555702330196022f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.8314696123025452f, 0.5555702330196022f, 0.0f}},
                                                    {{0.7071067811865476f, 0.7071067811865475f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.7071067811865476f, 0.7071067811865475f, 0.0f}},
                                                    {{0.5555702330196023f, 0.8314696123025452f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.5555702330196023f, 0.8314696123025452f, 0.0f}},
                                                    {{0.38268343236508984f, 0.9238795325112867f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.38268343236508984f, 0.9238795325112867f, 0.0f}},
                                                    {{0.19509032201612833f, 0.9807852804032304f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.19509032201612833f, 0.9807852804032304f, 0.0f}},
                                                    {{0.0f, 1.0f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.0f, 1.0f, 0.0f}},
                                                    {{-0.1950903220161282f, 0.9807852804032304f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.1950903220161282f, 0.9807852804032304f, 0.0f}},
                                                    {{-0.3826834323650897f, 0.9238795325112867f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.3826834323650897f, 0.9238795325112867f, 0.0f}},
                                                    {{-0.555570233019602f, 0.8314696123025455f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.555570233019602f, 0.8314696123025455f, 0.0f}},
                                                    {{-0.7071067811865475f, 0.7071067811865476f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.7071067811865475f, 0.7071067811865476f, 0.0f}},
                                                    {{-0.8314696123025453f, 0.5555702330196022f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.8314696123025453f, 0.5555702330196022f, 0.0f}},
                                                    {{-0.9238795325112867f, 0.3826834323650899f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.9238795325112867f, 0.3826834323650899f, 0.0f}},
                                                    {{-0.9807852804032304f, 0.1950903220161286f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.9807852804032304f, 0.1950903220161286f, 0.0f}},
                                                    {{-1.0f, 0.0f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-1.0f, 0.0f, 0.0f}},
                                                    {{-0.9807852804032304f, -0.19509032201612836f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.9807852804032304f, -0.19509032201612836f, 0.0f}},
                                                    {{-0.9238795325112868f, -0.38268343236508967f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.9238795325112868f, -0.38268343236508967f, 0.0f}},
                                                    {{-0.8314696123025455f, -0.555570233019602f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.8314696123025455f, -0.555570233019602f, 0.0f}},
                                                    {{-0.7071067811865477f, -0.7071067811865475f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.7071067811865477f, -0.7071067811865475f, 0.0f}},
                                                    {{-0.5555702330196022f, -0.8314696123025452f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.5555702330196022f, -0.8314696123025452f, 0.0f}},
                                                    {{-0.38268343236509034f, -0.9238795325112865f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.38268343236509034f, -0.9238795325112865f, 0.0f}},
                                                    {{-0.19509032201612866f, -0.9807852804032303f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{-0.19509032201612866f, -0.9807852804032303f, 0.0f}},
                                                    {{0.0f, -1.0f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.0f, -1.0f, 0.0f}},
                                                    {{0.1950903220161283f, -0.9807852804032304f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.1950903220161283f, -0.9807852804032304f, 0.0f}},
                                                    {{0.38268343236509f, -0.9238795325112866f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.38268343236509f, -0.9238795325112866f, 0.0f}},
                                                    {{0.5555702330196018f, -0.8314696123025455f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.5555702330196018f, -0.8314696123025455f, 0.0f}},
                                                    {{0.7071067811865474f, -0.7071067811865477f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.7071067811865474f, -0.7071067811865477f, 0.0f}},
                                                    {{0.8314696123025452f, -0.5555702330196022f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.8314696123025452f, -0.5555702330196022f, 0.0f}},
                                                    {{0.9238795325112865f, -0.3826834323650904f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.9238795325112865f, -0.3826834323650904f, 0.0f}},
                                                    {{0.9807852804032303f, -0.19509032201612872f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}},
                                                    {{0.9807852804032303f, -0.19509032201612872f, 0.0f}},
                                                    {{1.0f, 0.0f, 0.0f}},
                                                    {{0.0f, 0.0f, 0.0f}}};
  auto point_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  point_vertex_buffer->bind();
  point_vertex_buffer->copy(point_vertices);
  point_vertex_buffer->unbind();
  auto point_instance_buffer = std::make_unique<mge::Buffer<PointInstancedVertex>>(
      mge::Buffer<PointInstancedVertex>::Type::ARRAY, mge::Buffer<PointInstancedVertex>::UsageHint::DRAW,
      mge::Buffer<PointInstancedVertex>::FrequencyHint::DYNAMIC);
  auto point_vertex_array = std::make_unique<mge::InstancedVertexArray<GeometryVertex, PointInstancedVertex>>(
      std::move(point_vertex_buffer), GeometryVertex::get_vertex_attributes(), std::move(point_instance_buffer),
      PointInstancedVertex::get_vertex_attributes());
  m_point_pipeline = std::move(pipeline_builder.build_instanced<GeometryVertex, PointInstancedVertex>(
      mge::DrawPrimitiveType::TRIANGLE, std::move(point_vertex_array)));

  // Cursor
  create_cursor();

  // Mass center
  create_mass_center();

  // Vertial line
  create_vertical_line();

  // Anaglyph events
  AddEventListener(AnaglyphEvents::UpdateState, CadLayer::on_anaglyph_update_state, this);
  // Camera events
  mge::AddEventListener(mge::CameraEvents::CameraAngleChanged, CadLayer::on_camera_angle_changed, this);
  mge::AddEventListener(mge::CameraEvents::CameraPositionChanged, CadLayer::on_camera_position_changed, this);
  mge::AddEventListener(mge::CameraEvents::CameraZoom, CadLayer::on_camera_zoom, this);
  // Window events
  mge::AddEventListener(mge::WindowEvents::WindowFramebufferResized, CadLayer::on_window_framebuffer_resized, this);
  // Entity events
  mge::AddEventListener(mge::EntityEvents::Delete, CadLayer::on_delete_entity_by_id, this);
  mge::AddEventListener(mge::EntityEvents::QueryById, CadLayer::on_query_entity_by_id, this);
  mge::AddEventListener(mge::EntityEvents::QueryByPosition, CadLayer::on_query_entity_by_position, this);
  mge::AddEventListener(mge::EntityEvents::QueryByTag, CadLayer::on_query_entity_by_tag, this);
  // Point events
  AddEventListener(PointEvents::Add, CadLayer::on_add_point, this);
  AddEventListener(PointEvents::Collapse, CadLayer::on_collapse_points, this);
  // Torus events
  AddEventListener(TorusEvents::Add, CadLayer::on_add_torus, this);
  AddEventListener(TorusEvents::RadiusUpdated, CadLayer::on_torus_radius_updated, this);
  AddEventListener(TorusEvents::GridDensityUpdated, CadLayer::on_torus_grid_density_updated, this);
  // Bezier Curve C0 events
  AddEventListener(BezierCurveC0Events::Add, CadLayer::on_add_bezier_curve_c0, this);
  AddEventListener(BezierCurveC0Events::AddPoint, CadLayer::on_add_bezier_curve_c0_point, this);
  AddEventListener(BezierCurveC0Events::DeletePoint, CadLayer::on_delete_bezier_curve_c0_point, this);
  AddEventListener(BezierCurveC0Events::UpdatePolygonState, CadLayer::on_update_bezier_curve_c0_polygon_state, this);
  // Bezier Curve C2 events
  AddEventListener(BezierCurveC2Events::Add, CadLayer::on_add_bezier_curve_c2, this);
  AddEventListener(BezierCurveC2Events::AddPoint, CadLayer::on_add_bezier_curve_c2_point, this);
  AddEventListener(BezierCurveC2Events::DeletePoint, CadLayer::on_delete_bezier_curve_c2_point, this);
  AddEventListener(BezierCurveC2Events::UpdatePolygonState, CadLayer::on_update_bezier_curve_c2_polygon_state, this);
  AddEventListener(BezierCurveC2Events::UpdateBase, CadLayer::on_update_bezier_curve_c2_base, this);
  AddEventListener(BezierCurveC2Events::CreateBernsteinPoint, CadLayer::on_create_bernstein_point, this);
  // Bezier Curve C2 Interp events
  AddEventListener(BezierCurveC2InterpEvents::Add, CadLayer::on_add_bezier_curve_c2_interp, this);
  AddEventListener(BezierCurveC2InterpEvents::AddPoint, CadLayer::on_add_bezier_curve_c2_interp_point, this);
  AddEventListener(BezierCurveC2InterpEvents::DeletePoint, CadLayer::on_delete_bezier_curve_c2_interp_point, this);
  AddEventListener(BezierCurveC2InterpEvents::UpdatePolygonState,
                   CadLayer::on_update_bezier_curve_c2_interp_polygon_state, this);
  // Bezier Surface C0 events
  AddEventListener(BezierSurfaceC0Events::Add, CadLayer::on_add_bezier_surface_c0, this);
  AddEventListener(BezierSurfaceC0Events::UpdateGridState, CadLayer::on_update_bezier_surface_c0_grid_state, this);
  AddEventListener(BezierSurfaceC0Events::UpdateLineCount, CadLayer::on_update_bezier_surface_c0_line_count, this);
  // Bezier Surface C2 events
  AddEventListener(BezierSurfaceC2Events::Add, CadLayer::on_add_bezier_surface_c2, this);
  AddEventListener(BezierSurfaceC2Events::UpdateGridState, CadLayer::on_update_bezier_surface_c2_grid_state, this);
  AddEventListener(BezierSurfaceC2Events::UpdateLineCount, CadLayer::on_update_bezier_surface_c2_line_count, this);
  // Gregory Patch events
  AddEventListener(GregoryPatchEvents::FindHoles, CadLayer::on_find_hole, this);
  AddEventListener(GregoryPatchEvents::Add, CadLayer::on_add_gregory_patch, this);
  AddEventListener(GregoryPatchEvents::UpdateVectorsState, CadLayer::on_update_gregory_patch_vectors_state, this);
  AddEventListener(GregoryPatchEvents::UpdateLineCount, CadLayer::on_update_gregory_patch_line_count, this);
  // Intersection events
  AddEventListener(IntersectionEvents::FindStartingPoint, CadLayer::on_find_intersection_starting_point, this);
  AddEventListener(IntersectionEvents::Find, CadLayer::on_find_intersection, this);
  AddEventListener(IntersectionEvents::ConvertToInterCurve, CadLayer::on_convert_intersection_to_interp_curve, this);
  AddEventListener(IntersectionEvents::UpdateTrim, CadLayer::on_update_trim, this);
  AddEventListener(IntersectionEvents::UpdateHidePointsStatus, CadLayer::on_update_hide_points_status, this);
  // Cursor events
  AddEventListener(CursorEvents::Move, CadLayer::on_cursor_move, this);
  // Transform events
  AddEventListener(TransformEvents::TranslateToCursor, CadLayer::on_translate_to_cursor, this);
  AddEventListener(TransformEvents::RelativeTranslate, CadLayer::on_relative_translate, this);
  AddEventListener(TransformEvents::RelativeScale, CadLayer::on_relative_scale, this);
  AddEventListener(TransformEvents::RelativeRotate, CadLayer::on_relative_rotate, this);
  AddEventListener(TransformEvents::Translate, CadLayer::on_translate, this);
  AddEventListener(TransformEvents::Scale, CadLayer::on_scale, this);
  AddEventListener(TransformEvents::Rotate, CadLayer::on_rotate, this);
  // Tag events
  mge::AddEventListener(mge::TagEvents::Update, CadLayer::on_tag_updated, this);
  // Render mode events
  mge::AddEventListener(mge::RenderModeEvents::RenderModeUpdated, CadLayer::on_render_mode_updated, this);
  // Selection Events
  AddEventListener(SelectionEvents::SelectionUpdate, CadLayer::on_selection_updated, this);
  AddEventListener(SelectionEvents::UnselectAllEntities, CadLayer::on_unselect_all_entities, this);
  AddEventListener(SelectionEvents::Degrade, CadLayer::on_degrade_selection, this);
}

void CadLayer::update() {
  // destroy marked entities
  while (!m_to_be_destroyed.empty()) {
    auto id = m_to_be_destroyed.back();
    m_to_be_destroyed.pop_back();
    m_scene.get_entity(id).destroy();
    mge::DeletedEntityEvent event(id);
    mge::SendEvent(event);
  }

  // draw
  if (m_do_anaglyphs) {
    glBlendFunc(GL_ONE, GL_ONE);
    auto& anaglyph_camera = dynamic_cast<mge::AnaglyphCamera&>(m_scene.get_current_camera());
    // left eye
    anaglyph_camera.set_eye(mge::AnaglyphCamera::Eye::Left);
    m_geometry_wireframe_pipeline->dynamic_uniform_update("anaglyph_state", 1);
    m_geometry_wireframe_pipeline->run();
    m_geometry_solid_pipeline->dynamic_uniform_update("anaglyph_state", 1);
    m_geometry_solid_pipeline->run();
    m_bezier_pipeline->dynamic_uniform_update("anaglyph_state", 1);
    m_bezier_pipeline->run();
    m_bezier_c2_interp_pipeline->dynamic_uniform_update("anaglyph_state", 1);
    m_bezier_c2_interp_pipeline->run();
    m_bezier_polygon_pipeline->dynamic_uniform_update("anaglyph_state", 1);
    m_bezier_polygon_pipeline->run();
    m_bezier_grid_pipeline->dynamic_uniform_update("anaglyph_state", 1);
    m_bezier_grid_pipeline->run();
    m_bezier_surface_pipeline->dynamic_uniform_update("anaglyph_state", 1);
    m_bezier_surface_pipeline->dynamic_uniform_update("flip_uv", false);
    m_bezier_surface_pipeline->run();
    m_bezier_surface_pipeline->dynamic_uniform_update("flip_uv", true);
    m_bezier_surface_pipeline->run();
    m_gregory_patch_pipeline->dynamic_uniform_update("anaglyph_state", 1);
    m_gregory_patch_pipeline->dynamic_uniform_update("flip_uv", false);
    m_gregory_patch_pipeline->run();
    m_gregory_patch_pipeline->dynamic_uniform_update("flip_uv", true);
    m_gregory_patch_pipeline->run();
    m_gregory_vectors_pipeline->dynamic_uniform_update("anaglyph_state", 1);
    m_gregory_vectors_pipeline->run();
    m_point_pipeline->dynamic_uniform_update("anaglyph_state", 1);
    m_point_pipeline->run();
    m_cursor_pipeline->dynamic_uniform_update("anaglyph_state", 1);
    m_cursor_pipeline->run();
    m_vertical_line_pipeline->dynamic_uniform_update("anaglyph_state", 1);
    m_vertical_line_pipeline->run();
    glClear(GL_DEPTH_BUFFER_BIT);
    // right eye
    anaglyph_camera.set_eye(mge::AnaglyphCamera::Eye::Right);
    m_geometry_wireframe_pipeline->dynamic_uniform_update("anaglyph_state", 2);
    m_geometry_wireframe_pipeline->run();
    m_geometry_solid_pipeline->dynamic_uniform_update("anaglyph_state", 2);
    m_geometry_solid_pipeline->run();
    m_bezier_pipeline->dynamic_uniform_update("anaglyph_state", 2);
    m_bezier_pipeline->run();
    m_bezier_c2_interp_pipeline->dynamic_uniform_update("anaglyph_state", 2);
    m_bezier_c2_interp_pipeline->run();
    m_bezier_polygon_pipeline->dynamic_uniform_update("anaglyph_state", 2);
    m_bezier_polygon_pipeline->run();
    m_bezier_grid_pipeline->dynamic_uniform_update("anaglyph_state", 2);
    m_bezier_grid_pipeline->run();
    m_bezier_surface_pipeline->dynamic_uniform_update("anaglyph_state", 2);
    m_bezier_surface_pipeline->dynamic_uniform_update("flip_uv", false);
    m_bezier_surface_pipeline->run();
    m_bezier_surface_pipeline->dynamic_uniform_update("flip_uv", true);
    m_bezier_surface_pipeline->run();
    m_gregory_patch_pipeline->dynamic_uniform_update("anaglyph_state", 2);
    m_gregory_patch_pipeline->dynamic_uniform_update("flip_uv", false);
    m_gregory_patch_pipeline->run();
    m_gregory_patch_pipeline->dynamic_uniform_update("flip_uv", true);
    m_gregory_patch_pipeline->run();
    m_gregory_vectors_pipeline->dynamic_uniform_update("anaglyph_state", 2);
    m_gregory_vectors_pipeline->run();
    m_point_pipeline->dynamic_uniform_update("anaglyph_state", 2);
    m_point_pipeline->run();
    m_cursor_pipeline->dynamic_uniform_update("anaglyph_state", 2);
    m_cursor_pipeline->run();
    m_vertical_line_pipeline->dynamic_uniform_update("anaglyph_state", 2);
    m_vertical_line_pipeline->run();
  } else {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_geometry_wireframe_pipeline->dynamic_uniform_update("anaglyph_state", 0);
    m_geometry_wireframe_pipeline->run();
    m_geometry_solid_pipeline->dynamic_uniform_update("anaglyph_state", 0);
    m_geometry_solid_pipeline->run();
    m_bezier_pipeline->dynamic_uniform_update("anaglyph_state", 0);
    m_bezier_pipeline->run();
    m_bezier_c2_interp_pipeline->dynamic_uniform_update("anaglyph_state", 0);
    m_bezier_c2_interp_pipeline->run();
    m_bezier_polygon_pipeline->dynamic_uniform_update("anaglyph_state", 0);
    m_bezier_polygon_pipeline->run();
    m_bezier_grid_pipeline->dynamic_uniform_update("anaglyph_state", 0);
    m_bezier_grid_pipeline->run();
    m_bezier_surface_pipeline->dynamic_uniform_update("anaglyph_state", 0);
    m_bezier_surface_pipeline->dynamic_uniform_update("flip_uv", false);
    m_bezier_surface_pipeline->run();
    m_bezier_surface_pipeline->dynamic_uniform_update("flip_uv", true);
    m_bezier_surface_pipeline->run();
    m_gregory_patch_pipeline->dynamic_uniform_update("anaglyph_state", 0);
    m_gregory_patch_pipeline->dynamic_uniform_update("flip_uv", false);
    m_gregory_patch_pipeline->run();
    m_gregory_patch_pipeline->dynamic_uniform_update("flip_uv", true);
    m_gregory_patch_pipeline->run();
    m_gregory_vectors_pipeline->dynamic_uniform_update("anaglyph_state", 0);
    m_gregory_vectors_pipeline->run();
    m_point_pipeline->dynamic_uniform_update("anaglyph_state", 0);
    m_point_pipeline->run();
    m_cursor_pipeline->dynamic_uniform_update("anaglyph_state", 0);
    m_cursor_pipeline->run();
    m_vertical_line_pipeline->dynamic_uniform_update("anaglyph_state", 0);
    m_vertical_line_pipeline->run();
  }
}

glm::vec3 CadLayer::unproject_point(glm::vec2 pos) const {
  auto& camera = m_scene.get_current_camera();
  auto proj_view = camera.get_projection_matrix() * camera.get_view_matrix();
  glm::vec4 unprojected_point = glm::inverse(proj_view) * glm::vec4(pos, 0.995f, 1.0f);
  return glm::vec3(unprojected_point) / unprojected_point.w;
}

mge::OptionalEntity CadLayer::get_closest_selectible_entity(glm::vec2 screen_space_position) const {
  float min_dist = 0.3f;
  mge::EntityId closest_id = mge::EntityId(-1);
  auto& camera = m_scene.get_current_camera();
  auto proj_view = camera.get_projection_matrix() * camera.get_view_matrix();
  m_scene.foreach<>(entt::get<mge::TransformComponent, SelectibleComponent>, entt::exclude<>, [&](auto& entity) {
    if (!entity.template get_component<SelectibleComponent>().is_enabled()) return;
    auto& transform = entity.template get_component<mge::TransformComponent>();
    auto entity_position = proj_view * glm::vec4(transform.get_position(), 1.0f);
    auto corrected_entity_position = glm::vec3(entity_position) / entity_position.w;
    auto dist = glm::distance(screen_space_position, glm::vec2(corrected_entity_position));
    if (dist < min_dist) {
      min_dist = dist;
      closest_id = entity.get_id();
    }
  });

  if (m_scene.contains(closest_id)) {
    return m_scene.get_entity(closest_id);
  }

  return std::nullopt;
}

void CadLayer::update_mass_center() {
  m_mass_center.get().patch<MassCenterComponent>([](auto& center) { center.update_position(); });
  auto mass_center_position = m_mass_center.get().template get_component<MassCenterComponent>().get_position();
  auto mass_center_color = m_mass_center.get().template get_component<ColorComponent>().get_color();
  m_mass_center.get().template patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
      [&mass_center_position, &mass_center_color](auto& renderable) {
        renderable.set_instance_data({mass_center_position, mass_center_color});
      });
}

void CadLayer::update_point_instance_data(mge::Entity& entity) {
  auto point_position = entity.get_component<mge::TransformComponent>().get_position();
  auto point_color = entity.get_component<ColorComponent>().get_color();
  entity.patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
      [&point_position, &point_color](auto& renderable) {
        renderable.set_instance_data({point_position, point_color});
      });
}

void CadLayer::relative_translate(mge::Entity& entity, const glm::vec3& center, const glm::vec3& destination, std::list<mge::EntityId>& visited) {
  entity.run_and_propagate([offset = destination - center, &visited](auto& entity) {
    if (!entity.template has_component<mge::TransformComponent>()) return;
    if (std::find(visited.begin(), visited.end(), entity.get_id()) != visited.end()) return;
    visited.push_back(entity.get_id());
    entity.template patch<mge::TransformComponent>([&offset](auto& transform) { transform.translate(offset); });
  });
}

void CadLayer::relative_scale(mge::Entity& entity, const glm::vec3& center, const glm::vec3& scaling_factor) {
  entity.patch<mge::TransformComponent>([&scaling_factor, &center](auto& transform) {
    transform.scale(scaling_factor);
    glm::vec3 offset;
    offset.x =
        (scaling_factor.x > 1.0f) ? transform.get_position().x - center.x : center.x - transform.get_position().x;
    offset.x *= std::abs(scaling_factor.x - 1.0f);
    offset.y =
        (scaling_factor.y > 1.0f) ? transform.get_position().y - center.y : center.y - transform.get_position().y;
    offset.y *= std::abs(scaling_factor.y - 1.0f);
    offset.z =
        (scaling_factor.z > 1.0f) ? transform.get_position().z - center.z : center.z - transform.get_position().z;
    offset.z *= std::abs(scaling_factor.z - 1.0f);
    transform.translate(offset);
  });

  auto new_center = entity.get_component<mge::TransformComponent>().get_position();

  for (auto& child : entity.get_children()) {
    if (!child.get().has_component<mge::TransformComponent>()) continue;
    relative_scale(child, new_center, scaling_factor);
  }
}

void CadLayer::relative_rotate(mge::Entity& entity, const glm::vec3& center, const glm::quat& q) {
  entity.patch<mge::TransformComponent>([&center, &q](auto& transform) {
    transform.rotate(q);
    transform.set_position(center + q * (transform.get_position() - center));
  });

  for (auto& child : entity.get_children()) {
    if (!child.get().has_component<mge::TransformComponent>()) continue;
    relative_rotate(child, center, q);
  }
}

bool CadLayer::on_anaglyph_update_state(AnaglyphUpdateStateEvent& event) {
  m_do_anaglyphs = event.state;
  return false;
}

bool CadLayer::on_camera_angle_changed(mge::CameraAngleChangedEvent& event) {
  m_scene.get_current_camera().rotate(event.yaw, event.pitch, event.get_dt());
  return true;
}

bool CadLayer::on_camera_position_changed(mge::CameraPositionChangedEvent& event) {
  m_scene.get_current_camera().move({event.offset, 0.0f}, event.get_dt());
  return true;
}

bool CadLayer::on_camera_zoom(mge::CameraZoomEvent& event) {
  m_scene.get_current_camera().zoom(event.zoom, event.get_dt());
  return true;
}

bool CadLayer::on_window_framebuffer_resized(mge::WindowFramebufferResizedEvent& event) {
  m_window_size = {event.width, event.height};
  return true;
}

bool CadLayer::on_delete_entity_by_id(mge::DeleteEntityEvent& event) {
  if (!m_scene.contains(event.id)) return false;
  auto& entity = m_scene.get_entity(event.id);
  if (!entity.get_parents().empty() && entity.get_parents().begin()->get() != m_mass_center.get()) {
    return false;
  }

  m_to_be_destroyed.push_back(event.id);

  return true;
}

bool CadLayer::on_query_entity_by_id(mge::QueryEntityByIdEvent& event) {
  if (!m_scene.contains(event.id)) return false;
  event.entity = m_scene.get_entity(event.id);
  return true;
}

bool CadLayer::on_query_entity_by_position(mge::QueryEntityByPositionEvent& event) {
  event.entity = get_closest_selectible_entity(event.screen_space_position);
  return event.entity.has_value();
}

bool CadLayer::on_query_entity_by_tag(mge::QueryEntityByTagEvent& event) {
  m_scene.foreach<>(entt::get<mge::TagComponent>, entt::exclude<>, [&](auto& entity) {
    auto& tag = entity.template get_component<mge::TagComponent>();
    if (tag.get_tag() == event.tag) {
      event.entity = m_scene.get_entity(entity.get_id());
    }
  });
  return event.entity.has_value();
}

bool CadLayer::on_add_point(AddPointEvent& event) {
  auto pos = m_cursor.get().get_component<mge::TransformComponent>().get_position();
  auto& entity = create_point(pos);
  mge::AddedEntityEvent add_event(entity.get_id());
  mge::SendEvent(add_event);
  event.point = entity;
  return true;
}

bool CadLayer::on_collapse_points(CollapsePointsEvent& event) {
  auto pos = glm::vec3{0.0f};
  for (const auto& id : event.ids) {
    if (!m_scene.get_entity(id).has_component<PointComponent>()) return true;
    pos += m_scene.get_entity(id).get_component<mge::TransformComponent>().get_position();
  }
  pos /= static_cast<float>(event.ids.size());

  auto& middle_point = create_point(pos);
  for (const auto& id : event.ids) {
    auto& old_point = m_scene.get_entity(id);
    for (auto& parent : old_point.get_parents()) {
      if (parent.get().has_component<BezierCurveC0Component>())
        parent.get().patch<BezierCurveC0Component>([&old_point, &middle_point](auto& component){ component.swap_points(old_point, middle_point); });
      if (parent.get().has_component<BezierCurveC2Component>())
        parent.get().patch<BezierCurveC2Component>([&old_point, &middle_point](auto& component){ component.swap_points(old_point, middle_point); });
      if (parent.get().has_component<BezierCurveC2InterpComponent>())
        parent.get().patch<BezierCurveC2InterpComponent>([&old_point, &middle_point](auto& component){ component.swap_points(old_point, middle_point); });
      if (parent.get().has_component<BezierSurfaceC0Component>())
        parent.get().patch<BezierSurfaceC0Component>([&old_point, &middle_point](auto& component){ component.swap_points(old_point, middle_point); });
      if (parent.get().has_component<BezierSurfaceC2Component>())
        parent.get().patch<BezierSurfaceC2Component>([&old_point, &middle_point](auto& component){ component.swap_points(old_point, middle_point); });
    }
    
    mge::DeletedEntityEvent event(old_point.get_id());
    mge::SendEvent(event);
    m_scene.destroy_entity(old_point);
  }

  mge::AddedEntityEvent add_event(middle_point.get_id());
  mge::SendEvent(add_event);

  return true;
}

bool CadLayer::on_add_torus(AddTorusEvent& event) {
  auto pos = m_cursor.get().get_component<mge::TransformComponent>().get_position();
  auto& entity = create_torus(pos, event.inner_radius, event.outer_radius, event.inner_density, event.outer_density);
  mge::AddedEntityEvent add_event(entity.get_id());
  mge::SendEvent(add_event);
  return true;
}

bool CadLayer::on_torus_radius_updated(TorusRadiusUpdatedEvent& event) {
  auto& entity = m_scene.get_entity(event.id);
  entity.patch<TorusComponent>([&event](auto& torus) {
    torus.set_inner_radius(event.inner_radius);
    torus.set_outer_radius(event.outer_radius);
  });
  auto& torus = entity.get_component<TorusComponent>();
  entity.patch<mge::RenderableComponent<PosUvVertex>>([&torus](auto& renderable) {
    auto vertices = torus.generate_geometry();
    auto& vertex_buffer = renderable.get_vertex_array().get_vertex_buffer();
    vertex_buffer.bind();
    vertex_buffer.copy(vertices);
    vertex_buffer.unbind();
  });
  return true;
}

bool CadLayer::on_torus_grid_density_updated(TorusGridDensityUpdatedEvent& event) {
  auto& entity = m_scene.get_entity(event.id);
  entity.patch<TorusComponent>([&event](auto& torus) {
    torus.set_horizontal_density(event.inner_density);
    torus.set_vertical_density(event.outer_density);
  });
  auto& torus = entity.get_component<TorusComponent>();
  entity.patch<mge::RenderableComponent<PosUvVertex>>([&torus](auto& renderable) {
    auto vertices = torus.generate_geometry();
    auto& vertex_buffer = renderable.get_vertex_array().get_vertex_buffer();
    vertex_buffer.bind();
    vertex_buffer.copy(vertices);
    vertex_buffer.unbind();
    std::vector<unsigned int> indices;
    if (renderable.get_render_mode() == mge::RenderMode::SOLID) {
      indices = torus.generate_topology<mge::RenderMode::SOLID>();
    } else if (renderable.get_render_mode() == mge::RenderMode::WIREFRAME) {
      indices = torus.generate_topology<mge::RenderMode::WIREFRAME>();
    }
    auto& element_buffer = renderable.get_vertex_array().get_element_buffer();
    element_buffer.bind();
    element_buffer.copy(indices);
    element_buffer.unbind();
  });
  return true;
}

bool CadLayer::on_add_bezier_curve_c0(AddBezierCurveC0Event& event) {
  for (auto id : event.control_points) {
    if (!m_scene.get_entity(id).has_component<PointComponent>()) return false;
  }
  auto& entity = create_bezier_curve_c0(event.control_points);
  mge::AddedEntityEvent add_event(entity.get_id());
  mge::SendEvent(add_event);
  return true;
}

bool CadLayer::on_add_bezier_curve_c0_point(BezierCurveC0AddPointEvent& event) {
  auto& point = m_scene.get_entity(event.point_id);
  auto& bezier = m_scene.get_entity(event.bezier_id);
  if (!point.has_component<PointComponent>()) return false;
  point.patch<SelectibleComponent>([&bezier](auto& selectible) {
    selectible.set_selection(bezier.get_component<SelectibleComponent>().is_selected());
  });
  point.patch<ColorComponent>(
      [&bezier](auto& color) { color.set_color(bezier.get_component<ColorComponent>().get_color()); });
  bezier.patch<BezierCurveC0Component>([&point](auto& component) { component.add_point(point); });
  return true;
}

bool CadLayer::on_delete_bezier_curve_c0_point(BezierCurveC0DeletePointEvent& event) {
  auto& point = m_scene.get_entity(event.point_id);
  auto& bezier = m_scene.get_entity(event.bezier_id);
  if (!point.has_component<PointComponent>()) return false;
  point.patch<SelectibleComponent>([&bezier](auto& selectible) { selectible.set_selection(false); });
  auto color = point.get_component<SelectibleComponent>().get_regular_color();
  point.patch<ColorComponent>([&color](auto& color_component) { color_component.set_color(color); });
  bezier.remove_child(point);
  bezier.patch<BezierCurveC0Component>([&point](auto& component) { component.remove_point(point); });
  return true;
}

bool CadLayer::on_update_bezier_curve_c0_polygon_state(BezierCurveC0UpdatePolygonStateEvent& event) {
  m_scene.get_entity(event.id).patch<BezierCurveC0Component>(
      [&event](auto& bezier) { bezier.set_polygon_status(event.state); });
  return true;
}

bool CadLayer::on_add_bezier_curve_c2(AddBezierCurveC2Event& event) {
  for (auto id : event.control_points) {
    if (!m_scene.get_entity(id).has_component<PointComponent>()) return false;
  }
  auto& entity = create_bezier_curve_c2(event.control_points);
  mge::AddedEntityEvent add_event(entity.get_id());
  mge::SendEvent(add_event);
  return true;
}

bool CadLayer::on_add_bezier_curve_c2_point(BezierCurveC2AddPointEvent& event) {
  auto& point = m_scene.get_entity(event.point_id);
  auto& bezier = m_scene.get_entity(event.bezier_id);
  if (!point.has_component<PointComponent>()) return false;
  point.patch<SelectibleComponent>([&bezier](auto& selectible) {
    selectible.set_selection(bezier.get_component<SelectibleComponent>().is_selected());
  });
  point.patch<ColorComponent>(
      [&bezier](auto& color) { color.set_color(bezier.get_component<ColorComponent>().get_color()); });
  bezier.patch<BezierCurveC2Component>([&point](auto& component) { component.add_point(point); });
  return true;
}

bool CadLayer::on_delete_bezier_curve_c2_point(BezierCurveC2DeletePointEvent& event) {
  auto& point = m_scene.get_entity(event.point_id);
  auto& bezier = m_scene.get_entity(event.bezier_id);
  if (!point.has_component<PointComponent>()) return false;
  point.patch<SelectibleComponent>([&bezier](auto& selectible) { selectible.set_selection(false); });
  auto color = point.get_component<SelectibleComponent>().get_regular_color();
  point.patch<ColorComponent>([&color](auto& color_component) { color_component.set_color(color); });
  bezier.remove_child(point);
  bezier.patch<BezierCurveC2Component>([&point](auto& component) { component.remove_point(point); });
  return true;
}

bool CadLayer::on_update_bezier_curve_c2_polygon_state(BezierCurveC2UpdatePolygonStateEvent& event) {
  m_scene.get_entity(event.id).patch<BezierCurveC2Component>(
      [&event](auto& bezier) { bezier.set_polygon_status(event.state); });
  return true;
}

bool CadLayer::on_update_bezier_curve_c2_base(BezierCurveC2UpdateBaseEvent& event) {
  m_scene.get_entity(event.id).patch<BezierCurveC2Component>([&event](auto& bezier) { bezier.set_base(event.base); });
  return true;
}

bool CadLayer::on_create_bernstein_point(CreateBernsteinPointEvent& event) {
  auto& entity = m_scene.create_entity();
  entity.template add_component<PointComponent>();
  entity.template add_component<mge::TransformComponent>();
  auto point_position = entity.get_component<mge::TransformComponent>().get_position();
  entity.template add_component<SelectibleComponent>(glm::vec3{0.0f, 1.0f, 1.0f}, glm::vec3{0.0f, 0.0f, 1.0f});
  entity.template add_component<ColorComponent>(entity.get_component<SelectibleComponent>().get_regular_color());
  auto position = entity.get_component<mge::TransformComponent>().get_position();
  auto color = entity.get_component<ColorComponent>().get_color();
  entity.template add_component<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
      mge::InstancedRenderPipelineMap<GeometryVertex, PointInstancedVertex>{
          {mge::RenderMode::SOLID, *m_point_pipeline}},
      mge::RenderMode::SOLID, PointInstancedVertex(position, color));
  m_point_pipeline->update_instance_buffer();
  entity.register_on_update<mge::TransformComponent>(&CadLayer::update_point_instance_data, this);
  entity.register_on_update<ColorComponent>(&CadLayer::update_point_instance_data, this);
  event.bernstein_point = entity;
  mge::AddedEntityEvent add_event(entity.get_id());
  mge::SendEvent(add_event);
  return true;
}

bool CadLayer::on_add_bezier_curve_c2_interp(AddBezierCurveC2InterpEvent& event) {
  for (auto id : event.control_points) {
    if (!m_scene.get_entity(id).has_component<PointComponent>()) return false;
  }
  auto& entity = create_bezier_curve_c2_interp(event.control_points);
  mge::AddedEntityEvent add_event(entity.get_id());
  mge::SendEvent(add_event);
  return true;
}

bool CadLayer::on_add_bezier_curve_c2_interp_point(BezierCurveC2InterpAddPointEvent& event) {
  auto& point = m_scene.get_entity(event.point_id);
  auto& bezier = m_scene.get_entity(event.bezier_id);
  if (!point.has_component<PointComponent>()) return false;
  point.patch<SelectibleComponent>([&bezier](auto& selectible) {
    selectible.set_selection(bezier.get_component<SelectibleComponent>().is_selected());
  });
  point.patch<ColorComponent>(
      [&bezier](auto& color) { color.set_color(bezier.get_component<ColorComponent>().get_color()); });
  bezier.patch<BezierCurveC2InterpComponent>([&point](auto& component) { component.add_point(point); });
  return true;
}

bool CadLayer::on_delete_bezier_curve_c2_interp_point(BezierCurveC2InterpDeletePointEvent& event) {
  auto& point = m_scene.get_entity(event.point_id);
  auto& bezier = m_scene.get_entity(event.bezier_id);
  if (!point.has_component<PointComponent>()) return false;
  point.patch<SelectibleComponent>([&bezier](auto& selectible) { selectible.set_selection(false); });
  auto color = point.get_component<SelectibleComponent>().get_regular_color();
  point.patch<ColorComponent>([&color](auto& color_component) { color_component.set_color(color); });
  bezier.remove_child(point);
  bezier.patch<BezierCurveC2InterpComponent>([&point](auto& component) { component.remove_point(point); });
  return true;
}

bool CadLayer::on_update_bezier_curve_c2_interp_polygon_state(BezierCurveC2InterpUpdatePolygonStateEvent& event) {
  m_scene.get_entity(event.id).patch<BezierCurveC2InterpComponent>(
      [&event](auto& bezier) { bezier.set_polygon_status(event.state); });
  return true;
}

bool CadLayer::on_add_bezier_surface_c0(AddBezierSurfaceC0Event& event) {
  auto pos = m_cursor.get().get_component<mge::TransformComponent>().get_position();
  auto& entity = create_bezier_surface_c0(pos, event.wrapping, event.patch_count_u, event.patch_count_v, event.size_u,
                                          event.size_v);
  mge::AddedEntityEvent add_event(entity.get_id());
  mge::SendEvent(add_event);
  return true;
}

bool CadLayer::on_update_bezier_surface_c0_grid_state(BezierSurfaceC0UpdateGridStateEvent& event) {
  m_scene.get_entity(event.id).patch<BezierSurfaceC0Component>(
      [&event](auto& bezier) { bezier.set_grid_status(event.state); });
  return true;
}

bool CadLayer::on_update_bezier_surface_c0_line_count(BezierSurfaceC0UpdateLineCountEvent& event) {
  m_scene.get_entity(event.id).patch<BezierSurfaceC0Component>(
      [&event](auto& bezier) { bezier.set_line_count(event.line_count); });
  return true;
}

bool CadLayer::on_add_bezier_surface_c2(AddBezierSurfaceC2Event& event) {
  auto pos = m_cursor.get().get_component<mge::TransformComponent>().get_position();
  auto& entity = create_bezier_surface_c2(pos, event.wrapping, event.patch_count_u, event.patch_count_v, event.size_u,
                                          event.size_v);
  mge::AddedEntityEvent add_event(entity.get_id());
  mge::SendEvent(add_event);
  return true;
}

bool CadLayer::on_update_bezier_surface_c2_grid_state(BezierSurfaceC2UpdateGridStateEvent& event) {
  m_scene.get_entity(event.id).patch<BezierSurfaceC2Component>(
      [&event](auto& bezier) { bezier.set_grid_status(event.state); });
  return true;
}

bool CadLayer::on_update_bezier_surface_c2_line_count(BezierSurfaceC2UpdateLineCountEvent& event) {
  m_scene.get_entity(event.id).patch<BezierSurfaceC2Component>(
      [&event](auto& bezier) { bezier.set_line_count(event.line_count); });
  return true;
}

bool CadLayer::on_update_gregory_patch_vectors_state(GregoryPatchUpdateVectorsStateEvent& event) {
  m_scene.get_entity(event.id).patch<GregoryPatchComponent>(
    [&event](auto& gregory) { gregory.set_vectors_status(event.state); });
return true;
}

bool CadLayer::on_update_gregory_patch_line_count(GregoryPatchUpdateLineCountEvent& event) {
  m_scene.get_entity(event.id).patch<GregoryPatchComponent>(
    [&event](auto& gregory) { gregory.set_line_count(event.line_count); });
return true;
}

bool CadLayer::on_find_hole(FindHoleEvent& event) {
  mge::EntityVector surfaces = {};
  for (const auto& id : event.patch_ids) {
    if (m_scene.get_entity(id).has_component<BezierSurfaceC0Component>()) surfaces.push_back(m_scene.get_entity(id));
  }
  event.hole_ids = m_gregory_patch_builder.find_holes(surfaces);
  return true;
}

bool CadLayer::on_add_gregory_patch(AddGregoryPatchEvent& event) {
  auto data = m_gregory_patch_builder.fill_hole(event.hole_ids);
  auto& entity = create_gregory_patch(data);
  mge::AddedEntityEvent add_event(entity.get_id());
  mge::SendEvent(add_event);
  return true;
}

bool CadLayer::on_find_intersection_starting_point(FindIntersectionStartingPointEvent& event) {
  if (event.cursor) {
    event.starting_point = m_intersection_builder.find_starting_point(m_scene.get_entity(event.s1), m_scene.get_entity(event.s2), m_cursor.get().get_component<mge::TransformComponent>().get_position());
  }
  else {
    event.starting_point = m_intersection_builder.find_starting_point(m_scene.get_entity(event.s1), m_scene.get_entity(event.s2));
  }
  
  return true;
}

bool CadLayer::on_find_intersection(FindIntersectionEvent& event) {
  auto [points1, points2] = m_intersection_builder.find(m_scene.get_entity(event.s1), m_scene.get_entity(event.s2), event.starting_point.first, event.starting_point.second, event.newton_factor, event.max_dist, event.rough);
  if (points1.empty() || points2.empty()) return false;
  auto& entity = create_intersection(m_scene.get_entity(event.s1), m_scene.get_entity(event.s2), points1, points2);
  mge::AddedEntityEvent add_event(entity.get_id());
  mge::SendEvent(add_event);
  return true;
}

bool CadLayer::on_convert_intersection_to_interp_curve(ConvertIntersectionToInterpCurveEvent& event) {
  auto& intersection = m_scene.get_entity(event.intersection);
  std::vector<mge::EntityId> points;
  for (const auto& pos : intersection.get_component<IntersectionComponent>().get_points()) {
    points.push_back(create_point(pos).get_id());
    mge::AddedEntityEvent add_event(points.back());
    mge::SendEvent(add_event);
  }
  points.push_back(points.front());
  auto& curve = create_bezier_curve_c2_interp(points);
  mge::AddedEntityEvent add_event(curve.get_id());
  mge::SendEvent(add_event);
  m_to_be_destroyed.push_back(intersection.get_id());
  
  return true;
}

bool CadLayer::on_update_trim(UpdateTrimEvent& event) {
  auto& intersection_entity = m_scene.get_entity(event.intersection);
  intersection_entity.patch<IntersectionComponent>([uv=event.uv, first=event.first](auto& intersection_component){
      intersection_component.update_trim(uv, first);
  });
  return true;
}

bool CadLayer::on_update_hide_points_status(UpdateHidePointsStatusEvent& event) {
  auto& intersection_entity = m_scene.get_entity(event.intersection);
  intersection_entity.patch<IntersectionComponent>([status=event.status](auto& intersection){
    intersection.set_hide_points_status(status);
  });
  return true;
}

bool CadLayer::on_cursor_move(CursorMoveEvent& event) {
  m_cursor.get().patch<mge::TransformComponent>(
      [this, &event](auto& transform) { transform.set_position(unproject_point(event.screen_space_position)); });
  return true;
}

bool CadLayer::on_translate_to_cursor(TranslateToCursorEvent& event) {
  glm::vec3 center = m_mass_center.get().get_component<MassCenterComponent>().get_position();
  glm::vec3 cursor = m_cursor.get().get_component<mge::TransformComponent>().get_position();
  std::list<mge::EntityId> visited = {};

  for (auto id : event.ids) {
    auto& entity = m_scene.get_entity(id);
    if (!entity.has_component<mge::TransformComponent>()) continue;
    relative_translate(entity, center, cursor, visited);
  }

  update_mass_center();

  return true;
}

bool CadLayer::on_relative_translate(RelativeTranslateEvent& event) {
  glm::vec3 center = m_mass_center.get().get_component<MassCenterComponent>().get_position();
  std::list<mge::EntityId> visited = {};

  for (auto id : event.ids) {
    auto& entity = m_scene.get_entity(id);
    if (!entity.has_component<mge::TransformComponent>()) continue;
    relative_translate(entity, center, event.translation, visited);
  }

  update_mass_center();

  return true;
}

bool CadLayer::on_relative_scale(RelativeScaleEvent& event) {
  glm::vec3 center = m_mass_center.get().get_component<MassCenterComponent>().get_position();

  for (auto id : event.ids) {
    auto& entity = m_scene.get_entity(id);
    if (!entity.has_component<mge::TransformComponent>()) continue;
    relative_scale(entity, center, event.scaling);
  }

  update_mass_center();

  return true;
}

bool CadLayer::on_relative_rotate(RelativeRotateEvent& event) {
  glm::vec3 center = m_mass_center.get().get_component<MassCenterComponent>().get_position();

  for (auto id : event.ids) {
    auto& entity = m_scene.get_entity(id);
    if (!entity.has_component<mge::TransformComponent>()) continue;
    relative_rotate(entity, center, event.quat);
  }

  update_mass_center();

  return true;
}

bool CadLayer::on_translate(TranslateEvent& event) {
  auto& entity = m_scene.get_entity(event.id);
  auto center = entity.get_component<mge::TransformComponent>().get_position();
  std::list<mge::EntityId> visited = {};

  relative_translate(entity, center, event.destination, visited);

  update_mass_center();

  return true;
}

bool CadLayer::on_scale(ScaleEvent& event) {
  auto& entity = m_scene.get_entity(event.id);
  auto center = entity.get_component<mge::TransformComponent>().get_position();
  auto old_scale = entity.get_component<mge::TransformComponent>().get_scale();

  relative_scale(entity, center, event.scale / old_scale);

  update_mass_center();

  return true;
}

bool CadLayer::on_rotate(RotateEvent& event) {
  auto& entity = m_scene.get_entity(event.id);
  auto center = entity.get_component<mge::TransformComponent>().get_position();
  auto q1 = glm::quat(entity.get_component<mge::TransformComponent>().get_rotation());
  auto q2 = event.rotation;

  relative_rotate(entity, center, glm::inverse(q1) * q2);

  update_mass_center();

  return true;
}

bool CadLayer::on_tag_updated(mge::TagUpdateEvent& event) {
  auto& entity = m_scene.get_entity(event.id);
  entity.patch<mge::TagComponent>([&event](auto& tag) { tag.set_tag(event.tag); });
  return true;
}

bool CadLayer::on_render_mode_updated(mge::RenderModeUpdatedEvent& event) {
  auto& entity = m_scene.get_entity(event.id);
  entity.patch<mge::RenderableComponent<PosUvVertex>>(
      [&event](auto& renderable) { renderable.set_render_mode(event.render_mode); });
  if (entity.has_component<TorusComponent>()) {
    auto& torus = entity.get_component<TorusComponent>();
    entity.patch<mge::RenderableComponent<PosUvVertex>>([&torus](auto& renderable) {
      auto vertices = torus.generate_geometry();
      auto& vertex_buffer = renderable.get_vertex_array().get_vertex_buffer();
      vertex_buffer.bind();
      vertex_buffer.copy(vertices);
      vertex_buffer.unbind();
      std::vector<unsigned int> indices;
      if (renderable.get_render_mode() == mge::RenderMode::SOLID) {
        indices = torus.generate_topology<mge::RenderMode::SOLID>();
      } else if (renderable.get_render_mode() == mge::RenderMode::WIREFRAME) {
        indices = torus.generate_topology<mge::RenderMode::WIREFRAME>();
      }
      auto& element_buffer = renderable.get_vertex_array().get_element_buffer();
      element_buffer.bind();
      element_buffer.copy(indices);
      element_buffer.unbind();
    });
  }
  return true;
}

bool CadLayer::on_selection_updated(SelectionUpdateEvent& event) {
  if (!m_scene.contains(event.id)) return false;
  auto& entity = m_scene.get_entity(event.id);
  if (entity.get_component<SelectibleComponent>().is_selected() == event.selection) return false;

  entity.patch<SelectibleComponent>([&event](auto& selectible) { selectible.set_selection(event.selection); });
  glm::vec3 color;
  if (event.selection) {
    color = entity.get_component<SelectibleComponent>().get_selected_color();
  } else {
    color = entity.get_component<SelectibleComponent>().get_regular_color();
  }
  entity.patch<ColorComponent>([&color](auto& color_component) { color_component.set_color(color); });

  if (entity.has_component<mge::TransformComponent>() && event.is_parent) {
    if (event.selection) {
      m_mass_center.get().add_child(entity);
      m_mass_center.get().patch<MassCenterComponent>(
          [&entity, &event](auto& mass_center) { mass_center.add_entity(entity); });
    } else {
      m_mass_center.get().remove_child(entity);
      m_mass_center.get().patch<MassCenterComponent>(
          [&entity, &event](auto& mass_center) { mass_center.remove_entity(entity); });
    }
    update_mass_center();
    if (m_mass_center.get().get_component<MassCenterComponent>().get_count() > 1) {
      m_mass_center.get().patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
          [](auto& renderable) { renderable.enable(); });
    } else {
      m_mass_center.get().patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
          [](auto& renderable) { renderable.disable(); });
    }
  }
  return true;
}

bool CadLayer::on_unselect_all_entities(UnselectAllEntitiesEvent& event) {
  m_mass_center.get().patch<MassCenterComponent>([](auto& mass_center) { mass_center.remove_all_entities(); });
  m_mass_center.get().remove_all_children();
  m_scene.foreach<>(entt::get<SelectibleComponent>, entt::exclude<>, [&](mge::Entity& entity) {
    if (!entity.get_component<SelectibleComponent>().is_selected()) return;
    entity.patch<SelectibleComponent>([](auto& selectible) { selectible.set_selection(false); });
  });
  m_scene.foreach<>(entt::get<SelectibleComponent, ColorComponent>, entt::exclude<>, [&](mge::Entity& entity) {
    auto color = entity.get_component<SelectibleComponent>().get_regular_color();
    entity.patch<ColorComponent>([&color](auto& color_component) { color_component.set_color(color); });
  });
  m_mass_center.get().patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
      [](auto& renderable) { renderable.disable(); });
  return true;
}

bool CadLayer::on_degrade_selection(DegradeSelectionEvent& event) {
  auto& entity = m_scene.get_entity(event.id);
  m_mass_center.get().patch<MassCenterComponent>([&entity](auto& mass_center) { mass_center.remove_entity(entity); });
  m_mass_center.get().remove_child(entity);
  if (m_mass_center.get().get_component<MassCenterComponent>().get_count() > 1) {
    m_mass_center.get().patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
        [](auto& renderable) { renderable.enable(); });
  } else {
    m_mass_center.get().patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
        [](auto& renderable) { renderable.disable(); });
  }
  return true;
}

mge::Entity& CadLayer::create_point(const glm::vec3& pos) {
  auto& entity = m_scene.create_entity();
  entity.add_component<PointComponent>();
  entity.add_component<mge::TagComponent>(PointComponent::get_new_name());
  entity.add_component<mge::TransformComponent>(pos);
  entity.add_component<SelectibleComponent>();
  entity.add_component<ColorComponent>();
  auto color = entity.get_component<ColorComponent>().get_color();
  auto point_data = PointInstancedVertex{pos, color};
  entity.add_component<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
      mge::InstancedRenderPipelineMap<GeometryVertex, PointInstancedVertex>{
          {mge::RenderMode::SOLID, *m_point_pipeline}},
      mge::RenderMode::SOLID, point_data);
  entity.register_on_update<mge::TransformComponent>(&CadLayer::update_point_instance_data, this);
  entity.register_on_update<ColorComponent>(&CadLayer::update_point_instance_data, this);
  return entity;
}

mge::Entity& CadLayer::create_torus(const glm::vec3& pos, float inner_radius, float outer_radius,
                                    unsigned int inner_density, unsigned int outer_density) {
  auto& entity = m_scene.create_entity();
  entity.template add_component<TorusComponent>(inner_radius, outer_radius, inner_density, outer_density);
  entity.template add_component<mge::TagComponent>(TorusComponent::get_new_name());
  entity.template add_component<mge::TransformComponent>(pos);
  entity.template add_component<SelectibleComponent>();
  entity.template add_component<ColorComponent>();
  auto vertices = entity.get_component<TorusComponent>().generate_geometry();
  auto vertex_buffer = std::make_unique<mge::Buffer<PosUvVertex>>();
  vertex_buffer->bind();
  vertex_buffer->copy(vertices);
  vertex_buffer->unbind();
  auto indices = entity.get_component<TorusComponent>().generate_topology<mge::RenderMode::WIREFRAME>();
  auto element_buffer = std::make_unique<mge::ElementBuffer>(mge::ElementBuffer::Type::ELEMENT_ARRAY);
  element_buffer->bind();
  element_buffer->copy(indices);
  element_buffer->unbind();
  auto vertex_array = std::make_unique<mge::VertexArray<PosUvVertex>>(
      std::move(vertex_buffer), PosUvVertex::get_vertex_attributes(), std::move(element_buffer));
  entity.template add_component<mge::RenderableComponent<PosUvVertex>>(
      mge::RenderPipelineMap<PosUvVertex>{{mge::RenderMode::WIREFRAME, *m_geometry_wireframe_pipeline},
                                             {mge::RenderMode::SOLID, *m_geometry_solid_pipeline}},
      mge::RenderMode::WIREFRAME, std::move(vertex_array), [&entity](auto& render_pipeline) {
        render_pipeline.template dynamic_uniform_update_and_commit<glm::mat4>(
            "model", [&entity]() { return entity.get_component<mge::TransformComponent>().get_model_mat(); });
        render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
            "color", [&entity]() { return entity.get_component<ColorComponent>().get_color(); });
        auto parent = std::find_if(entity.get_parents().begin(), entity.get_parents().end(), [](const auto& parent) { return parent.get().template has_component<IntersectionComponent>(); });
        bool useTexture = parent != entity.get_parents().end();
        render_pipeline.template dynamic_uniform_update_and_commit<int>("useTexture", [&useTexture]() { return static_cast<int>(useTexture); });
        if (useTexture) {
          parent->get().template patch<IntersectionComponent>([&entity](auto& intersection_component){
            intersection_component.use_texture_for(entity, 0);
          });
        }
      });
  return entity;
}

mge::Entity& CadLayer::create_bezier_curve_c0(const std::vector<mge::EntityId>& points) {
  auto& bezier_entity = m_scene.create_entity();
  auto& polygon_entity = m_scene.create_entity();
  for (auto id : points) {
    bezier_entity.add_child(m_scene.get_entity(id));
  }
  bezier_entity.add_component<mge::TagComponent>(BezierCurveC0Component::get_new_name());
  bezier_entity.add_component<BezierCurveC0Component>(bezier_entity.get_children(), bezier_entity, polygon_entity);
  bezier_entity.add_component<mge::TransformComponent>();
  bezier_entity.add_component<SelectibleComponent>();
  bezier_entity.add_component<ColorComponent>();
  auto& bezier = bezier_entity.get_component<BezierCurveC0Component>();
  auto curve_vertices = bezier.generate_geometry();
  auto curve_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  curve_vertex_buffer->bind();
  curve_vertex_buffer->copy(curve_vertices);
  curve_vertex_buffer->unbind();
  auto curve_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(std::move(curve_vertex_buffer),
                                                                               GeometryVertex::get_vertex_attributes());
  bezier_entity.add_component<mge::RenderableComponent<GeometryVertex>>(
      mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_pipeline}},
      mge::RenderMode::WIREFRAME, std::move(curve_vertex_array), [&bezier_entity](auto& render_pipeline) {
        render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
            "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
      });
  auto polygon_vertices = bezier.generate_polygon_geometry();
  auto polygon_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  polygon_vertex_buffer->bind();
  polygon_vertex_buffer->copy(polygon_vertices);
  polygon_vertex_buffer->unbind();
  auto polygon_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(polygon_vertex_buffer), GeometryVertex::get_vertex_attributes());
  polygon_entity
      .add_component<mge::RenderableComponent<GeometryVertex>>(
          mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_polygon_pipeline}},
          mge::RenderMode::WIREFRAME, std::move(polygon_vertex_array),
          [&bezier_entity](auto& render_pipeline) {
            render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
                "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
          })
      .disable();
  bezier_entity.patch<BezierCurveC0Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<BezierCurveC0Component, BezierCurveC0Component>(
        &BezierCurveC0Component::update_curve, &bezier_component);
  });
  bezier_entity.patch<BezierCurveC0Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<mge::TransformComponent, BezierCurveC0Component>(
        &BezierCurveC0Component::update_curve_by_self, &bezier_component);
  });
  return bezier_entity;
}

mge::Entity& CadLayer::create_bezier_curve_c2(const std::vector<mge::EntityId>& points) {
  auto& bezier_entity = m_scene.create_entity();
  auto& polygon_entity = m_scene.create_entity();
  for (auto id : points) {
    bezier_entity.add_child(m_scene.get_entity(id));
  }
  bezier_entity.add_component<mge::TagComponent>(BezierCurveC2Component::get_new_name());
  bezier_entity.add_component<BezierCurveC2Component>(bezier_entity.get_children(), bezier_entity, polygon_entity);
  bezier_entity.add_component<mge::TransformComponent>();
  bezier_entity.add_component<SelectibleComponent>();
  bezier_entity.add_component<ColorComponent>();
  auto& bezier = bezier_entity.get_component<BezierCurveC2Component>();
  auto curve_vertices = bezier.generate_geometry();
  auto curve_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  curve_vertex_buffer->bind();
  curve_vertex_buffer->copy(curve_vertices);
  curve_vertex_buffer->unbind();
  auto curve_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(std::move(curve_vertex_buffer),
                                                                               GeometryVertex::get_vertex_attributes());
  bezier_entity.add_component<mge::RenderableComponent<GeometryVertex>>(
      mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_pipeline}},
      mge::RenderMode::WIREFRAME, std::move(curve_vertex_array), [&bezier_entity](auto& render_pipeline) {
        render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
            "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
      });
  auto polygon_vertices = bezier.generate_polygon_geometry();
  auto polygon_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  polygon_vertex_buffer->bind();
  polygon_vertex_buffer->copy(polygon_vertices);
  polygon_vertex_buffer->unbind();
  auto polygon_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(polygon_vertex_buffer), GeometryVertex::get_vertex_attributes());
  polygon_entity
      .add_component<mge::RenderableComponent<GeometryVertex>>(
          mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_polygon_pipeline}},
          mge::RenderMode::WIREFRAME, std::move(polygon_vertex_array),
          [&bezier_entity](auto& render_pipeline) {
            render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
                "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
          })
      .disable();
  bezier_entity.patch<BezierCurveC2Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<BezierCurveC2Component, BezierCurveC2Component>(
        &BezierCurveC2Component::update_curve, &bezier_component);
  });
  bezier_entity.patch<BezierCurveC2Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<mge::TransformComponent, BezierCurveC2Component>(
        &BezierCurveC2Component::update_curve_by_self, &bezier_component);
  });
  return bezier_entity;
}

mge::Entity& CadLayer::create_bezier_curve_c2_interp(const std::vector<mge::EntityId>& points) {
  auto& bezier_entity = m_scene.create_entity();
  auto& polygon_entity = m_scene.create_entity();
  for (auto id : points) {
    bezier_entity.add_child(m_scene.get_entity(id));
  }
  bezier_entity.add_component<mge::TagComponent>(BezierCurveC2InterpComponent::get_new_name());
  bezier_entity.add_component<BezierCurveC2InterpComponent>(bezier_entity.get_children(), bezier_entity,
                                                            polygon_entity);
  bezier_entity.add_component<mge::TransformComponent>();
  bezier_entity.template add_component<SelectibleComponent>();
  bezier_entity.template add_component<ColorComponent>();
  auto& bezier = bezier_entity.get_component<BezierCurveC2InterpComponent>();
  auto curve_vertices = bezier.generate_geometry();
  auto curve_vertex_buffer = std::make_unique<mge::Buffer<BezierCurveC2InterpVertex>>();
  curve_vertex_buffer->bind();
  curve_vertex_buffer->copy(curve_vertices);
  curve_vertex_buffer->unbind();
  auto curve_vertex_array = std::make_unique<mge::VertexArray<BezierCurveC2InterpVertex>>(
      std::move(curve_vertex_buffer), BezierCurveC2InterpVertex::get_vertex_attributes());
  bezier_entity.add_component<mge::RenderableComponent<BezierCurveC2InterpVertex>>(
      mge::RenderPipelineMap<BezierCurveC2InterpVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_c2_interp_pipeline}},
      mge::RenderMode::WIREFRAME, std::move(curve_vertex_array), [&bezier_entity](auto& render_pipeline) {
        render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
            "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
      });
  auto polygon_vertices = bezier.generate_polygon_geometry();
  auto polygon_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  polygon_vertex_buffer->bind();
  polygon_vertex_buffer->copy(polygon_vertices);
  polygon_vertex_buffer->unbind();
  auto polygon_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(polygon_vertex_buffer), GeometryVertex::get_vertex_attributes());
  polygon_entity
      .add_component<mge::RenderableComponent<GeometryVertex>>(
          mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_polygon_pipeline}},
          mge::RenderMode::WIREFRAME, std::move(polygon_vertex_array),
          [&bezier_entity](auto& render_pipeline) {
            render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
                "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
          })
      .disable();
  bezier_entity.patch<BezierCurveC2InterpComponent>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<BezierCurveC2InterpComponent, BezierCurveC2InterpComponent>(
        &BezierCurveC2InterpComponent::update_curve, &bezier_component);
  });
  bezier_entity.patch<BezierCurveC2InterpComponent>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<mge::TransformComponent, BezierCurveC2InterpComponent>(
        &BezierCurveC2InterpComponent::update_curve_by_self, &bezier_component);
  });
  return bezier_entity;
}

mge::Entity& CadLayer::create_bezier_surface_c0(const glm::vec3& pos, BezierSurfaceWrapping wrapping,
                                                unsigned int patches_u, unsigned int patches_v, float size_u,
                                                float size_v) {
  auto& bezier_entity = m_scene.create_entity();
  auto& grid_entity = m_scene.create_entity();
  bezier_entity.add_component<mge::TagComponent>(BezierSurfaceC0Component::get_new_name());
  bezier_entity.add_component<mge::TransformComponent>(pos);
  bezier_entity.add_component<BezierSurfaceC0Component>(patches_u, patches_v, size_u, size_v, wrapping, bezier_entity,
                                                        grid_entity);
  bezier_entity.add_component<SelectibleComponent>();
  bezier_entity.add_component<ColorComponent>();
  auto& bezier = bezier_entity.get_component<BezierSurfaceC0Component>();
  auto vertices = bezier.generate_geometry();
  auto surface_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  surface_vertex_buffer->bind();
  surface_vertex_buffer->copy(vertices);
  surface_vertex_buffer->unbind();
  auto surface_indices = bezier.generate_surface_topology();
  auto surface_element_buffer = std::make_unique<mge::ElementBuffer>(mge::ElementBuffer::Type::ELEMENT_ARRAY);
  surface_element_buffer->bind();
  surface_element_buffer->copy(surface_indices);
  surface_element_buffer->unbind();
  auto surface_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(surface_vertex_buffer), GeometryVertex::get_vertex_attributes(), std::move(surface_element_buffer));
  bezier_entity.add_component<mge::RenderableComponent<GeometryVertex>>(
      mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_surface_pipeline}},
      mge::RenderMode::WIREFRAME, std::move(surface_vertex_array), [&bezier_entity](auto& render_pipeline) {
        render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
            "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
        render_pipeline.template dynamic_uniform_update_and_commit<int>("line_count", [&bezier_entity]() {
          return bezier_entity.get_component<BezierSurfaceC0Component>().get_line_count();
        });
        auto parent = std::find_if(bezier_entity.get_parents().begin(), bezier_entity.get_parents().end(), [](const auto& parent) { return parent.get().template has_component<IntersectionComponent>(); });
        bool useTexture = parent != bezier_entity.get_parents().end();
        render_pipeline.template dynamic_uniform_update_and_commit<int>("useTexture", [&useTexture]() { return static_cast<int>(useTexture); });
        if (useTexture) {
          parent->get().template patch<IntersectionComponent>([&bezier_entity](auto& intersection_component){
            intersection_component.use_texture_for(bezier_entity, 0);
          });
        }
      });
  auto grid_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  grid_vertex_buffer->bind();
  grid_vertex_buffer->copy(vertices);
  grid_vertex_buffer->unbind();
  auto grid_indices = bezier.generate_grid_topology();
  auto grid_element_buffer = std::make_unique<mge::ElementBuffer>(mge::ElementBuffer::Type::ELEMENT_ARRAY);
  grid_element_buffer->bind();
  grid_element_buffer->copy(grid_indices);
  grid_element_buffer->unbind();
  auto grid_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(grid_vertex_buffer), GeometryVertex::get_vertex_attributes(), std::move(grid_element_buffer));
  grid_entity
      .add_component<mge::RenderableComponent<GeometryVertex>>(
          mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_grid_pipeline}},
          mge::RenderMode::WIREFRAME, std::move(grid_vertex_array),
          [&bezier_entity](auto& render_pipeline) {
            render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
                "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
          })
      .disable();
  bezier_entity.patch<BezierSurfaceC0Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<BezierSurfaceC0Component, BezierSurfaceC0Component>(
        &BezierSurfaceC0Component::update_surface, &bezier_component);
  });
  bezier_entity.patch<BezierSurfaceC0Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<mge::TransformComponent, BezierSurfaceC0Component>(
        &BezierSurfaceC0Component::update_surface_by_self, &bezier_component);
  });
  return bezier_entity;
}

mge::Entity& CadLayer::create_bezier_surface_c0(const std::vector<std::vector<mge::EntityId>>& points,
                                                BezierSurfaceWrapping wrapping, unsigned int patches_u,
                                                unsigned int patches_v, unsigned int line_count) {
  std::vector<mge::EntityVector> point_entities;
  point_entities.resize(points.size());
  for (int v = 0; v < points.size(); ++v) {
    point_entities[v].reserve(points[v].size());
    for (int u = 0; u < points[v].size(); ++u) {
      point_entities[v].push_back(m_scene.get_entity(points[v][u]));
    }
  }
  auto& bezier_entity = m_scene.create_entity();
  auto& grid_entity = m_scene.create_entity();
  bezier_entity.add_component<mge::TagComponent>(BezierSurfaceC0Component::get_new_name());
  bezier_entity.add_component<mge::TransformComponent>();
  bezier_entity.add_component<BezierSurfaceC0Component>(point_entities, patches_u, patches_v, wrapping, line_count,
                                                        bezier_entity, grid_entity);
  bezier_entity.add_component<SelectibleComponent>();
  bezier_entity.add_component<ColorComponent>();
  auto& bezier = bezier_entity.get_component<BezierSurfaceC0Component>();
  auto vertices = bezier.generate_geometry();
  auto surface_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  surface_vertex_buffer->bind();
  surface_vertex_buffer->copy(vertices);
  surface_vertex_buffer->unbind();
  auto surface_indices = bezier.generate_surface_topology();
  auto surface_element_buffer = std::make_unique<mge::ElementBuffer>(mge::ElementBuffer::Type::ELEMENT_ARRAY);
  surface_element_buffer->bind();
  surface_element_buffer->copy(surface_indices);
  surface_element_buffer->unbind();
  auto surface_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(surface_vertex_buffer), GeometryVertex::get_vertex_attributes(), std::move(surface_element_buffer));
  bezier_entity.add_component<mge::RenderableComponent<GeometryVertex>>(
      mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_surface_pipeline}},
      mge::RenderMode::WIREFRAME, std::move(surface_vertex_array), [&bezier_entity](auto& render_pipeline) {
        render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
            "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
        render_pipeline.template dynamic_uniform_update_and_commit<int>("line_count", [&bezier_entity]() {
          return bezier_entity.get_component<BezierSurfaceC0Component>().get_line_count();
        });
        render_pipeline.template dynamic_uniform_update_and_commit<int>("width", [&bezier_entity]() {
          return bezier_entity.get_component<BezierSurfaceC0Component>().get_patch_count_u();
        });
        render_pipeline.template dynamic_uniform_update_and_commit<int>("height", [&bezier_entity]() {
          return bezier_entity.get_component<BezierSurfaceC0Component>().get_patch_count_v();
        });
        auto parent = std::find_if(bezier_entity.get_parents().begin(), bezier_entity.get_parents().end(), [](const auto& parent) { return parent.get().template has_component<IntersectionComponent>(); });
        bool useTexture = parent != bezier_entity.get_parents().end();
        render_pipeline.template dynamic_uniform_update_and_commit<int>("useTexture", [&useTexture]() { return static_cast<int>(useTexture); });
        if (useTexture) {
          parent->get().template patch<IntersectionComponent>([&bezier_entity](auto& intersection_component){
            intersection_component.use_texture_for(bezier_entity, 0);
          });
        }
      });
  auto grid_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  grid_vertex_buffer->bind();
  grid_vertex_buffer->copy(vertices);
  grid_vertex_buffer->unbind();
  auto grid_indices = bezier.generate_grid_topology();
  auto grid_element_buffer = std::make_unique<mge::ElementBuffer>(mge::ElementBuffer::Type::ELEMENT_ARRAY);
  grid_element_buffer->bind();
  grid_element_buffer->copy(grid_indices);
  grid_element_buffer->unbind();
  auto grid_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(grid_vertex_buffer), GeometryVertex::get_vertex_attributes(), std::move(grid_element_buffer));
  grid_entity
      .add_component<mge::RenderableComponent<GeometryVertex>>(
          mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_grid_pipeline}},
          mge::RenderMode::WIREFRAME, std::move(grid_vertex_array),
          [&bezier_entity](auto& render_pipeline) {
            render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
                "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
          })
      .disable();
  bezier_entity.patch<BezierSurfaceC0Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<BezierSurfaceC0Component, BezierSurfaceC0Component>(
        &BezierSurfaceC0Component::update_surface, &bezier_component);
  });
  bezier_entity.patch<BezierSurfaceC0Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<mge::TransformComponent, BezierSurfaceC0Component>(
        &BezierSurfaceC0Component::update_surface_by_self, &bezier_component);
  });
  bezier_entity.patch<BezierSurfaceC0Component>([](auto&) {});
  return bezier_entity;
}

mge::Entity& CadLayer::create_bezier_surface_c2(const glm::vec3& pos, BezierSurfaceWrapping wrapping,
                                                unsigned int patches_u, unsigned int patches_v, float size_u,
                                                float size_v) {
  auto& bezier_entity = m_scene.create_entity();
  auto& grid_entity = m_scene.create_entity();
  bezier_entity.add_component<mge::TagComponent>(BezierSurfaceC2Component::get_new_name());
  bezier_entity.add_component<mge::TransformComponent>(pos);
  bezier_entity.add_component<BezierSurfaceC2Component>(patches_u, patches_v, size_u, size_v, wrapping, bezier_entity,
                                                        grid_entity);
  bezier_entity.add_component<SelectibleComponent>();
  bezier_entity.add_component<ColorComponent>();
  auto& bezier = bezier_entity.get_component<BezierSurfaceC2Component>();
  auto vertices = bezier.generate_geometry();
  auto surface_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  surface_vertex_buffer->bind();
  surface_vertex_buffer->copy(vertices);
  surface_vertex_buffer->unbind();
  auto surface_indices = bezier.generate_surface_topology();
  auto surface_element_buffer = std::make_unique<mge::ElementBuffer>(mge::ElementBuffer::Type::ELEMENT_ARRAY);
  surface_element_buffer->bind();
  surface_element_buffer->copy(surface_indices);
  surface_element_buffer->unbind();
  auto surface_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(surface_vertex_buffer), GeometryVertex::get_vertex_attributes(), std::move(surface_element_buffer));
  bezier_entity.add_component<mge::RenderableComponent<GeometryVertex>>(
      mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_surface_pipeline}},
      mge::RenderMode::WIREFRAME, std::move(surface_vertex_array), [&bezier_entity](auto& render_pipeline) {
        render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
            "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
        render_pipeline.template dynamic_uniform_update_and_commit<int>("line_count", [&bezier_entity]() {
          return bezier_entity.get_component<BezierSurfaceC2Component>().get_line_count();
        });
        render_pipeline.template dynamic_uniform_update_and_commit<int>("width", [&bezier_entity]() {
          return bezier_entity.get_component<BezierSurfaceC2Component>().get_patch_count_u();
        });
        render_pipeline.template dynamic_uniform_update_and_commit<int>("height", [&bezier_entity]() {
          return bezier_entity.get_component<BezierSurfaceC2Component>().get_patch_count_v();
        });
        auto parent = std::find_if(bezier_entity.get_parents().begin(), bezier_entity.get_parents().end(), [](const auto& parent) { return parent.get().template has_component<IntersectionComponent>(); });
        bool useTexture = parent != bezier_entity.get_parents().end();
        render_pipeline.template dynamic_uniform_update_and_commit<int>("useTexture", [&useTexture]() { return static_cast<int>(useTexture); });
        if (useTexture) {
          parent->get().template patch<IntersectionComponent>([&bezier_entity](auto& intersection_component){
            intersection_component.use_texture_for(bezier_entity, 0);
          });
        }
      });
  auto grid_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  grid_vertex_buffer->bind();
  grid_vertex_buffer->copy(vertices);
  grid_vertex_buffer->unbind();
  auto grid_indices = bezier.generate_grid_topology();
  auto grid_element_buffer = std::make_unique<mge::ElementBuffer>(mge::ElementBuffer::Type::ELEMENT_ARRAY);
  grid_element_buffer->bind();
  grid_element_buffer->copy(grid_indices);
  grid_element_buffer->unbind();
  auto grid_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(grid_vertex_buffer), GeometryVertex::get_vertex_attributes(), std::move(grid_element_buffer));
  grid_entity
      .add_component<mge::RenderableComponent<GeometryVertex>>(
          mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_grid_pipeline}},
          mge::RenderMode::WIREFRAME, std::move(grid_vertex_array),
          [&bezier_entity](auto& render_pipeline) {
            render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
                "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
          })
      .disable();
  bezier_entity.patch<BezierSurfaceC2Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<BezierSurfaceC2Component, BezierSurfaceC2Component>(
        &BezierSurfaceC2Component::update_surface, &bezier_component);
  });
  bezier_entity.patch<BezierSurfaceC2Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<mge::TransformComponent, BezierSurfaceC2Component>(
        &BezierSurfaceC2Component::update_surface_by_self, &bezier_component);
  });
  return bezier_entity;
}

mge::Entity& CadLayer::create_bezier_surface_c2(const std::vector<std::vector<mge::EntityId>>& points,
                                                BezierSurfaceWrapping wrapping, unsigned int patches_u,
                                                unsigned int patches_v, unsigned int line_count) {
  std::vector<mge::EntityVector> point_entities;
  point_entities.resize(points.size());
  for (int v = 0; v < points.size(); ++v) {
    point_entities[v].reserve(points[v].size());
    for (int u = 0; u < points[v].size(); ++u) {
      point_entities[v].push_back(m_scene.get_entity(points[v][u]));
    }
  }
  auto& bezier_entity = m_scene.create_entity();
  auto& grid_entity = m_scene.create_entity();
  bezier_entity.add_component<mge::TagComponent>(BezierSurfaceC2Component::get_new_name());
  bezier_entity.add_component<mge::TransformComponent>();
  bezier_entity.add_component<BezierSurfaceC2Component>(point_entities, patches_u, patches_v, wrapping, line_count,
                                                        bezier_entity, grid_entity);
  bezier_entity.add_component<SelectibleComponent>();
  bezier_entity.add_component<ColorComponent>();
  auto& bezier = bezier_entity.get_component<BezierSurfaceC2Component>();
  auto vertices = bezier.generate_geometry();
  auto surface_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  surface_vertex_buffer->bind();
  surface_vertex_buffer->copy(vertices);
  surface_vertex_buffer->unbind();
  auto surface_indices = bezier.generate_surface_topology();
  auto surface_element_buffer = std::make_unique<mge::ElementBuffer>(mge::ElementBuffer::Type::ELEMENT_ARRAY);
  surface_element_buffer->bind();
  surface_element_buffer->copy(surface_indices);
  surface_element_buffer->unbind();
  auto surface_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(surface_vertex_buffer), GeometryVertex::get_vertex_attributes(), std::move(surface_element_buffer));
  bezier_entity.add_component<mge::RenderableComponent<GeometryVertex>>(
      mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_surface_pipeline}},
      mge::RenderMode::WIREFRAME, std::move(surface_vertex_array), [&bezier_entity](auto& render_pipeline) {
        render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
            "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
        render_pipeline.template dynamic_uniform_update_and_commit<int>("line_count", [&bezier_entity]() {
          return bezier_entity.get_component<BezierSurfaceC2Component>().get_line_count();
        });
        render_pipeline.template dynamic_uniform_update_and_commit<int>("width", [&bezier_entity]() {
          return bezier_entity.get_component<BezierSurfaceC2Component>().get_patch_count_u();
        });
        render_pipeline.template dynamic_uniform_update_and_commit<int>("height", [&bezier_entity]() {
          return bezier_entity.get_component<BezierSurfaceC2Component>().get_patch_count_v();
        });
        auto parent = std::find_if(bezier_entity.get_parents().begin(), bezier_entity.get_parents().end(), [](const auto& parent) { return parent.get().template has_component<IntersectionComponent>(); });
        bool useTexture = parent != bezier_entity.get_parents().end();
        render_pipeline.template dynamic_uniform_update_and_commit<int>("useTexture", [&useTexture]() { return static_cast<int>(useTexture); });
        if (useTexture) {
          parent->get().template patch<IntersectionComponent>([&bezier_entity](auto& intersection_component){
            intersection_component.use_texture_for(bezier_entity, 0);
          });
        }
      });
  auto grid_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  grid_vertex_buffer->bind();
  grid_vertex_buffer->copy(vertices);
  grid_vertex_buffer->unbind();
  auto grid_indices = bezier.generate_grid_topology();
  auto grid_element_buffer = std::make_unique<mge::ElementBuffer>(mge::ElementBuffer::Type::ELEMENT_ARRAY);
  grid_element_buffer->bind();
  grid_element_buffer->copy(grid_indices);
  grid_element_buffer->unbind();
  auto grid_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(grid_vertex_buffer), GeometryVertex::get_vertex_attributes(), std::move(grid_element_buffer));
  grid_entity
      .add_component<mge::RenderableComponent<GeometryVertex>>(
          mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_grid_pipeline}},
          mge::RenderMode::WIREFRAME, std::move(grid_vertex_array),
          [&bezier_entity](auto& render_pipeline) {
            render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
                "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
          })
      .disable();
  bezier_entity.patch<BezierSurfaceC2Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<BezierSurfaceC2Component, BezierSurfaceC2Component>(
        &BezierSurfaceC2Component::update_surface, &bezier_component);
  });
  bezier_entity.patch<BezierSurfaceC2Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<mge::TransformComponent, BezierSurfaceC2Component>(
        &BezierSurfaceC2Component::update_surface_by_self, &bezier_component);
  });
  bezier_entity.patch<BezierSurfaceC2Component>([](auto&) {});
  return bezier_entity;
}

mge::Entity& CadLayer::create_gregory_patch(const GregoryPatchData& data) {
  auto& gregory_entity = m_scene.create_entity();
  auto& vectors_entity = m_scene.create_entity();
  gregory_entity.add_component<mge::TagComponent>(GregoryPatchComponent::get_new_name());
  gregory_entity.add_component<mge::TransformComponent>();
  gregory_entity.add_component<GregoryPatchComponent>(data, gregory_entity, vectors_entity);
  gregory_entity.add_component<SelectibleComponent>();
  gregory_entity.add_component<ColorComponent>();
  vectors_entity.add_component<ColorComponent>(glm::vec3{0.4f, 0.5f, 0.8f});
  auto& gregory = gregory_entity.get_component<GregoryPatchComponent>();
  auto patch_vertices = gregory.generate_patch_geometry();
  auto surface_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  surface_vertex_buffer->bind();
  surface_vertex_buffer->copy(patch_vertices);
  surface_vertex_buffer->unbind();
  auto surface_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(surface_vertex_buffer), GeometryVertex::get_vertex_attributes());
  gregory_entity.add_component<mge::RenderableComponent<GeometryVertex>>(
  mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_gregory_patch_pipeline}},
  mge::RenderMode::WIREFRAME, std::move(surface_vertex_array), [&gregory_entity](auto& render_pipeline) {
    render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
        "color", [&gregory_entity]() { return gregory_entity.get_component<ColorComponent>().get_color(); });
    render_pipeline.template dynamic_uniform_update_and_commit<int>("line_count", [&gregory_entity]() {
      return gregory_entity.get_component<GregoryPatchComponent>().get_line_count();
    });
  });
  auto vectors_vertices = gregory.generate_vectors_geometry();
  auto vectors_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  vectors_vertex_buffer->bind();
  vectors_vertex_buffer->copy(vectors_vertices);
  vectors_vertex_buffer->unbind();
  auto vectors_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(vectors_vertex_buffer), GeometryVertex::get_vertex_attributes());
  vectors_entity
      .add_component<mge::RenderableComponent<GeometryVertex>>(
          mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_gregory_vectors_pipeline}},
          mge::RenderMode::WIREFRAME, std::move(vectors_vertex_array),
          [&vectors_entity](auto& render_pipeline) {
            render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
                "color", [&vectors_entity]() { return vectors_entity.get_component<ColorComponent>().get_color(); });
          })
      .disable();
      gregory_entity.patch<GregoryPatchComponent>([&gregory_entity](auto& gregory_component) {
        gregory_entity.template register_on_update<GregoryPatchComponent, GregoryPatchComponent>(
        &GregoryPatchComponent::update_patch, &gregory_component);
  });
  return gregory_entity;
}

mge::Entity& CadLayer::create_intersection(mge::Entity& intersectable1, mge::Entity& intersectable2, const std::vector<glm::vec2>& points1, const std::vector<glm::vec2>& points2) {
  std::vector<GeometryVertex> curve_vertices;
  curve_vertices.reserve(points1.size()+1);
  for (auto& point : points1) {
    curve_vertices.push_back(m_intersection_builder.get_surface_position(intersectable1, point));
  }
  curve_vertices.push_back(curve_vertices.front());
  auto& intersection_entity = m_scene.create_entity();
  intersection_entity.add_component<mge::TagComponent>(IntersectionComponent::get_new_name());
  intersection_entity.add_component<SelectibleComponent>(glm::vec3{1.0f, 0.0f, 1.0f});
  intersection_entity.add_component<ColorComponent>();
  intersection_entity.add_component<IntersectionComponent>(intersectable1, intersectable2, points1, points2);
  intersection_entity.add_child(intersectable1);
  if (intersectable1 != intersectable2) {
    intersection_entity.add_child(intersectable2);
  }
  auto curve_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  curve_vertex_buffer->bind();
  curve_vertex_buffer->copy(curve_vertices);
  curve_vertex_buffer->unbind();
  auto curve_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(curve_vertex_buffer), GeometryVertex::get_vertex_attributes());
  intersection_entity.add_component<mge::RenderableComponent<GeometryVertex>>(
  mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_polygon_pipeline}},
  mge::RenderMode::WIREFRAME, std::move(curve_vertex_array), [&intersection_entity](auto& render_pipeline) {
    render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
        "color", [&intersection_entity]() { return intersection_entity.get_component<ColorComponent>().get_color(); });
  });
  return intersection_entity;
}

void CadLayer::create_cursor() {
  m_cursor = m_scene.create_entity();
  m_cursor.get().add_component<mge::TransformComponent>(glm::vec3{0.0f, 0.0f, 0.0f});
  auto cursor_vertex = std::vector<GeometryVertex>{{{0.0f, 0.0f, 0.0f}}};
  auto cursor_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  cursor_vertex_buffer->bind();
  cursor_vertex_buffer->copy(cursor_vertex);
  cursor_vertex_buffer->unbind();
  auto cursor_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(cursor_vertex_buffer), GeometryVertex::get_vertex_attributes());
  m_cursor.get().add_component<mge::RenderableComponent<GeometryVertex>>(
      mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_cursor_pipeline}},
      mge::RenderMode::WIREFRAME, std::move(cursor_vertex_array), [&cursor = m_cursor.get()](auto& render_pipeline) {
        render_pipeline.template dynamic_uniform_update_and_commit<glm::mat4>(
            "model", [&cursor]() { return cursor.get_component<mge::TransformComponent>().get_model_mat(); });
      });
}
void CadLayer::create_mass_center() {
  m_mass_center = m_scene.create_entity();
  m_mass_center.get()
      .add_component<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
          mge::InstancedRenderPipelineMap<GeometryVertex, PointInstancedVertex>{
              {mge::RenderMode::SOLID, *m_point_pipeline}},
          mge::RenderMode::SOLID, PointInstancedVertex{{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}})
      .disable();
  m_mass_center.get().add_component<ColorComponent>(glm::vec3{1.0f, 0.0f, 0.0f});
  m_mass_center.get().add_component<MassCenterComponent>();
}

void CadLayer::create_vertical_line() {
  m_vertical_line = m_scene.create_entity();
  auto vertical_line_vertex = std::vector<GeometryVertex>{{{0.0f, 0.0f, 0.0f}}};
  auto vertical_line_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  vertical_line_vertex_buffer->bind();
  vertical_line_vertex_buffer->copy(vertical_line_vertex);
  vertical_line_vertex_buffer->unbind();
  auto vertical_line_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(vertical_line_vertex_buffer), GeometryVertex::get_vertex_attributes());
  m_vertical_line.get().add_component<mge::RenderableComponent<GeometryVertex>>(
      mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_vertical_line_pipeline}},
      mge::RenderMode::WIREFRAME, std::move(vertical_line_vertex_array),
      [&cursor = m_cursor.get()](auto& render_pipeline) {
        render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
            "beg_pos", [&cursor]() { return cursor.get_component<mge::TransformComponent>().get_position(); });
      });
}