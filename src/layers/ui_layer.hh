#ifndef MCAD_UI_LAYER_HH
#define MCAD_UI_LAYER_HH

#include "mge.hh"

#include "../geometry/ellipsoid.hh"

struct LightData {};

class UILayer : public mge::Layer {
 public:
  UILayer();
  ~UILayer() {}

  virtual void configure() override;
  virtual void update() override;
  virtual void handle_event(mge::Event& event, float dt) override;

 private:
  float m_inner_radius;
  float m_outer_radius;
  int m_horizontal_density;
  int m_vertical_density;
  float m_color[3];
};

#endif  // MCAD_UI_LAYER_HH