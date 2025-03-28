#include "mcad_application.hh"
#include "layers/cad_layer.hh"
#include "layers/grid_layer.hh"
#include "layers/ui_layer.hh"

#include <nfd.hpp>

MCadApplication::MCadApplication() {
  m_event_manager = EventManager::create(*m_timer);
  auto anaglyph_camera = std::make_unique<mge::AnaglyphCamera>(glm::vec3{3.0f, 3.0f, 0.0f}, 180, -45, 45,
                                                               m_scene->get_current_camera().get_aspect_ratio(), 0.01f,
                                                               100.0f, 0.06f, 1.0f, 1.0f);
  m_scene->add_camera(std::move(anaglyph_camera));

  // Anaglyph events
  AddEventListener(AnaglyphEvents::UpdateState, MCadApplication::on_anaglyph_update_state, this);
  AddEventListener(AnaglyphEvents::UpdateEyeDistance, MCadApplication::on_anaglyph_update_eye_distance, this);
  AddEventListener(AnaglyphEvents::UpdateProjectivePlaneDistance,
                   MCadApplication::on_anaglyph_update_projective_plane_distance, this);
  AddEventListener(AnaglyphEvents::UpdateScreenDistance, MCadApplication::on_anaglyph_update_screen_distance, this);
  // Deserialization events
  AddEventListener(SerializationEvents::DeserializePoints, MCadApplication::on_deserialize_points, this);
  AddEventListener(SerializationEvents::DeserializeTorus, MCadApplication::on_deserialize_torus, this);
  AddEventListener(SerializationEvents::DeserializeBezierCurveC0, MCadApplication::on_deserialize_bezier_curve_c0,
                   this);
  AddEventListener(SerializationEvents::DeserializeBezierCurveC2, MCadApplication::on_deserialize_bezier_curve_c2,
                   this);
  AddEventListener(SerializationEvents::DeserializeBezierCurveC2Interp,
                   MCadApplication::on_deserialize_bezier_curve_c2_interp, this);
  AddEventListener(SerializationEvents::DeserializeBezierSurfaceC0, MCadApplication::on_deserialize_bezier_surface_c0,
                   this);
  AddEventListener(SerializationEvents::DeserializeBezierSurfaceC2, MCadApplication::on_deserialize_bezier_surface_c2,
                   this);
  AddEventListener(SerializationEvents::SerializeScene, MCadApplication::on_serialize_scene, this);
  AddEventListener(SerializationEvents::DeserializeScene, MCadApplication::on_deserialize_scene, this);

  // Init NFD
  MGE_ASSERT(NFD::Init() == NFD_OKAY, "{}", NFD::GetError());

  // Layers
  auto window_dims = glm::ivec2{m_main_window->get_width(), m_main_window->get_height()};
  auto cad_layer = std::make_unique<CadLayer>(*m_scene, window_dims);
  push_layer(std::move(cad_layer));
  push_layer(std::move(std::make_unique<GridLayer>(*m_scene)));
  push_layer(std::move(std::make_unique<UILayer>()));
}

MCadApplication::~MCadApplication() {
  NFD::Quit();
}

bool MCadApplication::on_anaglyph_update_state(AnaglyphUpdateStateEvent& event) {
  if (event.state) {
    m_scene->get_camera(1).copy_camera_data(m_scene->get_camera(0));
    m_scene->set_current_camera(1);
  } else {
    m_scene->get_camera(0).copy_camera_data(m_scene->get_camera(1));
    m_scene->set_current_camera(0);
  }

  return false;
}

bool MCadApplication::on_anaglyph_update_eye_distance(AnaglyphUpdateEyeDistanceEvent& event) {
  auto& anaglyph_camera = dynamic_cast<mge::AnaglyphCamera&>(m_scene->get_camera(1));
  anaglyph_camera.set_eye_distance(event.distance);
  return true;
}

bool MCadApplication::on_anaglyph_update_projective_plane_distance(AnaglyphUpdateProjectivePlaneDistanceEvent& event) {
  auto& anaglyph_camera = dynamic_cast<mge::AnaglyphCamera&>(m_scene->get_camera(1));
  anaglyph_camera.set_projective_plane_distance(event.distance);
  return true;
}

bool MCadApplication::on_anaglyph_update_screen_distance(AnaglyphUpdateScreenDistanceEvent& event) {
  auto& anaglyph_camera = dynamic_cast<mge::AnaglyphCamera&>(m_scene->get_camera(1));
  anaglyph_camera.set_screen_distance(event.distance);
  return true;
}

