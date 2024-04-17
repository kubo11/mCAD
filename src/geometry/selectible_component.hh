#ifndef MCAD_GEOMETRY_SELECTIBLE_COMPONENT_HH
#define MCAD_GEOMETRY_SELECTIBLE_COMPONENT_HH

#include "mge.hh"

struct SelectibleComponent {
  SelectibleComponent() : m_selected(false) {}
  SelectibleComponent(SelectibleComponent&& other)
      : m_selected(std::move(other.m_selected)) {}
  inline SelectibleComponent& operator=(SelectibleComponent&& other) {
    m_selected = std::move(other.m_selected);
    return *this;
  }

  bool is_selected() const { return m_selected; }
  void set_selection(bool selected) { m_selected = selected; }

  template <class T>
  void on_update(mge::Entity& entity) {
    entity.patch<mge::RenderableComponent<T>>([this](auto& renderable) {
      if (m_selected) {
        renderable.set_color(s_selected_color);
      } else {
        renderable.set_color(s_regular_color);
      }
    });

    // TODO: announce selection
  }

 private:
  static const glm::vec3 s_selected_color;
  static const glm::vec3 s_regular_color;

  bool m_selected;
};

#endif  // MCAD_GEOMETRY_SELECTIBLE_COMPONENT_HH