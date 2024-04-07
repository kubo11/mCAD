#include "mass_center_component.hh"

#include "geometry_vertex.hh"
#include "point_component.hh"
#include "selectible_component.hh"

glm::vec3 MassCenterComponent::s_mass_center_color = {1.0f, 0.0f, 0.0f};

void MassCenterComponent::on_construct(entt::registry& registry,
                                       entt::entity entity) {
  registry.emplace_or_replace<PointComponent>(entity);
  auto& renderable =
      registry.get<mge::RenderableComponent<GeometryVertex>>(entity);
  renderable.set_color(s_mass_center_color);
  renderable.disable();
}

void MassCenterComponent::update_mass_center(mge::Scene& scene,
                                             mge::Entity& entity) {
  auto size =
      scene.size<>(entt::get_t<SelectedComponent, mge::TransformComponent>(),
                   entt::exclude_t<>()) +
      scene.size<>(
          entt::get_t<SelectedChildComponent, mge::TransformComponent>(),
          entt::exclude_t<>());
  if (size < 2) {
    entity.patch<mge::RenderableComponent<GeometryVertex>>(
        [](auto& renderable) { renderable.disable(); });
    if (size == 0) {
      return;
    }
  } else {
    entity.patch<mge::RenderableComponent<GeometryVertex>>(
        [](auto& renderable) { renderable.enable(); });
  }

  glm::vec3 center = {0.0f, 0.0f, 0.0f};
  float count = 0.0f;
  scene.foreach (entt::get_t<SelectedComponent, mge::TransformComponent>(),
                 entt::exclude_t<>(), [&center, &count](auto& entity) {
                   auto& transform =
                       entity.template get_component<mge::TransformComponent>();
                   center += transform.get_position();
                   count++;
                 });
  scene.foreach (entt::get_t<SelectedChildComponent, mge::TransformComponent>(),
                 entt::exclude_t<>(), [&center, &count](auto& entity) {
                   auto& transform =
                       entity.template get_component<mge::TransformComponent>();
                   center += transform.get_position();
                   count++;
                 });
  center /= count;

  entity.patch<mge::TransformComponent>(
      [&center](auto& transform) { transform.set_position(center); });
}