bool MCadApplication::on_deserialize_points(DeserializePointsEvent& event) {
  CadLayer& layer = dynamic_cast<CadLayer&>(*m_layer_stack->begin()->get());
  std::vector<std::pair<mge::EntityId, std::string>> announce_data;
  for (auto& point_data : event.data) {
    auto& point = layer.create_point(point_data.second);
    point.patch<mge::TagComponent>([&tag = point_data.first](auto& tag_component) { tag_component.set_tag(tag); });
    event.ids.push_back(point.get_id());
    announce_data.push_back({point.get_id(), point.get_component<mge::TagComponent>().get_tag()});
  }
  AnnounceDeserializedPointsEvent announce_event(announce_data);
  SendEvent(announce_event);
  return true;
}

bool MCadApplication::on_deserialize_torus(DeserializeTorusEvent& event) {
  CadLayer& layer = dynamic_cast<CadLayer&>(*m_layer_stack->begin()->get());
  auto& torus = layer.create_torus(event.position, event.inner_radius, event.outer_radius, event.inner_density,
                                   event.outer_density);
  torus.patch<mge::TransformComponent>([&event](auto& transform) {
    transform.set_rotation(event.rotation);
    transform.set_scale(event.scale);
  });
  torus.patch<mge::TagComponent>([&tag = event.name](auto& tag_component) { tag_component.set_tag(tag); });
  mge::AddedEntityEvent add_event(torus.get_id());
  SendEngineEvent(add_event);
  return true;
}

bool MCadApplication::on_deserialize_bezier_curve_c0(DeserializeBezierCurveC0Event& event) {
  CadLayer& layer = dynamic_cast<CadLayer&>(*m_layer_stack->begin()->get());
  auto& bezier = layer.create_bezier_curve_c0(event.points);
  bezier.patch<mge::TagComponent>([&tag = event.name](auto& tag_component) { tag_component.set_tag(tag); });
  mge::AddedEntityEvent add_event(bezier.get_id());
  SendEngineEvent(add_event);
  return true;
}

bool MCadApplication::on_deserialize_bezier_curve_c2(DeserializeBezierCurveC2Event& event) {
  CadLayer& layer = dynamic_cast<CadLayer&>(*m_layer_stack->begin()->get());
  auto& bezier = layer.create_bezier_curve_c2(event.points);
  bezier.patch<mge::TagComponent>([&tag = event.name](auto& tag_component) { tag_component.set_tag(tag); });
  mge::AddedEntityEvent add_event(bezier.get_id());
  SendEngineEvent(add_event);
  return true;
}

bool MCadApplication::on_deserialize_bezier_curve_c2_interp(DeserializeBezierCurveC2InterpEvent& event) {
  CadLayer& layer = dynamic_cast<CadLayer&>(*m_layer_stack->begin()->get());
  auto& bezier = layer.create_bezier_curve_c2_interp(event.points);
  bezier.patch<mge::TagComponent>([&tag = event.name](auto& tag_component) { tag_component.set_tag(tag); });
  mge::AddedEntityEvent add_event(bezier.get_id());
  SendEngineEvent(add_event);
  return true;
}

bool MCadApplication::on_deserialize_bezier_surface_c0(DeserializeBezierSurfaceC0Event& event) {
  CadLayer& layer = dynamic_cast<CadLayer&>(*m_layer_stack->begin()->get());
  auto& bezier = layer.create_bezier_surface_c0(event.points, event.wrapping, event.patch_count_u, event.patch_count_v,
                                                event.line_count);
  bezier.patch<mge::TagComponent>([&tag = event.name](auto& tag_component) { tag_component.set_tag(tag); });
  mge::AddedEntityEvent add_event(bezier.get_id());
  SendEngineEvent(add_event);
  return true;
}

bool MCadApplication::on_deserialize_bezier_surface_c2(DeserializeBezierSurfaceC2Event& event) {
  CadLayer& layer = dynamic_cast<CadLayer&>(*m_layer_stack->begin()->get());
  auto& bezier = layer.create_bezier_surface_c2(event.points, event.wrapping, event.patch_count_u, event.patch_count_v,
                                                event.line_count);
  bezier.patch<mge::TagComponent>([&tag = event.name](auto& tag_component) { tag_component.set_tag(tag); });
  mge::AddedEntityEvent add_event(bezier.get_id());
  SendEngineEvent(add_event);
  return true;
}

bool MCadApplication::on_serialize_scene(SerializeSceneEvent& event) {
  m_serializer.serialize(*m_scene, event.path);
  return true;
}

bool MCadApplication::on_deserialize_scene(DeserializeSceneEvent& event) {
  m_scene->clear();
  CadLayer& layer = dynamic_cast<CadLayer&>(*m_layer_stack->begin()->get());
  layer.create_cursor();
  layer.create_mass_center();
  m_serializer.deserialize(*m_scene, event.path);
  return true;
}