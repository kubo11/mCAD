#include "gregory_patch_builder.hh"

#include "../components/bezier_surface_c0_component.hh"

std::vector<std::vector<mge::EntityId>> GregoryPatchBuilder::find_holes(const mge::EntityVector& surfaces) {
  m_graph.clear();
  m_reverse_corner_map.clear();
  m_holes.clear();

  for (const auto& surface_entity : surfaces) {
    auto& surface = surface_entity.get().get_component<BezierSurfaceC0Component>();
    const auto& points = surface.get_points();
    auto v_size = surface.get_point_count_v();
    auto u_size = surface.get_point_count_u();
    
    if (surface.get_wrapping() == BezierSurfaceWrapping::none || surface.get_wrapping() == BezierSurfaceWrapping::u) {
      bool wrapped = surface.get_wrapping() == BezierSurfaceWrapping::u;
      // TOP
      for (unsigned int u = 0; u < u_size-2; u+=3) {
        add_vertex_to_graph({
          {points[0][u].second, points[0][u+1].second, points[0][u+2].second, (wrapped && u == u_size - 3) ? points[0][0].second : points[0][u+3].second},
          {points[1][u].second, points[1][u+1].second, points[1][u+2].second, (wrapped && u == u_size - 3) ? points[1][0].second : points[1][u+3].second},
          false,
          surface_entity
        });
      }

      // BOTTOM
      for (unsigned int u = 0; u < u_size-2; u+=3) {
        add_vertex_to_graph({
          {points[v_size-1][u].second, points[v_size-1][u+1].second, points[v_size-1][u+2].second, (wrapped && u == u_size - 3) ? points[v_size-1][0].second : points[v_size-1][u+3].second},
          {points[v_size-2][u].second, points[v_size-2][u+1].second, points[v_size-2][u+2].second, (wrapped && u == u_size - 3) ? points[v_size-2][0].second : points[v_size-2][u+3].second},
          false,
          surface_entity
        });
      }
    }

    if (surface.get_wrapping() == BezierSurfaceWrapping::none || surface.get_wrapping() == BezierSurfaceWrapping::v) {
      bool wrapped = surface.get_wrapping() == BezierSurfaceWrapping::v;
      // LEFT
      for (unsigned int v = 0; v < v_size-2; v+=3) {
        add_vertex_to_graph({
          {points[v][0].second, points[v+1][0].second, points[v+2][0].second, (wrapped && v == v_size - 3) ? points[0][0].second : points[v+3][0].second},
          {points[v][1].second, points[v+1][1].second, points[v+2][1].second, (wrapped && v == v_size - 3) ? points[0][1].second : points[v+3][1].second},
          false,
          surface_entity
        });
      }

      // RIGHT
      for (unsigned int v = 0; v < v_size-2; v+=3) {
        add_vertex_to_graph({
          {points[v][u_size-1].second, points[v+1][u_size-1].second, points[v+2][u_size-1].second, (wrapped && v == v_size - 3) ? points[0][u_size-1].second : points[v+3][u_size-1].second},
          {points[v][u_size-2].second, points[v+1][u_size-2].second, points[v+2][u_size-2].second, (wrapped && v == v_size - 3) ? points[0][u_size-2].second : points[v+3][u_size-2].second},
          false,
          surface_entity
        });
      }
    }
  }

  auto cycles = m_graph.find_cycles(6u);

  for (const auto& cycle : cycles) {
    m_holes.push_back({});
    unsigned int beg = 0;
    while (!cycle[beg].is_corner) ++beg;
    for (unsigned int i = beg; i < cycle.size(); ++i) {
      if (cycle[i].is_corner) continue;
      m_holes.back().push_back(cycle[i]);
    }
    for (unsigned int i = 0; i < beg; ++i) {
      if (cycle[i].is_corner) continue;
      m_holes.back().push_back(cycle[i]);
    }
    for (unsigned int i = 1; i < m_holes.back().size(); ++i) {
      if (m_holes.back()[i-1].points[0].get().get_id() == m_holes.back()[i].points[0].get().get_id()) {
        swap_order(m_holes.back()[i-1]);
      }
      else if (m_holes.back()[i-1].points[3].get().get_id() == m_holes.back()[i].points[3].get().get_id()) {
        swap_order(m_holes.back()[i]);
      }
      else if (m_holes.back()[i-1].points[0].get().get_id() == m_holes.back()[i].points[3].get().get_id()) {
        swap_order(m_holes.back()[i-1]);
        swap_order(m_holes.back()[i]);
      }
    }
  }

  std::vector<std::vector<mge::EntityId>> holes = {};
  for (auto& hole : m_holes) {
    holes.push_back({});
    for (auto& side : hole) {
      holes.back().push_back(side.points[0].get().get_id());
      holes.back().push_back(side.points[1].get().get_id());
      holes.back().push_back(side.points[2].get().get_id());
    }
  }

  return holes;
}

