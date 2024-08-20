#include "cad_layer.hh"

#include "../components/bezier_curve_c0_component.hh"
#include "../components/bezier_curve_c2_component.hh"
#include "../components/color_component.hh"
#include "../components/mass_center_component.hh"
#include "../components/point_component.hh"
#include "../components/selectible_component.hh"
#include "../components/torus_component.hh"
#include "../vertices/cursor_vertex.hh"

CadLayer::CadLayer(mge::Scene& scene, const glm::ivec2& window_size)
    : m_scene(scene),
      m_cursor(m_scene.create_entity()),
      m_mass_center(m_scene.create_entity()),
      m_window_size(window_size) {}

CadLayer::~CadLayer() { m_scene.clear(); }

void CadLayer::configure() {
  // Enable listeners
  m_scene.enable_on_update_listeners<mge::TransformComponent>();
  m_scene.enable_on_update_listeners<ColorComponent>();
  m_scene.enable_on_update_listeners<BezierCurveC0Component>();
  m_scene.enable_on_update_listeners<BezierCurveC2Component>();
  // Pipelines
  auto base_shader_path = fs::current_path() / "src" / "shaders";
  mge::RenderPipelineBuilder pipeline_builder;
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "solid" / "surface"))
      .add_uniform_update<glm::mat4>("projection_view", [&camera = m_scene.get_current_camera()]() {
        return camera.get_projection_matrix() * camera.get_view_matrix();
      });
  m_geometry_solid_pipeline = std::move(pipeline_builder.build<GeometryVertex>(mge::DrawPrimitiveType::TRIANGLE));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "solid" / "wireframe"))
      .add_uniform_update<glm::mat4>("projection_view", [&camera = m_scene.get_current_camera()]() {
        return camera.get_projection_matrix() * camera.get_view_matrix();
      });
  m_geometry_wireframe_pipeline = std::move(pipeline_builder.build<GeometryVertex>(mge::DrawPrimitiveType::LINE));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "cursor"))
      .add_uniform_update<glm::mat4>("projection_view",
                                     [&camera = m_scene.get_current_camera()]() {
                                       return camera.get_projection_matrix() * camera.get_view_matrix();
                                     })
      .add_uniform_update<glm::vec2>("window_size", [&window_size = m_window_size]() { return window_size; });
  m_cursor_pipeline = std::move(pipeline_builder.build<GeometryVertex>(mge::DrawPrimitiveType::POINT));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "bezier"))
      .add_uniform_update<glm::mat4>("projection_view",
                                     [&camera = m_scene.get_current_camera()]() {
                                       return camera.get_projection_matrix() * camera.get_view_matrix();
                                     })
      .add_uniform_update<glm::vec2>("window_size", [&window_size = m_window_size]() { return window_size; })
      .set_patch_count(4);
  m_bezier_pipeline = std::move(pipeline_builder.build<GeometryVertex>(mge::DrawPrimitiveType::PATCH));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "bezier_poly"))
      .add_uniform_update<glm::mat4>("projection_view", [&camera = m_scene.get_current_camera()]() {
        return camera.get_projection_matrix() * camera.get_view_matrix();
      });
  m_bezier_polygon_pipeline = std::move(pipeline_builder.build<GeometryVertex>(mge::DrawPrimitiveType::LINE_STRIP));
  pipeline_builder.add_shader_program(mge::ShaderSystem::acquire(base_shader_path / "point"))
      .add_uniform_update<glm::mat4>("projection_view",
                                     [&camera = m_scene.get_current_camera()]() {
                                       return camera.get_projection_matrix() * camera.get_view_matrix();
                                     })
      .add_uniform_update<glm::vec2>("window_size", [&window_size = m_window_size]() { return window_size; });
  // auto point_vertices =
  //     std::vector<GeometryVertex>{{{-1.0f, 1.0f, 0.0f}}, {{-1.0f, -1.0f, 0.0f}}, {{1.0f, -1.0f, 0.0f}},
  //                                 {{-1.0f, 1.0f, 0.0f}}, {{1.0f, -1.0f, 0.0f}},  {{1.0f, 1.0f, 0.0f}}};
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
  auto point_instance_buffer = std::make_unique<mge::Buffer<PointInstancedVertex>>();
  auto point_vertex_array = std::make_unique<mge::InstancedVertexArray<GeometryVertex, PointInstancedVertex>>(
      std::move(point_vertex_buffer), GeometryVertex::get_vertex_attributes(), std::move(point_instance_buffer),
      PointInstancedVertex::get_vertex_attributes());
  m_point_pipeline = std::move(pipeline_builder.build_instanced<GeometryVertex, PointInstancedVertex>(
      mge::DrawPrimitiveType::TRIANGLE, std::move(point_vertex_array)));

  // Cursor
  m_cursor.add_component<mge::TransformComponent>(glm::vec3{0.0f, 0.0f, 0.0f});
  auto cursor_vertex = std::vector<GeometryVertex>{{{0.0f, 0.0f, 0.0f}}};
  auto cursor_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  cursor_vertex_buffer->bind();
  cursor_vertex_buffer->copy(cursor_vertex);
  cursor_vertex_buffer->unbind();
  auto cursor_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(cursor_vertex_buffer), GeometryVertex::get_vertex_attributes());
  m_cursor.add_component<mge::RenderableComponent<GeometryVertex>>(
      mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_cursor_pipeline}},
      mge::RenderMode::WIREFRAME, std::move(cursor_vertex_array), [&cursor = m_cursor](auto& render_pipeline) {
        render_pipeline.template dynamic_uniform_update_and_commit<glm::mat4>(
            "model", [&cursor]() { return cursor.get_component<mge::TransformComponent>().get_model_mat(); });
      });

  // Mass center
  m_mass_center
      .add_component<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
          mge::InstancedRenderPipelineMap<GeometryVertex, PointInstancedVertex>{
              {mge::RenderMode::SOLID, *m_point_pipeline}},
          mge::RenderMode::SOLID, PointInstancedVertex{{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}})
      .disable();
  m_mass_center.add_component<ColorComponent>(glm::vec3{1.0f, 0.0f, 0.0f});
  m_mass_center.add_component<MassCenterComponent>();

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
  // Cursor events
  AddEventListener(CursorEvents::Move, CadLayer::on_cursor_move, this);
  // Transform events
  AddEventListener(TransformEvents::TranslateToCursor, CadLayer::on_translate_to_cursor, this);
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
}

