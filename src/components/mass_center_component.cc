#include "mass_center_component.hh"

MassCenterComponent::MassCenterComponent() : m_position{0.0f, 0.0f, 0.0f}, m_count{0u} {}

void MassCenterComponent::add_entity(mge::Entity& entity) {
  if (!entity.has_component<mge::TransformComponent>()) return;
  auto position = entity.get_component<mge::TransformComponent>().get_position();
  m_position = (m_position * static_cast<float>(m_count) + position) / static_cast<float>(m_count + 1);
  ++m_count;
  m_entities.push_back(entity);
}

void MassCenterComponent::remove_entity(mge::Entity& entity) {
  if (!entity.has_component<mge::TransformComponent>()) return;
  auto position = entity.get_component<mge::TransformComponent>().get_position();
  if (m_count == 1) {
    m_count = 0;
    m_position = {0.0f, 0.0f, 0.0f};
  } else {
    m_position = (m_position * static_cast<float>(m_count) - position) / static_cast<float>(m_count - 1);
    --m_count;
  }
  mge::vector_remove(m_entities, entity);
}

void MassCenterComponent::remove_all_entities() {
  m_position = {0.0f, 0.0f, 0.0f};
  m_count = 0;
  auto empty_vector = mge::EntityVector();
  m_entities.swap(empty_vector);
}

void MassCenterComponent::update_position() {
  m_position = {0.0f, 0.0f, 0.0f};
  m_count = 0;
  for (auto& entity : m_entities) {
    m_position += entity.get().get_component<mge::TransformComponent>().get_position();
    ++m_count;
  }
  m_position /= static_cast<float>(m_count);
}