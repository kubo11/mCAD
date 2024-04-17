#ifndef MCAD_EVENTS_TORUS_EVENT_HH
#define MCAD_EVENTS_TORUS_EVENT_HH

#include "mge.hh"

class TorusUpdatedEvent : public mge::Event {
 public:
  TorusUpdatedEvent(const std::string& tag, float inner_radius,
                    float outer_radius, unsigned int horizontal_density,
                    unsigned int vertival_density)
      : m_tag(tag),
        m_inner_radius(inner_radius),
        m_outer_radius(outer_radius),
        m_horizontal_density(horizontal_density),
        m_vertical_density(vertival_density) {}
  virtual inline const std::string name() const override {
    return "TorusUpdateEvent";
  }
  inline const std::string& get_tag() const { return m_tag; }
  inline float get_inner_radius() const { return m_inner_radius; }
  inline float get_outer_radius() const { return m_outer_radius; }
  inline unsigned int get_horizontal_density() const {
    return m_horizontal_density;
  }
  inline unsigned int get_vertical_density() const {
    return m_vertical_density;
  }

 protected:
  std::string m_tag;
  float m_inner_radius;
  float m_outer_radius;
  unsigned int m_horizontal_density;
  unsigned int m_vertical_density;
};

#endif  // MCAD_EVENTS_TORUS_EVENT_HH