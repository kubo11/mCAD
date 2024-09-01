#include "mcad_application.hh"
#include "layers/cad_layer.hh"
#include "layers/grid_layer.hh"
#include "layers/ui_layer.hh"

MCadApplication::MCadApplication() {
  m_event_manager = EventManager::create(*m_timer);
  m_scene->get_camera(0).set_velocity(10.0f);
  auto anaglyph_camera = std::make_unique<mge::AnaglyphCamera>(glm::vec3{2.0f, 2.0f, 2.0f}, -135, -35, 20,
                                                               m_scene->get_current_camera().get_aspect_ratio(), 0.1f,
                                                               100.0f, 0.06f, 1.0f, 1.0f);
  m_scene->add_camera(std::move(anaglyph_camera));
  m_scene->get_camera(1).set_velocity(10.0f);

  // Anaglyph events
  AddEventListener(AnaglyphEvents::UpdateState, MCadApplication::on_anaglyph_update_state, this);
  AddEventListener(AnaglyphEvents::UpdateEyeDistance, MCadApplication::on_anaglyph_update_eye_distance, this);
  AddEventListener(AnaglyphEvents::UpdateProjectivePlaneDistance,
                   MCadApplication::on_anaglyph_update_projective_plane_distance, this);
  AddEventListener(AnaglyphEvents::UpdateScreenDistance, MCadApplication::on_anaglyph_update_screen_distance, this);

  // Layers
  auto window_dims = glm::ivec2{m_main_window->get_width(), m_main_window->get_height()};
  auto cad_layer = std::make_unique<CadLayer>(*m_scene, window_dims);
  push_layer(std::move(cad_layer));
  push_layer(std::move(std::make_unique<GridLayer>(*m_scene)));
  push_layer(std::move(std::make_unique<UILayer>()));
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