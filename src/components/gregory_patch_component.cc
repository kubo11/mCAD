#include "gregory_patch_component.hh"

unsigned int GregoryPatchComponent::s_new_id = 1;

GregoryPatchComponent::GregoryPatchComponent(const GregoryPatchData& data, mge::Entity& gregory_entity, mge::Entity& vectors_entity) : m_data(data), m_self(gregory_entity), m_vectors(vectors_entity) {
  for (auto& side : m_data.patch_sides) {
    for (auto& point : side.points) {
      m_self.add_child(point);
      m_points.push_back(
          {point.get().register_on_update<mge::TransformComponent>(&GregoryPatchComponent::update_patch, this), point});
    }
    for (auto& point : side.prevs) {
      m_self.add_child(point);
      m_points.push_back(
          {point.get().register_on_update<mge::TransformComponent>(&GregoryPatchComponent::update_patch, this), point});
    }
  }
  update_position();
}

GregoryPatchComponent::~GregoryPatchComponent() {
  mge::DeleteEntityEvent event(m_vectors.get_id());
  mge::SendEvent(event);
  for (auto& [handle, point] : m_points) {
    point.get().unregister_on_update<mge::TransformComponent>(handle);
    m_self.remove_child(point);
  }
}

bool GregoryPatchComponent::get_vectors_status() const {
  return m_vectors.get_component<mge::RenderableComponent<GeometryVertex>>().is_enabled();
}

void GregoryPatchComponent::set_vectors_status(bool status) {
  m_vectors.patch<mge::RenderableComponent<GeometryVertex>>([&status](auto& renderable) {
    if (status) {
      renderable.enable();
    } else {
      renderable.disable();
    }
  });
}

unsigned int GregoryPatchComponent::get_line_count() const {
  return m_line_count;
}

void GregoryPatchComponent::set_line_count(unsigned int line_count) {
  m_line_count = line_count;
}

void GregoryPatchComponent::update_patch(mge::Entity& entity) {
  GregoryPatchBuilder builder;
  m_data = builder.fill_hole(m_data.patch_sides);
  update_renderables(generate_patch_geometry(), generate_vectors_geometry());
  update_position();
}

void GregoryPatchComponent::update_position() {
  glm::vec3 center = m_data.p0;
  m_self.patch<mge::TransformComponent>([&center](auto& transform) {
    transform.set_position(center);
  });
}

std::vector<GeometryVertex> GregoryPatchComponent::generate_patch_geometry() const {
  std::vector<GeometryVertex> vertices;

  for(int i = 0; i < m_data.patch_sides.size(); i++) {
    int side = i;
    int next_side = (i+1) % m_data.patch_sides.size();

    vertices.emplace_back(m_data.p3[side][3]);
    vertices.emplace_back(m_data.p3[side][4]);
    vertices.emplace_back(m_data.p3[side][5]);
    vertices.emplace_back(m_data.p3[side][6]);
    vertices.emplace_back(m_data.p3[next_side][1]);
    vertices.emplace_back(m_data.p3[next_side][2]);
    vertices.emplace_back(m_data.p3[next_side][3]);

    vertices.emplace_back(m_data.p2[next_side]);
    vertices.emplace_back(m_data.p1[next_side]);
    vertices.emplace_back(m_data.p0);
    vertices.emplace_back(m_data.p1[side]);
    vertices.emplace_back(m_data.p2[side]);

    vertices.emplace_back(m_data.fi0[side][2]);
    vertices.emplace_back(m_data.fi0[side][3]);
    vertices.emplace_back(m_data.fi0[next_side][0]);
    vertices.emplace_back(m_data.fi0[next_side][1]);
    vertices.emplace_back(m_data.fi1[next_side][0]);
    vertices.emplace_back(m_data.fi2[next_side][0]);
    vertices.emplace_back(m_data.fi2[side][1]);
    vertices.emplace_back(m_data.fi1[side][1]);
  }

  return vertices;
}

std::vector<GeometryVertex> GregoryPatchComponent::generate_vectors_geometry() const {
  std::vector<GeometryVertex> vectors;

  for(int i = 0; i < m_data.patch_sides.size(); i++) {
    int side = i;
    int next_side = (i+1) % m_data.patch_sides.size();

    for(int i = 0; i < m_data.p3[side].size()-1; i++) {
      vectors.emplace_back(m_data.p3[side][i]);
      vectors.emplace_back(m_data.p3[side][i+1]);
    }

    vectors.emplace_back(m_data.p3[side][3]);
    vectors.emplace_back(m_data.p2[side]);

    vectors.emplace_back(m_data.p2[side]);
    vectors.emplace_back(m_data.p1[side]);

    vectors.emplace_back(m_data.p1[side]);
    vectors.emplace_back(m_data.p0);

    auto s = {1, 2, 4, 5};
    for(int j = 0; j < s.size(); j++) {
      vectors.emplace_back(m_data.p3[side][s.begin()[j]]);
      vectors.emplace_back(m_data.fi0[side][j]);
    }
  }

  return vectors;
}