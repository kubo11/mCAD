#include "bezier_component.hh"

unsigned int BezierComponent::s_new_id = 1;
fs::path BezierComponent::s_draw_shader_path =
    fs::current_path() / "src" / "shaders" / "bezier";

void BezierComponent::on_construct(mge::Entity& entity) {
  auto vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(this->generate_geometry()),
      GeometryVertex::get_vertex_attributes());
  entity.add_or_replace_component<mge::RenderableComponent<GeometryVertex>>(
      mge::ShaderSystem::acquire(s_draw_shader_path), std::move(vertex_array));
}

void BezierComponent::on_update(mge::Entity& entity) {
  entity.patch<mge::RenderableComponent<GeometryVertex>>(
      [this](auto& renderbale) {
        renderbale.get_vertex_array().update_vertices(generate_geometry());
      });
}
std::vector<GeometryVertex> BezierComponent::generate_geometry() {
  return std::vector<GeometryVertex>();
}
