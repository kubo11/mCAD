#include "mass_center_component.hh"

#include "geometry_vertex.hh"
#include "point_component.hh"
#include "selectible_component.hh"

glm::vec3 MassCenterComponent::s_mass_center_color = {1.0f, 0.0f, 0.0f};

void MassCenterComponent::on_construct(mge::Entity& entity) {
  entity.add_or_replace_component<PointComponent>();
  entity.patch<mge::RenderableComponent<GeometryVertex>>(
      [this](auto& renderable) {
        renderable.set_color(s_mass_center_color);
        renderable.disable();
      });
}

void MassCenterComponent::update_mass_center(
    const mge::EntityVector& selected_entities, mge::Entity& entity) {
  if (selected_entities.size() < 2) {
    entity.patch<mge::RenderableComponent<GeometryVertex>>(
        [](auto& renderable) { renderable.disable(); });
    if (selected_entities.empty()) {
      return;
    }
  } else {
    entity.patch<mge::RenderableComponent<GeometryVertex>>(
        [](auto& renderable) { renderable.enable(); });
  }

  glm::vec3 center = {0.0f, 0.0f, 0.0f};
  float count = 0.0f;
  for (auto& selected : selected_entities) {
    selected.get().run_and_propagate([&center, &count](auto& entity) {
      if (entity.template has_component<mge::TransformComponent>()) {
        center += entity.template get_component<mge::TransformComponent>()
                      .get_position();
        count++;
      }
    });
  }
  center /= count;

  entity.patch<mge::TransformComponent>(
      [&center](auto& transform) { transform.set_position(center); });
}