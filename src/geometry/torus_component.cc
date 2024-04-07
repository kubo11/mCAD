#include "torus_component.hh"

unsigned int TorusComponent::s_new_id = 1;
const fs::path TorusComponent::s_default_shader_path =
    fs::current_path() / "src" / "shaders" / "solid" / "wireframe";

std::vector<GeometryVertex> TorusComponent::generate_geometry() {
  std::vector<GeometryVertex> vertices(m_vertical_density *
                                       m_horizontal_density);

  float ds = 2 * glm::pi<float>() / m_vertical_density;
  float dt = 2 * glm::pi<float>() / m_horizontal_density;
  for (int j = 0; j < m_vertical_density; ++j) {
    for (int i = 0; i < m_horizontal_density; ++i) {
      float s = j * ds;
      float t = i * dt;
      float c = m_inner_radius * std::cos(t) + m_outer_radius;

      vertices[j * m_horizontal_density + i] = GeometryVertex(glm::vec3{
          std::sin(s) * c, m_inner_radius * std::sin(t), std::cos(s) * c});
    }
  }

  return vertices;
}

template <>
std::vector<unsigned int>
TorusComponent::generate_topology<mge::RenderMode::WIREFRAME>() {
  std::vector<unsigned int> indices(m_vertical_density * m_horizontal_density *
                                    4);

  for (int j = 0; j < m_vertical_density; ++j) {
    for (int i = 0; i < m_horizontal_density; ++i) {
      auto a = m_horizontal_density * j + i;
      auto b = m_horizontal_density * j + (i + 1) % m_horizontal_density;
      auto d = m_horizontal_density * ((j + 1) % m_vertical_density) + i;

      // edges
      auto face_idx = j * m_horizontal_density + i;
      indices[4 * face_idx + 0] = a;
      indices[4 * face_idx + 1] = b;
      indices[4 * face_idx + 2] = a;
      indices[4 * face_idx + 3] = d;
    }
  }

  return indices;
}

template <>
std::vector<unsigned int>
TorusComponent::generate_topology<mge::RenderMode::SURFACE>() {
  std::vector<unsigned int> indices(m_vertical_density * m_horizontal_density *
                                    6);

  for (int j = 0; j < m_vertical_density; ++j) {
    for (int i = 0; i < m_horizontal_density; ++i) {
      auto a = m_horizontal_density * j + i;
      auto b = m_horizontal_density * j + (i + 1) % m_horizontal_density;
      auto c = m_horizontal_density * ((j + 1) % m_vertical_density) +
               (i + 1) % m_horizontal_density;
      auto d = m_horizontal_density * ((j + 1) % m_vertical_density) + i;

      // faces
      auto face_idx = j * m_horizontal_density + i;
      indices[6 * face_idx + 0] = a;
      indices[6 * face_idx + 1] = b;
      indices[6 * face_idx + 2] = d;
      indices[6 * face_idx + 3] = b;
      indices[6 * face_idx + 4] = c;
      indices[6 * face_idx + 5] = d;
    }
  }

  return indices;
}

void TorusComponent::on_construct(entt::registry& registry,
                                  entt::entity entity) {
  registry.emplace_or_replace<mge::TransformComponent>(entity);
  auto vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(this->generate_geometry()),
      GeometryVertex::get_vertex_attributes(),
      std::move(this->generate_topology<mge::RenderMode::WIREFRAME>()));
  registry.emplace_or_replace<mge::RenderableComponent<GeometryVertex>>(
      entity, mge::ShaderSystem::acquire(s_default_shader_path),
      std::move(vertex_array));
}

void TorusComponent::on_update(entt::registry& registry, entt::entity entity) {
  auto& renderable =
      registry.get<mge::RenderableComponent<GeometryVertex>>(entity);
  renderable.get_vertex_array().update_vertices(
      std::move(this->generate_geometry()));
  if (renderable.get_render_mode() == mge::RenderMode::WIREFRAME) {
    renderable.get_vertex_array().update_indices(
        std::move(this->generate_topology<mge::RenderMode::WIREFRAME>()));
  } else if (renderable.get_render_mode() == mge::RenderMode::SURFACE) {
    renderable.get_vertex_array().update_indices(
        std::move(this->generate_topology<mge::RenderMode::SURFACE>()));
  }
}