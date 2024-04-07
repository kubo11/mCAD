#ifndef MCAD_GEOMETRY_MASS_CENTER_COMPONENT_HH
#define MCAD_GEOMETRY_MASS_CENTER_COMPONENT_HH

#include "mge.hh"

struct MassCenterComponent {
  MassCenterComponent() {}
  MassCenterComponent(MassCenterComponent&&) = default;
  inline MassCenterComponent& operator=(MassCenterComponent&& other) {
    return *this;
  }

  void on_construct(entt::registry& registry, entt::entity entity);
  void update_mass_center(mge::Scene& scene, mge::Entity& entity);

 private:
  static glm::vec3 s_mass_center_color;

  int _ = 1;
};

#endif  // MCAD_GEOMETRY_MASS_CENTER_COMPONENT_HH