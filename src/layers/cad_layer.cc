#include "cad_layer.hh"

#include "../geometry/torus.hh"

CadLayer::CadLayer() : m_camera(20.0f, 1280.0f / 720.0f, 0.0f, 1000.0f) {}

void CadLayer::configure() {
  m_scene.push_back(
      std::make_unique<Torus>(4.0f, 6.0f, glm::vec3(0.0f, 0.0f, 0.0f)));

  for (const auto& object : m_scene) {
    object->init_draw_data();
  }
}

void CadLayer::update() {
  mge::Shader& shader = mge::ShaderSystem::acquire(
      fs::current_path() / "src" / "shaders" / "solid" / "wireframe");
  shader.use();
  shader.set_uniform("projection_view", m_camera.get_projection_view_matrix());

  for (const auto& object : m_scene) {
    object->draw();
  }
}

void CadLayer::handle_event(mge::Event& event, float dt) {
  mge::Event::try_handler<TorusModifiedEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_tours_modified));

  mge::Event::try_handler<mge::CameraAngleEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_camera_angle_modified));

  mge::Event::try_handler<mge::CameraPositionEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_camera_position_modified));

  mge::Event::try_handler<mge::CameraZoomEvent>(
      event, BIND_EVENT_HANDLER(CadLayer::on_camera_zoom));
}

bool CadLayer::on_tours_modified(TorusModifiedEvent& event) {
  Torus& torus = dynamic_cast<Torus&>(*m_scene.front());
  if (torus.get_inner_radius() != event.get_inner_radius()) {
    torus.set_inner_radius(event.get_inner_radius());
  }
  if (torus.get_outer_radius() != event.get_outer_radius()) {
    torus.set_outer_radius(event.get_outer_radius());
  }
  if (torus.get_horizontal_density() != event.get_horizontal_density()) {
    torus.set_horizontal_density(event.get_horizontal_density());
  }
  if (torus.get_vertical_density() != event.get_vertical_density()) {
    torus.set_vertical_density(event.get_vertical_density());
  }
  if (torus.get_color() != event.get_color()) {
    torus.set_color(event.get_color());
  }
  return true;
}

bool CadLayer::on_camera_angle_modified(mge::CameraAngleEvent& event) {
  m_camera.add_azimuth(event.get_azimuth());
  m_camera.add_elevation(event.get_elevation());
  return true;
}

bool CadLayer::on_camera_position_modified(mge::CameraPositionEvent& event) {
  m_camera.move(event.get_pos());
  return true;
}

bool CadLayer::on_camera_zoom(mge::CameraZoomEvent& event) {
  m_camera.zoom(event.get_zoom());
  return true;
}