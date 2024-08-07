#include "mass_center_component.hh"

MassCenterComponent::MassCenterComponent() : m_position{0.0f, 0.0f, 0.0f}, m_count{0u} {}

void MassCenterComponent::add_entity(mge::Entity& entity, bool is_parent) {
  glm::vec3 position_sum = {0.0f, 0.0f, 0.0f};
  unsigned int count = 0;
  entity.run_and_propagate([&position_sum, &count](auto& entity) {
    if (entity.template has_component<mge::TransformComponent>()) {
      position_sum += entity.template get_component<mge::TransformComponent>().get_position();
      count++;
    }
  });
  m_position = (m_position * static_cast<float>(m_count) + position_sum * static_cast<float>(count)) /
               static_cast<float>(m_count + count);
  m_count += count;
  if (is_parent) m_parent_count++;
  m_entities.push_back(entity);
}

void MassCenterComponent::remove_entity(mge::Entity& entity, bool is_parent) {
  glm::vec3 position_sum = {0.0f, 0.0f, 0.0f};
  unsigned int count = 0;
  entity.run_and_propagate([&position_sum, &count](auto& entity) {
    if (entity.template has_component<mge::TransformComponent>()) {
      position_sum += entity.template get_component<mge::TransformComponent>().get_position();
      count++;
    }
  });
  MGE_ASSERT(count <= m_count, "Attempted to remove too many entites from mass center");
  if (m_count == count) {
    m_count = 0;
    m_position = {0.0f, 0.0f, 0.0f};
  } else {
    m_position = (m_position * static_cast<float>(m_count) - position_sum * static_cast<float>(count)) /
                 static_cast<float>(m_count - count);
    m_count -= count;
  }
  mge::vector_remove(m_entities, entity);
  if (is_parent) m_parent_count--;
}

void MassCenterComponent::remove_all_entities() {
  m_position = {0.0f, 0.0f, 0.0f};
  m_count = 0;
  m_parent_count = 0;
  auto empty_vector = mge::EntityVector();
  m_entities.swap(empty_vector);
}

void MassCenterComponent::update_position() {
  m_position = {0.0f, 0.0f, 0.0f};
  m_count = 0;
  for (auto& entity : m_entities) {
    entity.get().run_and_propagate([&position_sum = m_position, &count = m_count](auto& entity) {
      if (entity.template has_component<mge::TransformComponent>()) {
        position_sum += entity.template get_component<mge::TransformComponent>().get_position();
        count++;
      }
    });
  }
  m_position /= static_cast<float>(m_count);
}