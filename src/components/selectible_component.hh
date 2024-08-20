#ifndef MCAD_GEOMETRY_SELECTIBLE_COMPONENT_HH
#define MCAD_GEOMETRY_SELECTIBLE_COMPONENT_HH

#include "mge.hh"

struct SelectibleComponent {
  SelectibleComponent(glm::vec3 selected_color = {0.8f, 0.3f, 0.0f}, glm::vec3 regular_color = {0.0f, 0.0f, 0.0f})
      : m_selected(false), m_enabled(true), m_selected_color(selected_color), m_regular_color(regular_color) {}
  SelectibleComponent(SelectibleComponent&& other)
      : m_selected(std::move(other.m_selected)),
        m_enabled(std::move(other.m_enabled)),
        m_selected_color(std::move(other.m_selected_color)),
        m_regular_color(std::move(other.m_regular_color)) {}
  inline SelectibleComponent& operator=(SelectibleComponent&& other) {
    m_selected = std::move(other.m_selected);
    m_enabled = std::move(other.m_enabled);
    m_selected_color = std::move(other.m_selected_color);
    m_regular_color = std::move(other.m_regular_color);
    return *this;
  }

  bool is_selected() const { return m_selected; }
  void set_selection(bool selected) { m_selected = selected; }

  bool is_enabled() const { return m_enabled; }
  void set_status(bool status) { m_enabled = status; }

  glm::vec3 get_selected_color() const { return m_selected_color; }
  glm::vec3 get_regular_color() const { return m_regular_color; }

 private:
  glm::vec3 m_selected_color;
  glm::vec3 m_regular_color;

  bool m_selected;
  bool m_enabled;
};

#endif  // MCAD_GEOMETRY_SELECTIBLE_COMPONENT_HH