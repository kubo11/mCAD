#ifndef MCAD_GEOMETRY_SELECTIBLE_COMPONENT_HH
#define MCAD_GEOMETRY_SELECTIBLE_COMPONENT_HH

#include "mge.hh"

struct SelectibleComponent {
  SelectibleComponent() : m_selected(false), m_enabled(true) {}
  SelectibleComponent(SelectibleComponent&& other) : m_selected(std::move(other.m_selected)) {}
  inline SelectibleComponent& operator=(SelectibleComponent&& other) {
    m_selected = std::move(other.m_selected);
    return *this;
  }

  bool is_selected() const { return m_selected; }
  void set_selection(bool selected) { m_selected = selected; }

  bool is_enabled() const { return m_enabled; }
  void set_status(bool status) { m_enabled = status; }

 private:
  static const glm::vec3 s_selected_color;
  static const glm::vec3 s_regular_color;

  bool m_selected;
  bool m_enabled;
};

#endif  // MCAD_GEOMETRY_SELECTIBLE_COMPONENT_HH