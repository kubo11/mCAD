#include "gregory_patch_builder.hh"

#include "../components/bezier_surface_c0_component.hh"

std::vector<std::vector<mge::EntityId>> GregoryPatchBuilder::find_holes(const mge::EntityVector& surfaces) {
  m_graph.clear();
  m_reverse_point_map.clear();

  for (const auto& surface_entity : surfaces) {
    auto& surface = surface_entity.get().get_component<BezierSurfaceC0Component>();
    const auto& points = surface.get_points();
    auto v_size = surface.get_point_count_v();
    auto u_size = surface.get_point_count_u();
    unsigned int idx, prev_idx;
    
    if (surface.get_wrapping() == BezierSurfaceWrapping::none || surface.get_wrapping() == BezierSurfaceWrapping::u) {
      // TOP
      idx = add_point_to_graph(points[0][0].second.get().get_id());
      prev_idx = idx;
      for (unsigned int u = 1; u < u_size; ++u) {
        idx = add_point_to_graph(points[0][u].second.get().get_id());
        m_graph.add_edge(idx, prev_idx);
        m_graph.add_edge(prev_idx, idx);
        prev_idx = idx;
      }

      // BOTTOM
      idx = add_point_to_graph(points[v_size-1][0].second.get().get_id());
      prev_idx = idx;
      for (unsigned int u = 1; u < u_size; ++u) {
      idx = add_point_to_graph(points[v_size-1][u].second.get().get_id());
        m_graph.add_edge(idx, prev_idx);
        m_graph.add_edge(prev_idx, idx);
        prev_idx = idx;
      }

      if (surface.get_wrapping() == BezierSurfaceWrapping::u) {
        auto beg = m_reverse_point_map.at(points[0][0].second.get().get_id());
        auto end = m_reverse_point_map.at(points[0][u_size-1].second.get().get_id());
        m_graph.add_edge(beg, end);
        m_graph.add_edge(end, beg);
        beg = m_reverse_point_map.at(points[v_size-1][0].second.get().get_id());
        end = m_reverse_point_map.at(points[v_size-1][u_size-1].second.get().get_id());
        m_graph.add_edge(beg, end);
        m_graph.add_edge(end, beg);
      }
    }

    if (surface.get_wrapping() == BezierSurfaceWrapping::none || surface.get_wrapping() == BezierSurfaceWrapping::v) {
      // LEFT
      idx = add_point_to_graph(points[0][0].second.get().get_id());
      prev_idx = idx;
      for (unsigned int v = 1; v < v_size; ++v) {
        idx = add_point_to_graph(points[v][0].second.get().get_id());
        m_graph.add_edge(idx, prev_idx);
        m_graph.add_edge(prev_idx, idx);
        prev_idx = idx;
      }

      // RIGHT
      idx = add_point_to_graph(points[0][u_size-1].second.get().get_id());
      prev_idx = idx;
      for (unsigned int v = 1; v < v_size; ++v) {
        idx = add_point_to_graph(points[v][u_size-1].second.get().get_id());
        m_graph.add_edge(idx, prev_idx);
        m_graph.add_edge(prev_idx, idx);
        prev_idx = idx;
      }

      if (surface.get_wrapping() == BezierSurfaceWrapping::v) {
        auto beg = m_reverse_point_map.at(points[0][0].second.get().get_id());
        auto end = m_reverse_point_map.at(points[v_size-1][0].second.get().get_id());
        m_graph.add_edge(beg, end);
        m_graph.add_edge(end, beg);
        beg = m_reverse_point_map.at(points[0][u_size-1].second.get().get_id());
        end = m_reverse_point_map.at(points[v_size-1][u_size-1].second.get().get_id());
        m_graph.add_edge(beg, end);
        m_graph.add_edge(end, beg);
      }
    }
  }

  std::vector<std::vector<mge::EntityId>> holes = {};
  auto cycles = m_graph.find_cycles(9u);

  for (const auto& cycle : cycles) {
    holes.push_back({});
    for (const auto& v : cycle) {
      holes.back().push_back(v.eid);
    }
  }

  return holes;
}

unsigned int GregoryPatchBuilder::add_point_to_graph(const mge::EntityId& id) {
  if (!m_reverse_point_map.contains(id)) {
    auto v =  m_graph.add_vertex({id});
    m_reverse_point_map.insert({id, v});
    return v;
  }
  return m_reverse_point_map.at(id);
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

std::vector<Vertex> BorderGraph::dfs() {
  std::vector<bool> visited(m_vertices.size());
  std::vector<Vertex> order;
  for(int i = 0; i < visited.size(); i++) {
    if(!visited[i]) {
      auto order_below = dfs(visited, i);
      order.insert(order.end(), order_below.begin(), order_below.end());
    }
  }
  return order;
}

std::vector<Vertex> BorderGraph::dfs(std::vector<bool> &visited, unsigned int v) {
  std::vector<Vertex> order;

  order.push_back(m_vertices[v]);
  visited[v] = true;

  for(unsigned int i = 0; i < m_neighbour_list[v].size(); ++i) {
    unsigned int n = m_neighbour_list[v][i];
    if(!visited[n]) {
      auto order_below = dfs(visited, n);
      order.insert(order.end(), order_below.begin(), order_below.end());
    }
  }
  return order;
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