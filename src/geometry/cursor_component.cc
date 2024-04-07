#include "cursor_component.hh"

#include "cursor_vertex.hh"

void CursorComponent::on_construct(entt::registry& registry,
                                   entt::entity entity) {
  registry.emplace_or_replace<mge::TransformComponent>(entity);
  registry.emplace_or_replace<mge::RenderableComponent<CursorVertex>>(
      entity,
      mge::ShaderSystem::acquire(fs::current_path() / "src" / "shaders" /
                                 "cursor"),
      std::move(std::make_unique<mge::VertexArray<CursorVertex>>(
          std::vector<CursorVertex>{{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
                                    {{0.3f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                    {{0.0f, 0.3f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                    {{0.0f, 0.0f, 0.3f}, {0.0f, 0.0f, 1.0f}}},
          CursorVertex::get_vertex_attributes(),
          std::vector<unsigned int>{0, 1, 0, 2, 0, 3})));
}