GregoryPatchData GregoryPatchBuilder::fill_hole(const std::vector<mge::EntityId>& hole_ids) {
  auto hole = get_hole(hole_ids);

  return fill_hole(hole);
}

GregoryPatchData GregoryPatchBuilder::fill_hole(const std::vector<Vertex>& hole) {
  GregoryPatchData data = {};
  data.patch_sides = hole;
  std::vector<glm::vec3> Q;
  for(auto &side : hole) {
    std::array<glm::vec3, 4> control_points_positions {
      side.points[0].get().get_component<mge::TransformComponent>().get_position(),
      side.points[1].get().get_component<mge::TransformComponent>().get_position(),
      side.points[2].get().get_component<mge::TransformComponent>().get_position(),
      side.points[3].get().get_component<mge::TransformComponent>().get_position()
    };
    auto divided = divideDeCasteljau(control_points_positions, 0.5f);

    data.p3.emplace_back();
    data.p3.back()[0] = divided.first[0];
    data.p3.back()[1] = divided.first[1];
    data.p3.back()[2] = divided.first[2];
    data.p3.back()[3] = divided.first[3];
    data.p3.back()[4] = divided.second[1];
    data.p3.back()[5] = divided.second[2];
    data.p3.back()[6] = divided.second[3];

    std::array<glm::vec3, 4> control_points_prev_positions {
      side.prevs[0].get().get_component<mge::TransformComponent>().get_position(),
      side.prevs[1].get().get_component<mge::TransformComponent>().get_position(),
      side.prevs[2].get().get_component<mge::TransformComponent>().get_position(),
      side.prevs[3].get().get_component<mge::TransformComponent>().get_position()
    };
    auto divided1 = divideDeCasteljau(control_points_prev_positions, 0.5f);

    data.fi0.emplace_back();
    data.fi0.back()[0] = 2.f * divided.first[1] - divided1.first[1];
    data.fi0.back()[1] = 2.f * divided.first[2] - divided1.first[2];
    data.fi0.back()[2] = 2.f * divided.second[1] - divided1.second[1];
    data.fi0.back()[3] = 2.f * divided.second[2] - divided1.second[2];

    auto p3 = divided.first[3];
    auto p4 = divided1.first[3];
    auto p2 = p3 + (p3 - p4);
    auto qi = (3.0f * p2 - p3) / 2.0f;
    Q.push_back(qi);

    data.p2.push_back(p2);
  }

  data.p0 = glm::vec3(0.0f);
  for(auto q : Q)
    data.p0 += q;
  data.p0 /= Q.size();

  for(unsigned int i = 0; i < hole.size(); ++i) {
      auto q = Q[i];
      auto &side = hole[i];

      auto p1 = (2.0f * q + data.p0) / 3.0f;

      data.p1.push_back(p1);
  }

  for(unsigned int i = 0; i < hole.size(); ++i) {
    unsigned int side = i;
    unsigned int next_side = (i+1) % hole.size();
    unsigned int prev_side = i == 0 ? hole.size()-1 : i-1;

    glm::vec3 g0R = data.p3[side][4] - data.p3[side][3];
    glm::vec3 g0L = data.p3[side][2] - data.p3[side][3];

    glm::vec3 a = data.p0 - data.p1[prev_side];
    glm::vec3 b = data.p1[next_side] - data.p0;

    glm::vec3 c0 = data.p1[side] - data.p1[next_side];
    glm::vec3 c1 = glm::cross(data.p3[next_side][3] - data.p3[side][3], data.p3[side][6] - data.p3[side][3]);


    glm::vec3 g2R = (hole.size() == 2) ? glm::cross(c0, c1) : ((a + b) / 2.0f);

    glm::vec3 g2L = -g2R;

    glm::vec3 g1Right = (g0R + g2R) / 2.f;
    glm::vec3 g1Left = (g0L + g2L) / 2.f;

    glm::vec3 g01R = deCasteljau({g2R, g1Right, g0R}, 1.0f/3.0f);
    glm::vec3 g12R = deCasteljau({g2R, g1Right, g0R}, 2.0f/3.0f);
    glm::vec3 g01L = deCasteljau({g2L, g1Left, g0L}, 1.0f/3.0f);
    glm::vec3 g12L = deCasteljau({g2L, g1Left, g0L}, 2.0f/3.0f);

    data.fi1.emplace_back();
    data.fi1.back()[0] = data.p2[side] + g01L;
    data.fi1.back()[1] = data.p2[side] + g01R;

    data.fi2.emplace_back();
    data.fi2.back()[0] = data.p1[side] + g12L;
    data.fi2.back()[1] = data.p1[side] + g12R;

  }

  return data;
}

