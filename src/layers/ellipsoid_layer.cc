#include "ellipsoid_layer.hh"

EllipsoidLayer::EllipsoidLayer()
    : m_ellipsoid(0.5f, 1.0f, 1.5f,
                  mge::Material({255, 255, 0}, 0.1f, 0.5f, 0.5f, 0.5f)),
      m_raycaster(5, 1280, 720, m_background_color),
      m_camera(20.0f, 1280.0f / 720.0f, 0.0f, 1000.0f) {}

void EllipsoidLayer::configure() {}

void EllipsoidLayer::update() { m_raycaster.draw(m_ellipsoid, m_camera); }

void EllipsoidLayer::handle_event(mge::Event& event, float dt) {
  mge::Event::try_handler<EllipsoidModifiedEvent>(
      event, BIND_EVENT_HANDLER(EllipsoidLayer::on_ellipsoid_modified));

  mge::Event::try_handler<mge::RaycasterAccuracyModifiedEvent>(
      event, BIND_EVENT_HANDLER(EllipsoidLayer::on_raycaster_modified));

  mge::Event::try_handler<mge::CameraAngleEvent>(
      event, BIND_EVENT_HANDLER(EllipsoidLayer::on_camera_angle_modified));

  mge::Event::try_handler<mge::CameraPositionEvent>(
      event, BIND_EVENT_HANDLER(EllipsoidLayer::on_camera_position_modified));

  mge::Event::try_handler<mge::CameraZoomEvent>(
      event, BIND_EVENT_HANDLER(EllipsoidLayer::on_camera_zoom));
}

bool EllipsoidLayer::on_ellipsoid_modified(EllipsoidModifiedEvent& event) {
  m_ellipsoid = event.get_ellipsoid();
  m_raycaster.reset();
  return true;
}

bool EllipsoidLayer::on_raycaster_modified(
    mge::RaycasterAccuracyModifiedEvent& event) {
  m_raycaster.set_max_pixel_size_exponent(event.get_max_pixel_size_exponent());
  m_raycaster.reset();
  return true;
}

bool EllipsoidLayer::on_camera_angle_modified(mge::CameraAngleEvent& event) {
  m_camera.add_azimuth(event.get_azimuth());
  m_camera.add_elevation(event.get_elevation());
  m_raycaster.reset();
  return true;
}

bool EllipsoidLayer::on_camera_position_modified(
    mge::CameraPositionEvent& event) {
  m_camera.move(event.get_pos());
  m_raycaster.reset();
  return true;
}

bool EllipsoidLayer::on_camera_zoom(mge::CameraZoomEvent& event) {
  m_camera.zoom(event.get_zoom());
  m_raycaster.reset();
  return true;
}