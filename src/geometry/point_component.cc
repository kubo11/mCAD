#include "point_component.hh"

unsigned int PointComponent::s_new_id = 1;
float PointComponent::s_scale = 0.05f;

std::unique_ptr<mge::VertexArray<GeometryVertex>>
PointComponent::get_vertex_array() {
  return std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::vector<GeometryVertex>{{{-1.0, -1.0, 1.0}},
                                  {{1.0, -1.0, 1.0}},
                                  {{1.0, 1.0, 1.0}},
                                  {{-1.0, 1.0, 1.0}},
                                  {{-1.0, -1.0, -1.0}},
                                  {{1.0, -1.0, -1.0}},
                                  {{1.0, 1.0, -1.0}},
                                  {{-1.0, 1.0, -1.0}}},
      GeometryVertex::get_vertex_attributes(),
      std::vector<unsigned int>{0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1,
                                7, 6, 5, 5, 4, 7, 4, 0, 3, 3, 7, 4,
                                4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3});
}

const std::shared_ptr<mge::Shader>& PointComponent::get_shader() {
  return mge::ShaderSystem::acquire(fs::current_path() / "src" / "shaders" /
                                    "solid" / "surface");
}

void PointComponent::on_construct(mge::Entity& entity) {
  entity.add_or_replace_component<mge::TransformComponent>();
  entity
      .add_or_replace_component<mge::RenderableComponent<GeometryVertex>>(
          PointComponent::get_shader(),
          std::move(PointComponent::get_vertex_array()))
      .set_render_mode(mge::RenderMode::SURFACE);
}