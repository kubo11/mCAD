#include "cursor_component.hh"

#include "cursor_vertex.hh"

void CursorComponent::on_construct(mge::Entity& entity) {
  entity.add_or_replace_component<mge::TransformComponent>();
  entity.add_or_replace_component<mge::RenderableComponent<CursorVertex>>(
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