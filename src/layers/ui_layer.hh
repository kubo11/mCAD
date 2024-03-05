#ifndef MCAD_UI_LAYER_HH
#define MCAD_UI_LAYER_HH

#include "mge.hh"

#include "../ellipsoid.hh"

struct LightData {};

class UILayer : public mge::Layer {
 public:
  UILayer();
  ~UILayer() {}

  virtual void configure() override;
  virtual void update() override;
  virtual void handle_event(mge::Event& event, float dt) override;

 private:
  float m_a;
  float m_b;
  float m_c;
  float m_color[3];
  float m_ambient;
  float m_diffuse;
  float m_specular;
  float m_shininess;
  int m_accuracy;

  inline Ellipsoid generate_ellipsoid() const {
    return {m_a,
            m_b,
            m_c,
            {{m_color[0] * 255, m_color[1] * 255, m_color[2] * 255},
             m_ambient,
             m_diffuse,
             m_specular,
             m_shininess}};
  }
};

#endif  // MCAD_UI_LAYER_HH