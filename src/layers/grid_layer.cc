#include "grid_layer.hh"

GridLayer::GridLayer(mge::Camera& camera)
    : m_shader(mge::ShaderSystem::acquire(fs::current_path() / "src" /
                                          "shaders" / "grid" / "grid")),
      m_camera(camera) {}

void GridLayer::configure() {
  m_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::vector<GeometryVertex>{
          glm::vec3{0.6f, 1.0f, 0.0f}, glm::vec3{0.6f, -1.0f, 0.0f},
          glm::vec3{-1.0f, -1.0f, 0.0f}, glm::vec3{-1.0f, 1.0f, 0.0f}},
      GeometryVertex::get_vertex_attributes(),
      std::vector<unsigned int>{0, 1, 3, 1, 2, 3});
}

void GridLayer::update() {
  m_shader.use();
  m_shader.set_uniform("projection_view",
                       m_camera.get_projection_view_matrix());
  m_vertex_array->bind();
  mge::Renderer<mge::RendererType::TRIANGLES>::draw_indexed(6);
  //   glClear(GL_DEPTH_BUFFER_BIT);
}

void GridLayer::handle_event(mge::Event& event, float dt) {}