void CadLayer::update() {
  for (auto id : m_to_be_destroyed) {
    m_scene.destroy_entity(id);
  }
  m_to_be_destroyed.clear();

  m_geometry_wireframe_pipeline->run();
  m_geometry_solid_pipeline->run();
  m_bezier_pipeline->run();
  m_bezier_polygon_pipeline->run();
  m_point_pipeline->run();
  m_cursor_pipeline->run();
}

glm::vec3 CadLayer::unproject_point(glm::vec2 pos) const {
  auto& camera = m_scene.get_current_camera();
  auto proj_view = camera.get_projection_matrix() * camera.get_view_matrix();
  glm::vec4 unprojected_point = glm::inverse(proj_view) * glm::vec4(pos, 0.0f, 1.0f);
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
  m_mass_center.patch<MassCenterComponent>([](auto& center) { center.update_position(); });
  auto mass_center_position = m_mass_center.template get_component<MassCenterComponent>().get_position();
  auto mass_center_color = m_mass_center.template get_component<ColorComponent>().get_color();
  m_mass_center.template patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
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
  if (!entity.get_parents().empty() && entity.get_parents().begin()->get() != m_mass_center) {
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
  auto& entity = m_scene.create_entity();
  entity.template add_component<PointComponent>();
  entity.template add_component<mge::TagComponent>(PointComponent::get_new_name());
  entity.template add_component<mge::TransformComponent>(
      m_cursor.get_component<mge::TransformComponent>().get_position());
  auto point_position = entity.get_component<mge::TransformComponent>().get_position();
  entity.template add_component<SelectibleComponent>();
  entity.template add_component<ColorComponent>();
  auto position = entity.get_component<mge::TransformComponent>().get_position();
  auto color = entity.get_component<ColorComponent>().get_color();
  entity.template add_component<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
      mge::InstancedRenderPipelineMap<GeometryVertex, PointInstancedVertex>{
          {mge::RenderMode::SOLID, *m_point_pipeline}},
      mge::RenderMode::SOLID, PointInstancedVertex(position, color));
  m_point_pipeline->update_instance_buffer();
  entity.register_on_update<mge::TransformComponent>(&CadLayer::update_point_instance_data, this);
  entity.register_on_update<ColorComponent>(&CadLayer::update_point_instance_data, this);
  mge::AddedEntityEvent add_event(entity.get_id());
  SendEngineEvent(add_event);
  event.point = entity;
  return true;
}

