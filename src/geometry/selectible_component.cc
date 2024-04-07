#include "selectible_component.hh"

const glm::vec3 SelectibleComponent::s_selected_color = {1.0f, 0.5f, 0.0f};
const glm::vec3 SelectibleComponent::s_regular_color = {0.0f, 0.0f, 0.0f};

void SelectedComponent::on_construct(entt::registry& registry,
                                     entt::entity entity) {
  registry.patch<SelectibleComponent>(
      entity, [](auto& selectible) { selectible.set_selection(true); });
}
void SelectedComponent::on_destroy(entt::registry& registry,
                                   entt::entity entity) {
  registry.patch<SelectibleComponent>(
      entity, [](auto& selectible) { selectible.set_selection(false); });
}

void SelectedChildComponent::on_construct(entt::registry& registry,
                                          entt::entity entity) {
  registry.patch<SelectibleComponent>(
      entity, [](auto& selectible) { selectible.set_selection(true); });
}
void SelectedChildComponent::on_destroy(entt::registry& registry,
                                        entt::entity entity) {
  registry.patch<SelectibleComponent>(
      entity, [](auto& selectible) { selectible.set_selection(false); });
}