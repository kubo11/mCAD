#ifndef MCAD_GEOMETRY_MASS_CENTER_COMPONENT_HH
#define MCAD_GEOMETRY_MASS_CENTER_COMPONENT_HH

#include "mge.hh"

struct MassCenterComponent {
  MassCenterComponent() = default;
  MassCenterComponent(MassCenterComponent&&) = default;
  inline MassCenterComponent& operator=(MassCenterComponent&& other) {
    return *this;
  }

  void on_construct(mge::Entity& entity);
  void update_mass_center(const mge::EntityVector& selected_entities,
                          mge::Entity& entity);

 private:
  static glm::vec3 s_mass_center_color;

  int _ = 1;
};

#endif  // MCAD_GEOMETRY_MASS_CENTER_COMPONENT_HH