std::pair<std::array<glm::vec3, 4>, std::array<glm::vec3, 4>>
GregoryPatchBuilder::divideDeCasteljau(std::array<glm::vec3, 4> control_points, float t) {
    auto B00 = control_points[0];
    auto B01 = control_points[1];
    auto B02 = control_points[2];
    auto B03 = control_points[3];

    auto B10 = B00 * t + B01 * (1 - t);
    auto B11 = B01 * t + B02 * (1 - t);
    auto B12 = B02 * t + B03 * (1 - t);

    auto B20 = B10 * t + B11 * (1 - t);
    auto B21 = B11 * t + B12 * (1 - t);

    auto B30 = B20 * t + B21 * (1 - t);

    return {{B00, B10, B20, B30}, {B30, B21, B12, B03}};
}

glm::vec3 GregoryPatchBuilder::deCasteljau(std::array<glm::vec3, 3> control_points, float t) {
    auto B00 = control_points[0];
    auto B01 = control_points[1];
    auto B02 = control_points[2];

    auto B10 = B00 * t + B01 * (1 - t);
    auto B11 = B01 * t + B02 * (1 - t);

    auto B20 = B10 * t + B11 * (1 - t);

    return {B20};
}

void GregoryPatchBuilder::add_vertex_to_graph(Vertex v) {
  auto v_idx = m_graph.add_vertex(v);
  if (!m_reverse_corner_map.contains(v.points[0].get().get_id())) {
    auto corner_idx = m_graph.add_vertex({{v.points[0]}, {}, true});
    m_reverse_corner_map.insert({v.points[0].get().get_id(), corner_idx});
    m_graph.add_edge(v_idx, corner_idx);
    m_graph.add_edge(corner_idx, v_idx);
  }
  else {
    auto corner_idx = m_reverse_corner_map.at(v.points[0].get().get_id());
    m_graph.add_edge(v_idx, corner_idx);
    m_graph.add_edge(corner_idx, v_idx);
  }
  if (!m_reverse_corner_map.contains(v.points[3].get().get_id())) {
    auto corner_idx = m_graph.add_vertex({{v.points[3]}, {}, true});
    m_reverse_corner_map.insert({v.points[3].get().get_id(), corner_idx});
    m_graph.add_edge(v_idx, corner_idx);
    m_graph.add_edge(corner_idx, v_idx);
  }
  else {
    auto corner_idx = m_reverse_corner_map.at(v.points[3].get().get_id());
    m_graph.add_edge(v_idx, corner_idx);
    m_graph.add_edge(corner_idx, v_idx);
  }
}

void GregoryPatchBuilder::swap_order(Vertex& v) {
  std::reference_wrapper<mge::Entity> tmp= v.points[0];
  v.points[0] = v.points[3];
  v.points[3] = tmp;
  tmp = v.points[1];
  v.points[1] = v.points[2];
  v.points[2] = tmp;
  tmp= v.prevs[0];
  v.prevs[0] = v.prevs[3];
  v.prevs[3] = tmp;
  tmp = v.prevs[1];
  v.prevs[1] = v.prevs[2];
  v.prevs[2] = tmp;
}

