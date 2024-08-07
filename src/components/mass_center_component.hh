#ifndef MCAD_COMPONENTS_MASS_CENTER_COMPONENT
#define MCAD_COMPONENTS_MASS_CENTER_COMPONENT

#include "mge.hh"

struct MassCenterComponent {
  MassCenterComponent();
  MassCenterComponent(MassCenterComponent&&) = default;
  MassCenterComponent& operator=(MassCenterComponent&& other) { return *this; }

  void add_entity(mge::Entity& entity, bool is_parent);
  void remove_entity(mge::Entity& entity, bool is_parent);
  void remove_all_entities();
  void update_position();

  unsigned int get_count() const { return m_count; }
  unsigned int get_parent_count() const { return m_parent_count; }
  glm::vec3 get_position() const { return m_position; }

 private:
  mge::EntityVector m_entities;
  unsigned int m_count;
  unsigned int m_parent_count;
  glm::vec3 m_position;
};

#endif  // MCAD_COMPONENTS_MASS_CENTER_COMPONENT