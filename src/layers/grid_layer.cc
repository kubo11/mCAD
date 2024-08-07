#include "grid_layer.hh"

GridLayer::GridLayer(mge::Scene& scene) : m_scene(scene) {}

void GridLayer::configure() {
  auto vertices = std::vector<GridVertex>{{{1.0f, 1.0f}}, {{1.0f, -1.0f}}, {{-1.0f, -1.0f}}, {{-1.0f, 1.0f}}};
  auto vertex_buffer = std::make_unique<mge::Buffer<GridVertex>>();
  vertex_buffer->bind();
  vertex_buffer->copy(vertices);
  vertex_buffer->unbind();
  auto indices = std::vector<unsigned int>{2, 0, 3, 2, 1, 0};
  auto element_buffet = std::make_unique<mge::ElementBuffer>(mge::ElementBuffer::Type::ELEMENT_ARRAY);
  element_buffet->bind();
  element_buffet->copy(indices);
  element_buffet->unbind();
  auto vertex_array = std::make_unique<mge::VertexArray<GridVertex>>(
      std::move(vertex_buffer), GridVertex::get_vertex_attributes(), std::move(element_buffet));
  mge::RenderPipelineBuilder pipeline_builder;
  auto shader_program = mge::ShaderSystem::acquire(fs::current_path() / "src" / "shaders" / "grid" / "grid");
  shader_program->set_uniform_value("near_plane", m_scene.get_current_camera().get_near_plane());
  shader_program->set_uniform_value("far_plane", m_scene.get_current_camera().get_far_plane());
  m_render_pipeline =
      std::move(pipeline_builder.add_shader_program(shader_program)
                    .add_uniform_update<glm::mat4>("projection_view",
                                                   [&camera = m_scene.get_current_camera()]() {
                                                     return camera.get_projection_matrix() * camera.get_view_matrix();
                                                   })
                    .build<GridVertex>(mge::DrawPrimitiveType::TRIANGLE));
  m_grid = std::move(std::make_unique<mge::RenderableComponent<GridVertex>>(
      std::move(mge::RenderPipelineMap<GridVertex>{{mge::RenderMode::SOLID, *m_render_pipeline}}),
      mge::RenderMode::SOLID, std::move(vertex_array)));
}

void GridLayer::update() { m_render_pipeline->run(); }