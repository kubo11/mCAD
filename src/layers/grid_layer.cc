#include "grid_layer.hh"

GridLayer::GridLayer(mge::Scene& scene)
    : m_shader(mge::ShaderSystem::acquire(fs::current_path() / "src" /
                                          "shaders" / "grid" / "grid")),
      m_scene(scene) {}

void GridLayer::configure() {
  m_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::vector<GeometryVertex>{
          glm::vec3{1.0f, 1.0f, 0.0f}, glm::vec3{1.0f, -1.0f, 0.0f},
          glm::vec3{-1.0f, -1.0f, 0.0f}, glm::vec3{-1.0f, 1.0f, 0.0f}},
      GeometryVertex::get_vertex_attributes(),
      std::vector<unsigned int>{2, 0, 3, 2, 1, 0});
}

void GridLayer::update() {
  m_shader->use();
  m_shader->set_uniform(
      "projection_view",
      m_scene.get_current_camera().get_projection_view_matrix());
  m_vertex_array->bind();
  mge::Renderer<mge::RendererType::TRIANGLES>::draw_indexed(6);
  m_vertex_array->unbind();
  m_shader->unuse();
}

void GridLayer::handle_event(mge::Event& event, float dt) {}