bool CadLayer::on_add_torus(AddTorusEvent& event) {
  auto& entity = m_scene.create_entity();
  entity.template add_component<TorusComponent>(event.inner_radius, event.outer_radius, event.inner_density,
                                                event.outer_density);
  entity.template add_component<mge::TagComponent>(TorusComponent::get_new_name());
  entity.template add_component<mge::TransformComponent>(
      m_cursor.get_component<mge::TransformComponent>().get_position());
  entity.patch<mge::TransformComponent>([](auto& transform) { transform.set_scale({0.1f, 0.1f, 0.1f}); });
  entity.template add_component<SelectibleComponent>();
  entity.template add_component<ColorComponent>();
  auto vertices = entity.get_component<TorusComponent>().generate_geometry();
  auto vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  vertex_buffer->bind();
  vertex_buffer->copy(vertices);
  vertex_buffer->unbind();
  auto indices = entity.get_component<TorusComponent>().generate_topology<mge::RenderMode::WIREFRAME>();
  auto element_buffer = std::make_unique<mge::ElementBuffer>(mge::ElementBuffer::Type::ELEMENT_ARRAY);
  element_buffer->bind();
  element_buffer->copy(indices);
  element_buffer->unbind();
  auto vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(vertex_buffer), GeometryVertex::get_vertex_attributes(), std::move(element_buffer));
  auto dupa = entity.get_component<mge::TransformComponent>().get_model_mat();
  entity.template add_component<mge::RenderableComponent<GeometryVertex>>(
      mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_geometry_wireframe_pipeline},
                                             {mge::RenderMode::SOLID, *m_geometry_solid_pipeline}},
      mge::RenderMode::WIREFRAME, std::move(vertex_array), [&entity](auto& render_pipeline) {
        render_pipeline.template dynamic_uniform_update_and_commit<glm::mat4>(
            "model", [&entity]() { return entity.get_component<mge::TransformComponent>().get_model_mat(); });
        render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
            "color", [&entity]() { return entity.get_component<ColorComponent>().get_color(); });
      });
  mge::AddedEntityEvent add_event(entity.get_id());
  SendEngineEvent(add_event);
  return true;
}

