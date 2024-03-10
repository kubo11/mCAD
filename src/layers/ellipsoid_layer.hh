#ifndef MCAD_ELLIPSOID_LAYER_HH
#define MCAD_ELLIPSOID_LAYER_HH

#include "mge.hh"

#include "../geometry/ellipsoid.hh"

class EllipsoidModifiedEvent : public mge::Event {
 public:
  EllipsoidModifiedEvent(const Ellipsoid& new_elipsoid)
      : m_ellipsoid(new_elipsoid) {}
  virtual inline const std::string name() const override {
    return "EllipsoidModifiedEvent";
  }
  inline const Ellipsoid& get_ellipsoid() const { return m_ellipsoid; }

 private:
  Ellipsoid m_ellipsoid;
};

class EllipsoidLayer : public mge::Layer {
 public:
  EllipsoidLayer();
  ~EllipsoidLayer() {}

  virtual void configure() override;
  virtual void update() override;
  virtual void handle_event(mge::Event& event, float dt) override;

 private:
  glm::u8vec3 m_background_color = {100, 100, 100};
  Ellipsoid m_ellipsoid;
  mge::AdaptiveRaycaster m_raycaster;
  mge::Camera m_camera;

  bool on_ellipsoid_modified(EllipsoidModifiedEvent& event);
  bool on_raycaster_modified(mge::RaycasterAccuracyModifiedEvent& event);
  bool on_camera_angle_modified(mge::CameraAngleEvent& event);
  bool on_camera_position_modified(mge::CameraPositionEvent& event);
  bool on_camera_zoom(mge::CameraZoomEvent& event);
};

#endif  // MCAD_ELLIPSOID_LAYER_HH