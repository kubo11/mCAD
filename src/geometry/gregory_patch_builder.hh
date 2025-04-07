#ifndef MCAD_GREOGRY_PATCH_BUILDER
#define MCAD_GREOGRY_PATCH_BUILDER

#include "mge.hh"

struct Vertex {
  std::vector<std::reference_wrapper<mge::Entity>> points;
  std::vector<std::reference_wrapper<mge::Entity>> prevs;
  bool is_corner;
  mge::OptionalEntity m_patch;

  friend bool operator==(const Vertex& e1, const Vertex& e2) {
    if (e1.points.size() != e2.points.size()) return false;
    for (unsigned int i = 0; i < e1.points.size(); ++i)
      if (e1.points[i].get().get_id() != e2.points[i].get().get_id()) return false;
    return true;
  }
};

class BorderGraph {
  public:
   BorderGraph();
 
   unsigned int add_vertex(const Vertex& v);
   void add_edge(unsigned int v1, unsigned int v2);
   unsigned int get_vertex_count() const;
   void clear();
   Vertex& get_vertex(unsigned int v);
 
   std::vector<Vertex> dfs();
   std::vector<std::vector<Vertex>> find_cycles(unsigned int length);
   std::vector<std::vector<Vertex>> find_cycles(unsigned int beg, unsigned int length);
 
  private:
   std::vector<Vertex> m_vertices;
   std::vector<std::vector<unsigned int>> m_neighbour_list;
 
   std::vector<Vertex> dfs(std::vector<bool> &visited, unsigned int v);
 
   std::vector<std::vector<Vertex>> find_cycles(unsigned int beg, unsigned int v, unsigned int length, std::vector<bool> visited);
 
   bool comapre_cycles(const std::vector<Vertex>& c1, const std::vector<Vertex>& c2);
};

struct GregoryPatchData {
  std::vector<Vertex> patch_sides;
  glm::vec3 p0;
  std::vector<glm::vec3> p1;
  std::vector<glm::vec3> p2;
  std::vector<std::array<glm::vec3, 7>> p3;
  std::vector<std::array<glm::vec3, 4>> fi0;
  std::vector<std::array<glm::vec3, 2>> fi1;
  std::vector<std::array<glm::vec3, 2>> fi2;
};

class GregoryPatchBuilder {
 public:
  std::vector<std::vector<mge::EntityId>> find_holes(const mge::EntityVector& surfaces);
  GregoryPatchData fill_hole(const std::vector<mge::EntityId>& hole_ids);
  GregoryPatchData fill_hole(const std::vector<Vertex>& hole);
 private:
  BorderGraph m_graph;
  std::unordered_map<mge::EntityId, unsigned int> m_reverse_corner_map;
  std::vector<std::vector<Vertex>> m_holes;

  void add_vertex_to_graph(Vertex v);
  void swap_order(Vertex& v);
  std::vector<Vertex> get_hole(const std::vector<mge::EntityId>& hole_ids);

  std::pair<std::array<glm::vec3, 4>, std::array<glm::vec3, 4>> divideDeCasteljau(std::array<glm::vec3, 4> control_points, float t);
  glm::vec3 deCasteljau(std::array<glm::vec3, 3> control_points, float t);
};

#endif // MCAD_GREOGRY_PATCH_BUILDER