std::vector<Vertex> GregoryPatchBuilder::get_hole(const std::vector<mge::EntityId>& hole_ids) {
  unsigned int counter = 0u;
  for (auto& hole : m_holes) {
    counter = 0u;
    for (auto& side : hole) {
      for (auto& point : side.points) {
        for (auto& id : hole_ids) {
          if (id == point.get().get_id()) ++counter;
        }
      }
    }
    if (counter == 12) return hole;
  }
  return {};
}

BorderGraph::BorderGraph() : m_vertices{}, m_neighbour_list{} {}

unsigned int BorderGraph::add_vertex(const Vertex& v) {
  m_vertices.push_back(v);
  m_neighbour_list.push_back({});
  return m_vertices.size() - 1u;
}

void BorderGraph::add_edge(unsigned int v1, unsigned int v2) {
  m_neighbour_list[v1].push_back(v2);
}

unsigned int BorderGraph::get_vertex_count() const {
  return m_vertices.size();
}

void BorderGraph::clear() {
  m_neighbour_list.clear();
  m_vertices.clear();
}

Vertex& BorderGraph::get_vertex(unsigned int v) {
  return m_vertices[v];
}

std::vector<Vertex> BorderGraph::dfs() {
  std::vector<bool> visited(m_vertices.size());
  std::vector<Vertex> path;
  for(int i = 0; i < visited.size(); i++) {
    if(!visited[i]) {
      auto path_below = dfs(visited, i);
      path.insert(path.end(), path_below.begin(), path_below.end());
    }
  }
  return path;
}

std::vector<Vertex> BorderGraph::dfs(std::vector<bool> &visited, unsigned int v) {
  std::vector<Vertex> path{};

  path.push_back(m_vertices[v]);
  visited[v] = true;

  for(unsigned int i = 0; i < m_neighbour_list[v].size(); ++i) {
    unsigned int n = m_neighbour_list[v][i];
    if(!visited[n]) {
      auto path_below = dfs(visited, n);
      path.insert(path.end(), path_below.begin(), path_below.end());
    }
  }
  return path;
}

std::vector<std::vector<Vertex>> BorderGraph::find_cycles(unsigned int length) {
  std::vector<std::vector<Vertex>> cycles;
  for(unsigned int i = 0; i < m_vertices.size(); ++i) {
    auto cycle = find_cycles(i, length);
    cycles.insert(cycles.end(), cycle.begin(), cycle.end());
  }
  std::vector<std::vector<Vertex>> unique_cycles;
  for(auto &cycle : cycles) {
    if(std::any_of(unique_cycles.begin(), unique_cycles.end(), [&](auto &c){return comapre_cycles(cycle, c);})) continue;
    unique_cycles.push_back(cycle);
  }
  return unique_cycles;
}

std::vector<std::vector<Vertex>> BorderGraph::find_cycles(unsigned int beg, unsigned int length) {
  std::vector<bool> visited(m_vertices.size(), false);
  return find_cycles(beg, beg, length, visited);
}

std::vector<std::vector<Vertex>> BorderGraph::find_cycles(unsigned int beg, unsigned int v, unsigned int length, std::vector<bool> visited) {
  visited[v] = true;
  if(length <= 1) {
    for(auto &n : m_neighbour_list[v]) {
      if (n == beg)
        return {{m_vertices[v]}};
    }
    return {};
  }
  std::vector<std::vector<Vertex>> cycles;
  for(auto &n : m_neighbour_list[v]) {
    if(visited[n] == true) continue;
    auto sub_cycles = find_cycles(beg, n, length - 1, visited);
    for(auto &sub_cycle : sub_cycles) {
      sub_cycle.push_back(m_vertices[v]);
      cycles.push_back(sub_cycle);
    }
  }
  return cycles;
}

bool BorderGraph::comapre_cycles(const std::vector<Vertex>& c1, const std::vector<Vertex>& c2) {
  for(auto &e1 : c1) {
    bool exists = false;
    for(auto &e2 : c2) {
      if (e1 == e2) exists = true;
    }
    if(!exists) return false;
  }
  return true;
}