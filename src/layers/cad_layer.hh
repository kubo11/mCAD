#ifndef MCAD_CAD_LAYER_HH
#define MCAD_CAD_LAYER_HH

#include "mge.hh"

#include "../geometry/geometry_vertex.hh"

class TorusModifiedEvent : public mge::Event {
 public:
  TorusModifiedEvent(float inner_radius, float outer_radius,
                     unsigned int horizontal_density,
                     unsigned int vertical_density, glm::vec3 color)
      : m_inner_radius(inner_radius),
        m_outer_radius(outer_radius),
        m_horizontal_density(horizontal_density),
        m_vertical_density(vertical_density),
        m_color(color) {}
  virtual inline const std::string name() const override {
    return "TorusModifiedEvent";
  }
  inline float get_inner_radius() const { return m_inner_radius; }
  inline float get_outer_radius() const { return m_outer_radius; }
  inline float get_horizontal_density() const { return m_horizontal_density; }
  inline float get_vertical_density() const { return m_vertical_density; }
  inline glm::vec3 get_color() const { return m_color; }

 private:
  float m_inner_radius;
  float m_outer_radius;
  unsigned int m_horizontal_density;
  unsigned int m_vertical_density;
  glm::vec3 m_color;
};

class CadLayer : public mge::Layer {
 public:
  CadLayer();
  ~CadLayer() {}

  virtual void configure() override;
  virtual void update() override;
  virtual void handle_event(mge::Event& event, float dt) override;

 private:
  glm::u8vec3 m_background_color = {100, 100, 100};
  std::vector<std::unique_ptr<mge::Drawable>> m_scene = {};
  mge::Camera m_camera;

  bool on_tours_modified(TorusModifiedEvent& event);
  bool on_camera_angle_modified(mge::CameraAngleEvent& event);
  bool on_camera_position_modified(mge::CameraPositionEvent& event);
  bool on_camera_zoom(mge::CameraZoomEvent& event);
};

#endif  // MCAD_CAD_LAYER_HH