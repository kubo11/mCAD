#ifndef MCAD_EVENTS_DRAW_MASS_CENTER_EVENT_HH
#define MCAD_EVENTS_DRAW_MASS_CENTER_EVENT_HH

#include "mge.hh"

class DrawMassCenterEvent : public mge::Event {
 public:
  DrawMassCenterEvent() : m_mass_center(std::nullopt) {}
  virtual inline const std::string name() const override {
    return "DrawMassCenterEvent";
  }
  inline std::optional<glm::vec3> get_mass_center() const {
    return m_mass_center;
  }
  inline void set_mass_center(std::optional<glm::vec3> mass_center) {
    m_mass_center = mass_center;
  }

 protected:
  std::optional<glm::vec3> m_mass_center;
};

#endif  // MCAD_EVENTS_DRAW_MASS_CENTER_EVENT_HH