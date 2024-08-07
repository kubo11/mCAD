#ifndef MGE_COMPONENTS_COLOR_COMPONENT
#define MGE_COMPONENTS_COLOR_COMPONENT

#include "mge.hh"

class ColorComponent {
 public:
  ColorComponent(glm::vec3 color = {0.0f, 0.0f, 0.0f}) : m_color(color) {}

  const glm::vec3 get_color() const { return m_color; }
  void set_color(glm::vec3 color) { m_color = color; }

 private:
  glm::vec3 m_color;
};

#endif  // MGE_COMPONENTS_COLOR_COMPONENT