bool CadLayer::on_torus_radius_updated(TorusRadiusUpdatedEvent& event) {
  auto& entity = m_scene.get_entity(event.id);
  entity.patch<TorusComponent>([&event](auto& torus) {
    torus.set_inner_radius(event.inner_radius);
    torus.set_outer_radius(event.outer_radius);
  });
  auto& torus = entity.get_component<TorusComponent>();
  entity.patch<mge::RenderableComponent<GeometryVertex>>([&torus](auto& renderable) {
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
  entity.patch<mge::RenderableComponent<GeometryVertex>>([&torus](auto& renderable) {
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
  auto& bezier_entity = m_scene.create_entity();
  auto& polygon_entity = m_scene.create_entity();
  for (auto id : event.control_points) {
    auto& entity = m_scene.get_entity(id);
    if (!entity.has_component<PointComponent>()) {
      m_scene.destroy_entity(bezier_entity);
      m_scene.destroy_entity(polygon_entity);
      return false;
    }
    bezier_entity.add_child(entity);
  }
  bezier_entity.template add_component<mge::TagComponent>(BezierCurveC0Component::get_new_name());
  bezier_entity.template add_component<BezierCurveC0Component>(bezier_entity.get_children(), bezier_entity,
                                                               polygon_entity);
  bezier_entity.template add_component<SelectibleComponent>();
  bezier_entity.template add_component<ColorComponent>();
  auto& bezier = bezier_entity.get_component<BezierCurveC0Component>();
  auto curve_vertices = bezier.generate_geometry();
  auto curve_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  curve_vertex_buffer->bind();
  curve_vertex_buffer->copy(curve_vertices);
  curve_vertex_buffer->unbind();
  auto curve_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(std::move(curve_vertex_buffer),
                                                                               GeometryVertex::get_vertex_attributes());
  bezier_entity.template add_component<mge::RenderableComponent<GeometryVertex>>(
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
      .template add_component<mge::RenderableComponent<GeometryVertex>>(
          mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_polygon_pipeline}},
          mge::RenderMode::WIREFRAME, std::move(polygon_vertex_array),
          [&bezier_entity](auto& render_pipeline) {
            render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
                "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
          })
      .disable();
  bezier_entity.patch<BezierCurveC0Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<BezierCurveC0Component, BezierCurveC0Component>(
        &BezierCurveC0Component::update_renderable, &bezier_component);
  });
  mge::AddedEntityEvent add_event(bezier_entity.get_id());
  SendEngineEvent(add_event);
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
  auto& bezier_entity = m_scene.create_entity();
  auto& polygon_entity = m_scene.create_entity();
  for (auto id : event.control_points) {
    auto& entity = m_scene.get_entity(id);
    if (!entity.has_component<PointComponent>()) {
      m_scene.destroy_entity(bezier_entity);
      m_scene.destroy_entity(polygon_entity);
      return false;
    }
    bezier_entity.add_child(entity);
  }
  bezier_entity.template add_component<mge::TagComponent>(BezierCurveC2Component::get_new_name());
  bezier_entity.template add_component<BezierCurveC2Component>(bezier_entity.get_children(), bezier_entity,
                                                               polygon_entity);
  bezier_entity.template add_component<SelectibleComponent>();
  bezier_entity.template add_component<ColorComponent>();
  auto& bezier = bezier_entity.get_component<BezierCurveC2Component>();
  auto curve_vertices = bezier.generate_geometry();
  auto curve_vertex_buffer = std::make_unique<mge::Buffer<GeometryVertex>>();
  curve_vertex_buffer->bind();
  curve_vertex_buffer->copy(curve_vertices);
  curve_vertex_buffer->unbind();
  auto curve_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(std::move(curve_vertex_buffer),
                                                                               GeometryVertex::get_vertex_attributes());
  bezier_entity.template add_component<mge::RenderableComponent<GeometryVertex>>(
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
      .template add_component<mge::RenderableComponent<GeometryVertex>>(
          mge::RenderPipelineMap<GeometryVertex>{{mge::RenderMode::WIREFRAME, *m_bezier_polygon_pipeline}},
          mge::RenderMode::WIREFRAME, std::move(polygon_vertex_array),
          [&bezier_entity](auto& render_pipeline) {
            render_pipeline.template dynamic_uniform_update_and_commit<glm::vec3>(
                "color", [&bezier_entity]() { return bezier_entity.get_component<ColorComponent>().get_color(); });
          })
      .disable();
  bezier_entity.patch<BezierCurveC2Component>([&bezier_entity](auto& bezier_component) {
    bezier_entity.template register_on_update<BezierCurveC2Component, BezierCurveC2Component>(
        &BezierCurveC2Component::update_renderable, &bezier_component);
  });
  mge::AddedEntityEvent add_event(bezier_entity.get_id());
  SendEngineEvent(add_event);
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
  bezier.patch<BezierCurveC0Component>([&point](auto& component) { component.remove_point(point); });
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
  SendEngineEvent(add_event);
  return true;
}

bool CadLayer::on_cursor_move(CursorMoveEvent& event) {
  m_cursor.patch<mge::TransformComponent>(
      [this, &event](auto& transform) { transform.set_position(unproject_point(event.screen_space_position)); });
  return true;
}

bool CadLayer::on_translate_to_cursor(TranslateToCursorEvent& event) {
  glm::vec3 center = m_mass_center.get_component<MassCenterComponent>().get_position();
  glm::vec3 offset = m_cursor.get_component<mge::TransformComponent>().get_position() - center;

  for (auto id : event.ids) {
    auto& entity = m_scene.get_entity(id);
    if (!entity.has_component<mge::TransformComponent>()) continue;
    entity.patch<mge::TransformComponent>([&offset](auto& transform) { transform.translate(offset); });
  }

  update_mass_center();

  return true;
}

bool CadLayer::on_relative_scale(RelativeScaleEvent& event) {
  glm::vec3 center = m_mass_center.get_component<MassCenterComponent>().get_position();
  float scale_factor = glm::distance(center, unproject_point(event.scaling_end)) /
                       glm::distance(center, unproject_point(event.scaling_begin));

  for (auto id : event.ids) {
    auto& entity = m_scene.get_entity(id);
    if (!entity.has_component<mge::TransformComponent>()) continue;
    entity.patch<mge::TransformComponent>([&scale_factor](auto& transform) { transform.scale(scale_factor); });
  }

  m_mass_center.patch<MassCenterComponent>([](auto& center) { center.update_position(); });
  update_mass_center();

  return true;
}

bool CadLayer::on_relative_rotate(RelativeRotateEvent& event) {
  glm::vec3 center = m_mass_center.get_component<MassCenterComponent>().get_position();
  auto camera = m_scene.get_current_camera();
  auto proj_view = camera.get_projection_matrix() * camera.get_view_matrix();
  glm::vec2 center_screen_space = proj_view * glm::vec4(center, 1.0f);
  float angle = glm::asin(glm::cross(glm::vec3(glm::normalize(event.rotation_begin - center_screen_space), 0.0f),
                                     glm::vec3(glm::normalize(event.rotation_end - center_screen_space), 0.0f)))
                    .z;

  for (auto id : event.ids) {
    auto& entity = m_scene.get_entity(id);
    if (!entity.has_component<mge::TransformComponent>()) continue;
    entity.patch<mge::TransformComponent>([&angle, &center, &event](auto& transform) {
      transform.rotate(angle, event.axis);
      transform.set_position(center + glm::angleAxis(angle, event.axis) * (transform.get_position() - center));
    });
  }

  update_mass_center();

  return true;
}

bool CadLayer::on_translate(TranslateEvent& event) {
  auto& entity = m_scene.get_entity(event.id);
  entity.run_and_propagate([this, &event](auto& entity) {
    if (!entity.template has_component<mge::TransformComponent>()) return;
    entity.template patch<mge::TransformComponent>([&event](auto& transform) { transform.set_position(event.offset); });
  });
  update_mass_center();

  return true;
}

bool CadLayer::on_scale(ScaleEvent& event) {
  auto& entity = m_scene.get_entity(event.id);
  entity.run_and_propagate([this, &event](auto& entity) {
    if (!entity.template has_component<mge::TransformComponent>()) return;
    entity.template patch<mge::TransformComponent>([&event](auto& transform) { transform.set_scale(event.scale); });
  });
  update_mass_center();
  return true;
}

bool CadLayer::on_rotate(RotateEvent& event) {
  auto& entity = m_scene.get_entity(event.id);
  entity.run_and_propagate([this, &event](auto& entity) {
    if (!entity.template has_component<mge::TransformComponent>()) return;
    entity.template patch<mge::TransformComponent>(
        [&event](auto& transform) { transform.set_rotation(event.rotation); });
  });
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
  entity.patch<mge::RenderableComponent<GeometryVertex>>(
      [&event](auto& renderable) { renderable.set_render_mode(event.render_mode); });
  if (entity.has_component<TorusComponent>()) {
    auto& torus = entity.get_component<TorusComponent>();
    entity.patch<mge::RenderableComponent<GeometryVertex>>([&torus](auto& renderable) {
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

  if (entity.has_component<mge::TransformComponent>()) {
    if (event.selection) {
      m_mass_center.add_child(entity);
      m_mass_center.patch<MassCenterComponent>(
          [&entity, &event](auto& mass_center) { mass_center.add_entity(entity, event.is_parent); });
    } else {
      m_mass_center.remove_child(entity);
      m_mass_center.patch<MassCenterComponent>(
          [&entity, &event](auto& mass_center) { mass_center.remove_entity(entity, event.is_parent); });
    }
    update_mass_center();
    if (m_mass_center.get_component<MassCenterComponent>().get_parent_count() > 1) {
      m_mass_center.patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
          [](auto& renderable) { renderable.enable(); });
    } else {
      m_mass_center.patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
          [](auto& renderable) { renderable.disable(); });
    }
  }
  return true;
}

bool CadLayer::on_unselect_all_entities(UnselectAllEntitiesEvent& event) {
  m_mass_center.patch<MassCenterComponent>([](auto& mass_center) { mass_center.remove_all_entities(); });
  m_mass_center.remove_all_children();
  m_scene.foreach<>(entt::get<SelectibleComponent>, entt::exclude<>, [&](mge::Entity& entity) {
    if (!entity.get_component<SelectibleComponent>().is_selected()) return;
    entity.patch<SelectibleComponent>([](auto& selectible) { selectible.set_selection(false); });
  });
  m_scene.foreach<>(entt::get<SelectibleComponent, ColorComponent>, entt::exclude<>, [&](mge::Entity& entity) {
    auto color = entity.get_component<SelectibleComponent>().get_regular_color();
    entity.patch<ColorComponent>([&color](auto& color_component) { color_component.set_color(color); });
  });
  m_mass_center.patch<mge::InstancedRenderableComponent<GeometryVertex, PointInstancedVertex>>(
      [](auto& renderable) { renderable.disable(); });
  return true;
}