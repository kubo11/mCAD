#ifndef MCAD_GREOGRY_PATCH_BUILDER
#define MCAD_GREOGRY_PATCH_BUILDER

#include "mge.hh"

struct Vertex {
  mge::EntityId eid;

  friend bool operator==(const Vertex& e1, const Vertex& e2) {
    return e1.eid == e2.eid;
  }
};

class BorderGraph {
  public:
   BorderGraph();
 
   unsigned int add_vertex(const Vertex& v);
   void add_edge(unsigned int v1, unsigned int v2);
   unsigned int get_vertex_count() const;
   void clear();
 
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

class GregoryPatchBuilder {
 public:
  std::vector<std::vector<mge::EntityId>> find_holes(const mge::EntityVector& surfaces);
 private:
  BorderGraph m_graph;
  std::unordered_map<mge::EntityId, unsigned int> m_reverse_point_map;

  unsigned int add_point_to_graph(const mge::EntityId& id);
};

#endif // MCAD_GREOGRY_PATCH_BUILDER